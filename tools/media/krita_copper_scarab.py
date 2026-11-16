"""Native Krita masters for the copper scarab material and its wave crest."""
from pathlib import Path
import math
import random

from PyQt5.QtCore import QRectF, Qt
from PyQt5.QtGui import QColor, QImage, QLinearGradient, QPainterPath

from krita_abyss_chests import Master, curve, ellipse, line


def produce(output, retained, inputs):
    output.mkdir(parents=True, exist_ok=False)
    previous = QImage(str(inputs / "previous-atlas.png"))
    copper = QImage(str(inputs / "copper-source.png"))
    if previous.size().width() != 512 or previous.height() not in (512, 640) or copper.isNull():
        raise ValueError("Missing source or unexpected atlas dimensions")
    material = Master(output, "skarabeusz-miedziany-atlas", 512, previous.height(), retained)
    material.layer("Pozostałe materiały", lambda p: p.drawImage(0, 0, previous))
    tile = QRectF(128, 256, 128, 128)

    def shell(painter):
        # Preserve a little of the previous local ComfyUI tile as an underpaint.
        painter.setOpacity(.42)
        painter.fillRect(tile, QColor("#633e30"))
        painter.setOpacity(.76)
        painter.drawImage(tile, copper, QRectF(0, 0, copper.width(), copper.height()))

    def etching(painter):
        painter.setClipRect(tile)
        painter.translate(128, 256)
        for y in (25, 59, 99):
            curve(painter, (0, y+13), [(37, y+3, 49, y+7, 78, y-11), (92, y-19, 105, y-19, 128, y-23)],
                  QColor(53, 36, 31, 155), 2.5)
            curve(painter, (0, y+11), [(37, y+1, 49, y+5, 78, y-13), (92, y-21, 105, y-21, 128, y-25)],
                  QColor(246, 185, 120, 150), 1.15)
        rng = random.Random(9116)
        for _ in range(58):
            x, y = rng.uniform(1, 127), rng.uniform(1, 127)
            ellipse(painter, x, y, rng.uniform(.7, 2.3), rng.uniform(.6, 1.3), QColor(73, 96, 83, 105))

    material.layer("Miedź i patyna", shell)
    material.layer("Żłobienia pancerza i ślady utlenienia", etching)
    material.save()

    wave = Master(output, "miedziana-fala-grzbiet", 384, 192, retained, 2)

    def curl(painter):
        shape = QPainterPath()
        shape.moveTo(6, 81)
        shape.cubicTo(44, 57, 67, 73, 101, 34)
        shape.cubicTo(119, 14, 136, 24, 140, 30)
        shape.cubicTo(120, 20, 124, 53, 162, 57)
        shape.cubicTo(171, 58, 182, 59, 189, 67)
        shape.cubicTo(145, 57, 117, 92, 72, 80)
        shape.cubicTo(42, 74, 30, 87, 6, 81)
        fill = QLinearGradient(85, 28, 94, 85)
        for at, value in ((0, "#fff0b4"), (.22, "#df9d50"), (.47, "#bd6d3d"),
                          (.73, "#7b4835"), (1, QColor(113, 58, 39, 0))):
            fill.setColorAt(at, QColor(value))
        painter.setPen(Qt.NoPen)
        painter.setBrush(fill)
        painter.drawPath(shape)

    def details(painter):
        curve(painter, (19, 78), [(55, 62, 71, 73, 102, 38), (116, 23, 125, 24, 135, 28)], QColor(255, 220, 142, 230), 2)
        curve(painter, (59, 78), [(92, 77, 111, 48, 124, 50), (137, 54, 143, 64, 179, 66)], QColor(246, 175, 103, 155), 1.8)
        curve(painter, (38, 81), [(63, 73, 97, 90, 135, 66)], QColor(104, 183, 160, 190), 1.4)
        for x, y, size in ((46, 55, 3), (88, 41, 2), (104, 25, 2), (121, 16, 3), (151, 48, 2), (175, 52, 2)):
            line(painter, [(x-size, y+size*.6), (x, y), (x+size, y-.7)], QColor(255, 214, 151, 160), 1.25)

    wave.layer("Grzbiet pyłu i miedzianej energii", curl)
    wave.layer("Żar, patyna i odpryski", details)
    wave.save((192, 96))


def run(args):
    inputs = Path(args[0]).resolve()
    retained = []
    produce(inputs / "masters", retained, inputs)
