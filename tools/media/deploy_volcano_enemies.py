"""Deployment and rollback manager for Caldera redesigned enemy graphics.

Usage:
    python tools/media/deploy_volcano_enemies.py status
    python tools/media/deploy_volcano_enemies.py apply
    python tools/media/deploy_volcano_enemies.py restore
    python tools/media/deploy_volcano_enemies.py verify
"""
import argparse, hashlib, json, os, shutil, sys
from pathlib import Path
from PIL import Image

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
ASSETS_DIR = REPO_ROOT / "assets/volcano"
STAGE_DIR = REPO_ROOT / "art/volcano/replacement-stage/assets/volcano"
BACKUP_DIR = REPO_ROOT / "art/pipeline/backups/volcano-enemies-before-redesign"

NAMES = ['slag-crawler', 'lava-salamander', 'fire-imp', 'ash-wraith', 'obsidian-sentinel', 'ember-drake', 'varkhor']

def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()

def backup_original():
    BACKUP_DIR.mkdir(parents=True, exist_ok=True)
    manifest_p = BACKUP_DIR / "backup-manifest.json"
    if manifest_p.exists():
        return # already backed up
    receipt = {}
    for name in NAMES:
        src = ASSETS_DIR / f"{name}.png"
        dst = BACKUP_DIR / f"{name}.png"
        shutil.copy2(src, dst)
        receipt[name] = {"sha256": sha(src), "bytes": src.stat().st_size}
    with open(manifest_p, "w", encoding="utf-8") as f:
        json.dump(receipt, f, indent=2)
    print("Created original backup in:", BACKUP_DIR)

def apply():
    backup_original()
    applied = {}
    for name in NAMES:
        src = STAGE_DIR / f"{name}.png"
        if not src.exists():
            raise FileNotFoundError(f"Staged file missing: {src}")
        dst = ASSETS_DIR / f"{name}.png"
        shutil.copy2(src, dst)
        applied[name] = {"sha256": sha(dst), "bytes": dst.stat().st_size}
        print(f"Applied {name}.png -> {dst}")
    with open(ASSETS_DIR / "redesign-receipt.json", "w", encoding="utf-8") as f:
        json.dump({"status": "APPLIED", "files": applied}, f, indent=2)
    print("SUCCESS: New masterpiece enemy graphics successfully applied to assets/volcano/!")

def restore():
    manifest_p = BACKUP_DIR / "backup-manifest.json"
    if not manifest_p.exists():
        raise RuntimeError("No backup manifest found!")
    manifest = json.loads(manifest_p.read_text(encoding="utf-8"))
    for name in NAMES:
        src = BACKUP_DIR / f"{name}.png"
        dst = ASSETS_DIR / f"{name}.png"
        shutil.copy2(src, dst)
        if sha(dst) != manifest[name]["sha256"]:
            raise RuntimeError(f"Hash mismatch after restoring {name}.png")
        print(f"Restored {name}.png -> {dst}")
    receipt_p = ASSETS_DIR / "redesign-receipt.json"
    if receipt_p.exists():
        receipt_p.unlink()
    print("SUCCESS: Original assets/volcano/ graphics restored successfully!")

def verify():
    results = {}
    for name in NAMES:
        p = ASSETS_DIR / f"{name}.png"
        if not p.exists():
            raise FileNotFoundError(f"Missing {p}")
        im = Image.open(p)
        expected_cell = 256 if name == 'varkhor' else 192
        expected_w = expected_cell * 8
        expected_h = expected_cell * 6
        if im.size != (expected_w, expected_h):
            raise ValueError(f"Wrong size for {name}: {im.size}, expected ({expected_w}, {expected_h})")
        if im.mode != "RGBA":
            raise ValueError(f"Wrong mode for {name}: {im.mode}")
        results[name] = {
            "size": im.size,
            "cell": expected_cell,
            "sha256": sha(p),
            "status": "VALID"
        }
        print(f"Verified {name}.png: {im.size} RGBA, cell={expected_cell} OK")
    return results

def status():
    print(f"Target directory: {ASSETS_DIR}")
    print(f"Staging directory: {STAGE_DIR} (exists: {STAGE_DIR.exists()})")
    print(f"Backup directory: {BACKUP_DIR} (exists: {BACKUP_DIR.exists()})")
    receipt_p = ASSETS_DIR / "redesign-receipt.json"
    is_applied = receipt_p.exists()
    print(f"Active state: {'NEW MASTERPIECE REDESIGN' if is_applied else 'ORIGINAL'}")
    for name in NAMES:
        p = ASSETS_DIR / f"{name}.png"
        print(f" - {name}.png: {p.stat().st_size} bytes, sha256={sha(p)[:12]}...")

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=["status", "apply", "restore", "verify"])
    args = parser.parse_args()

    if args.action == "status":
        status()
    elif args.action == "apply":
        apply()
        verify()
    elif args.action == "restore":
        restore()
        verify()
    elif args.action == "verify":
        verify()

if __name__ == "__main__":
    main()
