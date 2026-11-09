"""Offline acceptance of the actual native Krita master and runtime export."""
from pathlib import Path
import argparse
import hashlib
import json
import zipfile
import xml.etree.ElementTree as ET
from PIL import Image
import numpy as np

parser = argparse.ArgumentParser()
parser.add_argument("--source", type=Path, required=True)
parser.add_argument("--output", type=Path, required=True)
parser.add_argument("--no-audio", action="store_true", required=True)
args = parser.parse_args()
repo = Path(__file__).resolve().parents[2]
receipt = json.loads((args.source / "caldera-worn-basalt-trail-receipt.json").read_text(encoding="utf-8"))
checks = []


def check(ok, label):
    checks.append({"pass": bool(ok), "label": label})
    if not ok:
        raise AssertionError(label)


master = Path(receipt["master"])
export = Path(receipt["png"])
runtime = repo / "assets/volcano/road.png"
check(hashlib.sha256(master.read_bytes()).hexdigest() == receipt["kra_sha256"], "saved native KRA hash")
check(hashlib.sha256(export.read_bytes()).hexdigest() == receipt["png_sha256"], "saved native PNG hash")
check(runtime.read_bytes() == export.read_bytes(), "runtime is the exact native Krita export")
with zipfile.ZipFile(master) as archive:
    check(archive.testzip() is None, "native KRA ZIP integrity")
    xml = ET.fromstring(archive.read("maindoc.xml"))
    check(sum(node.get("nodetype") == "paintlayer" for node in xml.iter()) == 4, "four editable native paint layers")
    check(archive.read("mimetype") == b"application/x-krita", "native Krita document type")
    with archive.open("mergedimage.png") as file:
        native = Image.open(file).convert("RGBA")
        native.load()
        check(native.size == (1024, 1024), "decoded full-size native painting")
image = Image.open(runtime).convert("RGBA")
image.load()
check(image.size == (512, 512), "runtime tile dimensions and unchanged texture budget")
check(image.getextrema()[3] == (255, 255), "opaque material for continuous distance-mask blending")
pixels = np.asarray(image, dtype=float)[:, :, :3]
check(float(pixels.std(axis=(0, 1)).mean()) > 3, "visible painted gravel and material detail")
edge_x = float(np.abs(pixels[:, 0] - pixels[:, -1]).mean())
edge_y = float(np.abs(pixels[0] - pixels[-1]).mean())
interior_x = float(np.abs(np.diff(pixels, axis=1)).mean())
interior_y = float(np.abs(np.diff(pixels, axis=0)).mean())
check(edge_x < interior_x * 1.7 + 2, "horizontal wrapped seam stays within local paint variation")
check(edge_y < interior_y * 1.7 + 2, "vertical wrapped seam stays within local paint variation")
before = repo / "art/pipeline/backups/terrain-roads-20261108/road.png"
check(before.is_file() and before.read_bytes() != runtime.read_bytes(), "previous working material remains backed up")
result = {"status": "PASS", "checks": checks, "master": str(master), "runtime_sha256": hashlib.sha256(runtime.read_bytes()).hexdigest(), "seam_mae": [edge_x, edge_y], "interior_mae": [interior_x, interior_y], "rgba8_bytes": 512 * 512 * 4, "local_only": True, "external_inputs": [], "audio_opened": False}
args.output.write_text(json.dumps(result, indent=2), encoding="utf-8")
print(json.dumps({"status": "PASS", "checks": len(checks), "seam_mae": [edge_x, edge_y]}))
