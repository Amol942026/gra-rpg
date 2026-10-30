"""Local Laigter automation — generate normal, specular, occlusion and parallax maps.

Entry points:
    python tools/media/laigter_cli.py version
    python tools/media/laigter_cli.py generate <sprite.png> [--output-root DIR] [--preset FILE]
    python tools/media/laigter_cli.py verify   <manifest.json>

All map generation is silent (no GUI, no window). Originals are never passed
directly to Laigter; a private copy is used and hash-verified. Each run gets a
unique output folder under --output-root (default C:\\AI2D\\maps).
"""
from __future__ import annotations
import argparse, datetime, hashlib, json, os, re, struct, subprocess, sys, uuid, zlib
from pathlib import Path

LAIGTER = Path(r"C:\AI2D\Tools\Laigter\laigter.exe")
SPRITES_ROOT = Path(r"C:\AI2D\sprites")
OUTPUT_ROOT = Path(r"C:\AI2D\maps")
TIMEOUT = 120

MAP_SUFFIXES = (("normal", "n"), ("specular", "s"), ("occlusion", "o"), ("parallax", "p"))


def sha(path: str | Path) -> str:
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def _run(args: list, log_dir: Path, label: str, *, allowed=(0,)) -> str:
    """Run Laigter with given arguments, capture output, validate exit code."""
    log_dir.mkdir(parents=True, exist_ok=True)
    env = os.environ.copy()
    env.update(QT_QPA_PLATFORM="windows", QT_OPENGL="software")
    proc = subprocess.run(
        [str(LAIGTER)] + [str(a) for a in args],
        cwd=LAIGTER.parent,
        env=env,
        capture_output=True,
        timeout=TIMEOUT,
        creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
    )
    (log_dir / (label + "-stdout.log")).write_bytes(proc.stdout)
    (log_dir / (label + "-stderr.log")).write_bytes(proc.stderr)
    if proc.returncode not in allowed:
        raise RuntimeError(
            f"Laigter exit {proc.returncode}; see {log_dir / (label + '-stderr.log')}"
        )
    combined = proc.stdout.decode("utf-8-sig", "replace")
    combined += "\n" + proc.stderr.decode("utf-8-sig", "replace")
    return combined


# ── Minimal PNG decoder (stdlib only, no PIL/numpy dependency) ──────────────

def _read_png(path: Path) -> dict:
    """Decode a PNG and return width, height, pixel SHA-256 and map diagnostics."""
    data = Path(path).read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError(f"Not a PNG: {path}")
    # Parse IHDR
    pos = 8
    length = struct.unpack(">I", data[pos:pos+4])[0]; pos += 4
    chunk_type = data[pos:pos+4]; pos += 4
    if chunk_type != b"IHDR":
        raise ValueError(f"Missing IHDR in {path}")
    width, height, bit_depth, color_type = struct.unpack(">IIBB", data[pos:pos+10])
    pos += length + 4  # skip rest of IHDR + CRC

    # Collect IDAT chunks
    idat_chunks = []
    while pos < len(data) - 4:
        length = struct.unpack(">I", data[pos:pos+4])[0]; pos += 4
        chunk_type = data[pos:pos+4]; pos += 4
        if chunk_type == b"IDAT":
            idat_chunks.append(data[pos:pos+length])
        pos += length + 4  # data + CRC
        if chunk_type == b"IEND":
            break

    # Decompress and unfilter
    compressed = b"".join(idat_chunks)
    raw = zlib.decompress(compressed)

    # Determine bytes per pixel (support RGB, RGBA, Grayscale, GrayAlpha)
    bpp_map = {0: 1, 2: 3, 4: 2, 6: 4}
    if color_type not in bpp_map:
        raise ValueError(f"Unsupported PNG color type {color_type} in {path}")
    bpp = bpp_map[color_type] * max(1, bit_depth // 8)
    stride = bpp * width

    # Simple unfilter for 8-bit PNGs
    pixels = bytearray(height * stride)
    src = 0
    for y in range(height):
        filter_byte = raw[src]; src += 1
        row_start = y * stride
        prev_row_start = (y - 1) * stride if y > 0 else -1
        for x in range(stride):
            raw_byte = raw[src]; src += 1
            a = pixels[row_start + x - bpp] if x >= bpp else 0
            b = pixels[prev_row_start + x] if prev_row_start >= 0 else 0
            c = pixels[prev_row_start + x - bpp] if prev_row_start >= 0 and x >= bpp else 0
            if filter_byte == 0:
                val = raw_byte
            elif filter_byte == 1:
                val = (raw_byte + a) & 0xFF
            elif filter_byte == 2:
                val = (raw_byte + b) & 0xFF
            elif filter_byte == 3:
                val = (raw_byte + ((a + b) >> 1)) & 0xFF
            elif filter_byte == 4:
                # Paeth
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                pr = a if pa <= pb and pa <= pc else (b if pb <= pc else c)
                val = (raw_byte + pr) & 0xFF
            else:
                raise ValueError(f"Unknown PNG filter type {filter_byte}")
            pixels[row_start + x] = val

    pixel_sha = hashlib.sha256(bytes(pixels)).hexdigest()

    # Sample statistics (step through ~64x64 grid for performance)
    step_x = max(1, width // 64)
    step_y = max(1, height // 64)
    samples = 0
    grayscale = True
    max_normal_error = 0.0
    visible_pixels = 0

    for y in range(0, height, step_y):
        for x in range(0, width, step_x):
            offset = y * stride + x * bpp
            if color_type == 6:  # RGBA
                r, g, b_val, a = pixels[offset], pixels[offset+1], pixels[offset+2], pixels[offset+3]
                if a == 0:
                    continue
            elif color_type == 2:  # RGB
                r, g, b_val = pixels[offset], pixels[offset+1], pixels[offset+2]
            elif color_type == 0:  # Grayscale
                r = g = b_val = pixels[offset]
            elif color_type == 4:  # GrayAlpha
                r = g = b_val = pixels[offset]
                if pixels[offset+1] == 0:
                    continue
            else:
                continue
            visible_pixels += 1
            if r != g or g != b_val:
                grayscale = False
            # Normal vector unit check
            nx = 2.0 * r / 255.0 - 1.0
            ny = 2.0 * g / 255.0 - 1.0
            nz = 2.0 * b_val / 255.0 - 1.0
            mag = (nx*nx + ny*ny + nz*nz) ** 0.5
            max_normal_error = max(max_normal_error, abs(mag - 1.0))

    return {
        "width": width, "height": height, "pixel_sha256": pixel_sha,
        "sampled_pixels": visible_pixels, "grayscale": grayscale,
        "max_normal_unit_error": round(max_normal_error, 6),
    }


# ── Public API ──────────────────────────────────────────────────────────────

def version(log_dir: Path | None = None) -> str:
    """Return Laigter version string."""
    log_dir = log_dir or (OUTPUT_ROOT / "_version")
    output = _run(["--no-gui", "--software-opengl", "--version"], log_dir, "version")
    match = re.search(r"(?i)\blaigter\s+\d+\.\d+(?:\.\d+)?", output)
    if not match:
        raise RuntimeError("Laigter did not return a valid version.")
    return match.group(0).strip()


def generate_maps(
    sprite: str | Path,
    *,
    output_root: str | Path | None = None,
    sprites_root: str | Path | None = None,
    preset: str | Path | None = None,
) -> dict:
    """Generate all four map types and return a verified manifest dict.

    Parameters
    ----------
    sprite : path to source PNG inside sprites_root
    output_root : base dir for output (default C:\\AI2D\\maps)
    sprites_root : source validation root (default C:\\AI2D\\sprites)
    preset : optional Laigter preset file path
    """
    sprites_root = Path(sprites_root or SPRITES_ROOT).resolve()
    output_root = Path(output_root or OUTPUT_ROOT).resolve()
    sprite = Path(sprite).resolve()

    # Validate input location
    if not str(sprite).lower().startswith(str(sprites_root).lower() + "\\"):
        raise ValueError(f"Input must be inside {sprites_root}")
    if sprite.suffix.lower() != ".png":
        raise ValueError(f"Input must be a PNG: {sprite}")
    if not sprite.is_file():
        raise FileNotFoundError(sprite)
    if str(output_root).lower().startswith(str(sprites_root).lower()):
        raise ValueError("output_root must be outside sprites_root.")

    source_hash = sha(sprite)
    source_info = _read_png(sprite)

    # Create unique run directory with private copy
    stem = sprite.stem
    run_name = f"{stem}_{datetime.datetime.now().strftime('%Y%m%d_%H%M%S_%f')}_{uuid.uuid4().hex[:6]}"
    run_dir = output_root / run_name
    private_dir = run_dir / "_input"
    private_dir.mkdir(parents=True, exist_ok=True)

    import shutil
    private_input = private_dir / sprite.name
    shutil.copy2(sprite, private_input)
    if sha(private_input) != source_hash:
        raise RuntimeError("Input changed while making a private copy.")

    # Copy preset if provided
    preset_copy = None
    preset_hash = None
    if preset:
        preset = Path(preset).resolve()
        preset_copy = private_dir / ("preset" + preset.suffix)
        shutil.copy2(preset, preset_copy)
        preset_hash = sha(preset_copy)

    # Get version
    import time
    t0 = time.perf_counter()
    ver = version(run_dir)

    # Build arguments
    arguments = [
        "--no-gui", "--software-opengl", "--not-recursive", "--flatten",
        "-d", str(private_input), "-l", str(run_dir),
        "-n", "-c", "-o", "-p",
    ]
    if preset_copy:
        arguments.extend(["-r", str(preset_copy)])

    _run(arguments, run_dir, "generate")

    # Verify originals unchanged
    if sha(sprite) != source_hash:
        raise RuntimeError("Original input changed during generation.")
    if sha(private_input) != source_hash:
        raise RuntimeError("Laigter changed its private input copy.")

    # Validate output maps
    maps = []
    for map_type, suffix in MAP_SUFFIXES:
        map_path = run_dir / f"{stem}_{suffix}.png"
        if not map_path.is_file():
            raise RuntimeError(f"Missing {map_type} map: {map_path}")

        decoded = _read_png(map_path)
        if decoded["width"] != source_info["width"] or decoded["height"] != source_info["height"]:
            raise RuntimeError(f"Wrong dimensions in {map_type} map.")
        if decoded["sampled_pixels"] == 0:
            raise RuntimeError(f"Map has no visible pixels: {map_path}")
        if map_type == "normal" and decoded["max_normal_unit_error"] > 0.10:
            raise RuntimeError(f"Invalid normal vectors in {map_type} map: {map_path}")
        if map_type in ("specular", "occlusion", "parallax") and not decoded["grayscale"]:
            raise RuntimeError(f"Expected grayscale {map_type} map: {map_path}")

        maps.append({
            "type": map_type,
            "path": str(map_path),
            "bytes": map_path.stat().st_size,
            "sha256": sha(map_path),
            "decoded": decoded,
        })

    elapsed = round(time.perf_counter() - t0, 3)

    manifest = {
        "Status": "PASS",
        "Source": str(sprite),
        "OriginalUnchanged": True,
        "SourceSha256": source_hash,
        "SourceWidth": source_info["width"],
        "SourceHeight": source_info["height"],
        "SourcePixelSha256": source_info["pixel_sha256"],
        "Laigter": str(LAIGTER),
        "LaigterSha256": sha(LAIGTER),
        "Version": ver,
        "Preset": str(preset) if preset else None,
        "PresetSha256": preset_hash,
        "Arguments": arguments,
        "OutputDirectory": str(run_dir),
        "MapCount": len(maps),
        "Maps": maps,
        "ElapsedSeconds": elapsed,
    }

    manifest_path = run_dir / "maps-manifest.json"
    with manifest_path.open("w", encoding="utf-8") as f:
        json.dump(manifest, f, indent=2, ensure_ascii=False)

    print(f"Generated {len(maps)} maps ({source_info['width']} x {source_info['height']}): {sprite}")
    return manifest


def verify_manifest(manifest_path: str | Path) -> dict:
    """Re-read and validate an existing manifest."""
    manifest_path = Path(manifest_path)
    manifest = json.loads(manifest_path.read_text(encoding="utf-8-sig"))
    if manifest["Status"] != "PASS":
        raise ValueError(f"Manifest status: {manifest['Status']}")
    for m in manifest["Maps"]:
        path = Path(m["path"])
        if not path.is_file():
            raise FileNotFoundError(f"Missing map: {path}")
        if sha(path) != m["sha256"]:
            raise ValueError(f"Hash mismatch: {path}")
        decoded = _read_png(path)
        if decoded["width"] != manifest["SourceWidth"] or decoded["height"] != manifest["SourceHeight"]:
            raise ValueError(f"Dimension mismatch: {path}")
    return {"status": "PASS", "manifest": str(manifest_path), "maps_verified": len(manifest["Maps"])}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="action", required=True)

    sub.add_parser("version")

    p = sub.add_parser("generate")
    p.add_argument("sprite", type=Path, help="Source PNG inside sprites root")
    p.add_argument("--output-root", type=Path, default=None)
    p.add_argument("--sprites-root", type=Path, default=None)
    p.add_argument("--preset", type=Path, default=None)

    p = sub.add_parser("verify")
    p.add_argument("manifest", type=Path, help="Path to maps-manifest.json")

    args = parser.parse_args()

    if args.action == "version":
        print(version())
    elif args.action == "generate":
        result = generate_maps(
            args.sprite,
            output_root=args.output_root,
            sprites_root=args.sprites_root,
            preset=args.preset,
        )
        print(json.dumps(result, indent=2))
    elif args.action == "verify":
        result = verify_manifest(args.manifest)
        print(json.dumps(result, indent=2))


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print("ERROR: " + str(e), file=sys.stderr)
        sys.exit(1)
