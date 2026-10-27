"""Bake the bundled OFL Cinzel variable font into the game's bold title face.

Authoring only: Python + fonttools==4.60.1. No runtime dependency/download.
The original variable font and its OFL license remain unchanged.
"""
from pathlib import Path
import hashlib
import io
import json

from fontTools.ttLib import TTFont
from fontTools.varLib.instancer import instantiateVariableFont


def main():
    root = Path(__file__).resolve().parents[2]
    source = root / "assets/Cinzel.ttf"
    target = root / "assets/Cinzel-Bold.ttf"
    font = TTFont(source, recalcTimestamp=False)
    bold = instantiateVariableFont(font, {"wght": 700}, inplace=False, updateFontNames=True)
    output = io.BytesIO()
    bold.save(output)
    if target.exists() and target.read_bytes() != output.getvalue():
        raise FileExistsError("Back up the existing font before replacing it: " + str(target))
    if not target.exists():
        target.write_bytes(output.getvalue())
    assert "fvar" not in bold and bold["OS/2"].usWeightClass == 700
    required = "ĄĆĘŁŃÓŚŹŻąćęłńóśźż"
    assert all(ord(char) in bold.getBestCmap() for char in required)
    report = {
        "source": "assets/Cinzel.ttf", "output": "assets/Cinzel-Bold.ttf",
        "source_sha256": hashlib.sha256(source.read_bytes()).hexdigest(),
        "output_sha256": hashlib.sha256(target.read_bytes()).hexdigest(),
        "weight": 700, "license": "assets/Cinzel-LICENSE.txt (SIL OFL 1.1)",
        "method": "fontTools 4.60.1 varLib.instancer; genuine static outlines",
        "polish_glyphs": required, "external_assets": False,
    }
    report_dir = root / "art/typography"
    report_dir.mkdir(parents=True, exist_ok=True)
    (report_dir / "provenance.json").write_text(
        json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    print(json.dumps(report, ensure_ascii=True))


if __name__ == "__main__":
    main()
