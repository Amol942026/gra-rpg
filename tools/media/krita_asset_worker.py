"""Run inside the official kritarunner; local file jobs, no server or playback.

The worker retains its owned saved documents, avoiding edited-project Close.
Jobs and outputs must be inside C:/AI2D. No foreign documents are touched.
"""
from pathlib import Path
import importlib
import json
import time
import traceback
from krita import Krita, InfoObject
from PyQt5.QtCore import QCoreApplication, QByteArray
from PyQt5.QtGui import QImage, QColor

OWNED = []


def state():
    return {"version": Krita.instance().version(), "owned_documents": [
        {"name": d.name(), "file": d.fileName(), "layers": [n.name() for n in d.topLevelNodes()]}
        for d in OWNED]}


def run(args):
    root = Path(args[0]).resolve()
    assert root.is_relative_to(Path("C:/AI2D"))
    root.mkdir(parents=True, exist_ok=True)
    (root / "ready.json").write_text(json.dumps(state()), encoding="utf-8")
    while True:
        for job in root.glob("job-*.json"):
            if job.name.endswith(".receipt.json"):
                continue
            receipt = job.with_suffix(".receipt.json")
            if receipt.exists():
                continue
            command = json.loads(job.read_text(encoding="utf-8"))
            output = Path(command["output"]).resolve()
            assert output.is_relative_to(Path("C:/AI2D"))
            before = state()
            try:
                module = importlib.reload(importlib.import_module(command["module"]))
                result = module.produce(output, OWNED)
                result.update(status="PASS", before=before, after=state())
            except Exception:
                result = {"status": "FAIL", "before": before, "error": traceback.format_exc(), "after": state()}
            receipt.write_text(json.dumps(result, indent=2), encoding="utf-8")
        QCoreApplication.processEvents()
        time.sleep(.1)
