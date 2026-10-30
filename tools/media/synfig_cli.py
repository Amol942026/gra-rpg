"""Local Synfig automation — render projects, build SIF XML, inspect canvases.

Entry points:
    python tools/media/synfig_cli.py version
    python tools/media/synfig_cli.py info       <project>
    python tools/media/synfig_cli.py render      <project> [--output-root DIR] [--threads N]
                                                  [--expected-frames N] [--require-motion]
    python tools/media/synfig_cli.py verify      <manifest.json>
    python tools/media/synfig_cli.py make-cutout <sprite.png> <output.sif> [--pixels N]
                                                  [--fps N] [--frames N]

All rendering is silent (no GUI). Each render gets a NEW unique output folder.
Existing files are never deleted or overwritten.
"""
from __future__ import annotations
import argparse, datetime, hashlib, json, math, os, re, shutil, struct, subprocess
import sys, time, uuid, xml.etree.ElementTree as ET, zlib
from pathlib import Path

SYNFIG = Path(r"C:\Program Files\Synfig\bin\synfig.exe")
TIMEOUT = 300


def sha(path: str | Path) -> str:
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def _write_json(path: Path, data: dict):
    with path.open("w", encoding="utf-8") as f:
        json.dump(data, f, indent=2, ensure_ascii=False)


def _run(
    args: list,
    log_dir: Path,
    label: str,
    *,
    allowed: tuple[int, ...] = (0,),
    cwd: Path | None = None,
) -> str:
    """Run synfig.exe with arguments, capture output, check exit code."""
    log_dir.mkdir(parents=True, exist_ok=True)
    env = os.environ.copy()
    env.update(QT_QPA_PLATFORM="windows", QT_OPENGL="software")

    proc = subprocess.run(
        [str(SYNFIG)] + [str(a) for a in args],
        cwd=str(cwd or SYNFIG.parent),
        env=env,
        capture_output=True,
        timeout=TIMEOUT,
        creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
    )
    (log_dir / (label + "-stdout.log")).write_bytes(proc.stdout)
    (log_dir / (label + "-stderr.log")).write_bytes(proc.stderr)
    if proc.returncode not in allowed:
        raise RuntimeError(
            f"Synfig exit {proc.returncode}; see {log_dir / (label + '-stderr.log')}"
        )
    return proc.stdout.decode("utf-8-sig", "replace")


# ── Minimal PNG decoder (stdlib-only, no PIL/numpy) ────────────────────────

def _read_png_info(path: Path) -> dict:
    """Decode a PNG and return width, height, pixel SHA-256."""
    data = Path(path).read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError(f"Not a PNG: {path}")
    pos = 8
    length = struct.unpack(">I", data[pos:pos+4])[0]; pos += 4
    chunk_type = data[pos:pos+4]; pos += 4
    if chunk_type != b"IHDR":
        raise ValueError(f"Missing IHDR in {path}")
    width, height, bit_depth, color_type = struct.unpack(">IIBB", data[pos:pos+10])
    pos += length + 4

    idat_chunks = []
    while pos < len(data) - 4:
        length = struct.unpack(">I", data[pos:pos+4])[0]; pos += 4
        chunk_type = data[pos:pos+4]; pos += 4
        if chunk_type == b"IDAT":
            idat_chunks.append(data[pos:pos+length])
        pos += length + 4
        if chunk_type == b"IEND":
            break

    compressed = b"".join(idat_chunks)
    raw = zlib.decompress(compressed)

    bpp_map = {0: 1, 2: 3, 4: 2, 6: 4}
    if color_type not in bpp_map:
        raise ValueError(f"Unsupported PNG color type {color_type}")
    bpp = bpp_map[color_type] * max(1, bit_depth // 8)
    stride = bpp * width

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
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                pr = a if pa <= pb and pa <= pc else (b if pb <= pc else c)
                val = (raw_byte + pr) & 0xFF
            else:
                raise ValueError(f"Unknown PNG filter type {filter_byte}")
            pixels[row_start + x] = val

    pixel_sha = hashlib.sha256(bytes(pixels)).hexdigest()
    return {"width": width, "height": height, "pixel_sha256": pixel_sha}


# ── Public API ──────────────────────────────────────────────────────────────

def version(log_dir: Path | None = None) -> str:
    """Return Synfig version string."""
    log_dir = log_dir or Path(os.environ.get("TEMP", ".")) / "synfig_version"
    output = _run(["--version"], log_dir, "version", allowed=(0, 3))
    # Synfig may print version to stderr
    stderr_path = log_dir / "version-stderr.log"
    if stderr_path.exists():
        output += "\n" + stderr_path.read_text(encoding="utf-8", errors="replace")
    match = re.search(r"(?i)\bsynfig\s+\d+\.\d+(?:\.\d+)?(?:[-\w.]*)?", output)
    if not match:
        raise RuntimeError("Synfig did not return a valid version.")
    return match.group(0).strip()


def canvas_info(project: str | Path, log_dir: Path | None = None) -> dict:
    """Query canvas metadata from a Synfig project.

    Returns dict with keys: frame_start, frame_end, frame_rate, w, h.
    """
    project = Path(project).resolve()
    if project.suffix.lower() not in (".sif", ".sifz", ".sfg"):
        raise ValueError(f"Unsupported project format: {project}")
    log_dir = log_dir or project.parent / "renders" / "_info"
    options = ["-i", str(project)]
    _run(
        options + ["--canvas-info=all"],
        log_dir, "canvas-info",
        cwd=project.parent,
    )
    info_path = log_dir / "canvas-info-stdout.log"
    values = {}
    for line in info_path.read_text(encoding="utf-8", errors="replace").splitlines():
        match = re.match(r"^(frame_start|frame_end|frame_rate|w|h)=(.+)$", line)
        if match:
            values[match.group(1)] = match.group(2).strip()
    for key in ("frame_start", "frame_end", "frame_rate", "w", "h"):
        if key not in values:
            raise RuntimeError(f"Missing '{key}' in Synfig canvas information")
    return {
        "project": str(project),
        "frame_start": int(values["frame_start"]),
        "frame_end": int(values["frame_end"]),
        "frame_rate": float(values["frame_rate"].replace(",", ".")),
        "width": int(values["w"]),
        "height": int(values["h"]),
    }


def render(
    project: str | Path,
    *,
    output_root: str | Path | None = None,
    threads: int | None = None,
    expected_frames: int = 0,
    require_motion: bool = False,
) -> dict:
    """Render a Synfig project to a verified PNG sequence.

    Parameters
    ----------
    project : path to .sif, .sifz or .sfg
    output_root : base dir for render output (default: <project-dir>/renders)
    threads : render thread count (default: min(4, CPU count))
    expected_frames : if > 0, validate frame count matches
    require_motion : require at least two different pixel frames

    Returns
    -------
    dict with status, paths, frame records, and SHA-256 hashes.
    """
    project = Path(project).resolve()
    if project.suffix.lower() not in (".sif", ".sifz", ".sfg"):
        raise ValueError(f"Unsupported project format: {project}")
    if not project.is_file():
        raise FileNotFoundError(project)

    if threads is None:
        threads = max(1, min(4, os.cpu_count() or 1))

    project_dir = project.parent
    root = Path(output_root).resolve() if output_root else project_dir / "renders"
    stem = project.stem
    run_name = f"{stem}_{datetime.datetime.now().strftime('%Y%m%d_%H%M%S_%f')}_{uuid.uuid4().hex[:6]}"
    run_dir = root / run_name
    run_dir.mkdir(parents=True, exist_ok=True)

    t0 = time.perf_counter()

    # Version check
    ver = version(run_dir)

    # Canvas info
    info = canvas_info(project, run_dir)
    first_frame = info["frame_start"]
    last_frame = info["frame_end"]
    total_frames = last_frame - first_frame + 1
    if total_frames < 1:
        raise RuntimeError("Invalid render range.")
    if expected_frames and expected_frames != total_frames:
        raise RuntimeError(f"Canvas specifies {total_frames} frames, not {expected_frames}.")

    # Render
    output_pattern = str(run_dir / "frame.png")
    render_args = [
        "-i", str(project),
        "-T", str(threads),
        "-t", "png",
        "-o", output_pattern,
        "--sequence-separator", "_",
        "-a", "2",
        "--quiet",
    ]
    _run(render_args, run_dir, "render", cwd=project_dir)

    # Collect and validate frames
    files = sorted(run_dir.glob("frame*.png"), key=lambda p: p.name)
    if len(files) != total_frames:
        raise RuntimeError(
            f"Incomplete render: got {len(files)}, expected {total_frames}. See {run_dir}"
        )

    frame_records = []
    pixel_hashes = set()
    for f in files:
        match = re.match(r"^frame_(\d+)\.png$", f.name)
        if match:
            index = int(match.group(1))
        elif total_frames == 1 and f.name == "frame.png":
            index = first_frame
        else:
            raise RuntimeError(f"Unexpected PNG filename: {f.name}")

        decoded = _read_png_info(f)
        if decoded["width"] != info["width"] or decoded["height"] != info["height"]:
            raise RuntimeError(f"Wrong frame dimensions: {f.name}")

        file_hash = sha(f)
        pixel_hashes.add(decoded["pixel_sha256"])
        frame_records.append({
            "index": index,
            "name": f.name,
            "bytes": f.stat().st_size,
            "sha256": file_hash,
            "pixel_sha256": decoded["pixel_sha256"],
        })

    frame_records.sort(key=lambda r: r["index"])

    # Verify sequential
    for i in range(1, len(frame_records)):
        if frame_records[i]["index"] != frame_records[i-1]["index"] + 1:
            raise RuntimeError("PNG sequence has missing or duplicate frame numbers.")

    unique_frames = len(pixel_hashes)
    if require_motion and unique_frames < 2:
        raise RuntimeError("Frames are pixel-identical; no visible change was rendered.")

    elapsed = round(time.perf_counter() - t0, 3)

    manifest = {
        "Status": "PASS",
        "Project": str(project),
        "ProjectSha256": sha(project),
        "Synfig": str(SYNFIG),
        "Version": ver,
        "OutputDirectory": str(run_dir),
        "Width": info["width"],
        "Height": info["height"],
        "Fps": info["frame_rate"],
        "ExpectedFrames": total_frames,
        "ActualFrames": len(files),
        "FirstFrame": first_frame,
        "LastFrame": last_frame,
        "FirstFilenameIndex": frame_records[0]["index"],
        "LastFilenameIndex": frame_records[-1]["index"],
        "UniquePixelFrames": unique_frames,
        "ElapsedSeconds": elapsed,
        "RenderArguments": render_args,
        "Frames": frame_records,
    }

    manifest_path = run_dir / "render-manifest.json"
    _write_json(manifest_path, manifest)

    print(f"Rendered {len(files)} PNG frames ({info['width']} x {info['height']}); unique images: {unique_frames}")
    return manifest


def verify_manifest(manifest_path: str | Path) -> dict:
    """Re-read and validate an existing render manifest."""
    manifest_path = Path(manifest_path)
    manifest = json.loads(manifest_path.read_text(encoding="utf-8-sig"))
    if manifest["Status"] != "PASS":
        raise ValueError(f"Manifest status: {manifest['Status']}")
    folder = Path(manifest["OutputDirectory"])
    for f in manifest["Frames"]:
        path = folder / f["name"]
        if not path.is_file():
            raise FileNotFoundError(f"Missing frame: {path}")
        if sha(path) != f["sha256"]:
            raise ValueError(f"Hash mismatch: {path}")
        decoded = _read_png_info(path)
        if decoded["width"] != manifest["Width"] or decoded["height"] != manifest["Height"]:
            raise ValueError(f"Dimension mismatch: {path}")
    return {
        "status": "PASS",
        "manifest": str(manifest_path),
        "frames_verified": len(manifest["Frames"]),
    }


def make_cutout(
    sprite: str | Path,
    output: str | Path,
    *,
    pixels: int = 128,
    fps: int = 60,
    frames: int = 60,
) -> dict:
    """Build a Synfig SIF project with a periodic animated import layer.

    This is the pure-Python equivalent of asset_tools.make_synfig_cutout(),
    exposed as a CLI action.
    """
    output = Path(output)
    if output.exists():
        raise FileExistsError(output)
    output.parent.mkdir(parents=True, exist_ok=True)

    linked = output.with_name(output.stem + "_source.png")
    if linked.exists():
        raise FileExistsError(linked)
    shutil.copy2(sprite, linked)

    root = ET.Element("canvas", version="1.2", width=str(pixels), height=str(pixels),
        xres="2834.645669", yres="2834.645669",
        **{"view-box": "-1.5 1.5 1.5 -1.5", "antialias": "2", "fps": str(fps),
           "begin-time": "0f", "end-time": f"{frames-1}f", "bgcolor": "0 0 0 0"})

    ET.SubElement(root, "name").text = "Cutout animation — periodic breathing"
    ET.SubElement(root, "desc").text = f"Sprite import, {fps} Hz, {frames} frames, cyclic."

    layer = ET.SubElement(root, "layer", type="import", active="true", version="0.1",
                          desc="Sprite source")

    def value(name, kind, val):
        param = ET.SubElement(layer, "param", name=name)
        el = ET.SubElement(param, kind)
        if kind == "string":
            el.text = val
        else:
            el.set("value", str(val))

    value("z_depth", "real", 0)
    value("amount", "real", 1)
    value("blend_method", "integer", 0)
    value("filename", "string", linked.name)
    value("interpolation", "integer", 1)
    value("gamma_adjust", "real", 1)
    value("time_offset", "time", "0s")

    for name, corner in [("tl", (-1.15, 1.15)), ("br", (1.15, -1.15))]:
        animated = ET.SubElement(ET.SubElement(layer, "param", name=name), "animated", type="vector")
        for frame in range(-1, frames + 2):
            t = 2 * math.pi * frame / frames
            x = corner[0] + 0.10 * math.sin(t)
            y = corner[1] + 0.09 * math.cos(t)
            point = ET.SubElement(animated, "waypoint", time=f"{frame}f",
                                  before="linear", after="linear")
            vector = ET.SubElement(point, "vector")
            ET.SubElement(vector, "x").text = f"{x:.9f}"
            ET.SubElement(vector, "y").text = f"{y:.9f}"

    ET.indent(root)
    with output.open("xb") as f:
        f.write(ET.tostring(root, encoding="utf-8", xml_declaration=True))

    return {
        "project": str(output),
        "linked_sprite": str(linked),
        "fps": fps,
        "frames": frames,
        "pixels": pixels,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="action", required=True)

    sub.add_parser("version")

    p = sub.add_parser("info")
    p.add_argument("project", type=Path)

    p = sub.add_parser("render")
    p.add_argument("project", type=Path)
    p.add_argument("--output-root", type=Path, default=None)
    p.add_argument("--threads", type=int, default=None)
    p.add_argument("--expected-frames", type=int, default=0)
    p.add_argument("--require-motion", action="store_true")

    p = sub.add_parser("verify")
    p.add_argument("manifest", type=Path)

    p = sub.add_parser("make-cutout")
    p.add_argument("sprite", type=Path)
    p.add_argument("output", type=Path)
    p.add_argument("--pixels", type=int, default=128)
    p.add_argument("--fps", type=int, default=60)
    p.add_argument("--frames", type=int, default=60)

    args = parser.parse_args()

    if args.action == "version":
        print(version())
    elif args.action == "info":
        result = canvas_info(args.project)
        print(json.dumps(result, indent=2))
    elif args.action == "render":
        result = render(
            args.project,
            output_root=args.output_root,
            threads=args.threads,
            expected_frames=args.expected_frames,
            require_motion=args.require_motion,
        )
        print(json.dumps(result, indent=2))
    elif args.action == "verify":
        result = verify_manifest(args.manifest)
        print(json.dumps(result, indent=2))
    elif args.action == "make-cutout":
        result = make_cutout(
            args.sprite, args.output,
            pixels=args.pixels, fps=args.fps, frames=args.frames,
        )
        print(json.dumps(result, indent=2))


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print("ERROR: " + str(e), file=sys.stderr)
        sys.exit(1)
