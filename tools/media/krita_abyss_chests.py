"""Original layered 2D art, painted inside Krita's native Python/Qt runtime.

Uses no external images, generators, models or network. Run through the local
kritarunner worker; retain owned saved documents rather than closing them.
"""
from pathlib import Path
import json
import math
import random
import hashlib
from krita import Krita, InfoObject
from PyQt5.QtCore import Qt, QPointF, QRectF, QByteArray
from PyQt5.QtGui import QColor, QImage, QPainter, QPainterPath, QPolygonF, QPen, QLinearGradient, QRadialGradient


def color(value):
    return value if isinstance(value, (QColor, QLinearGradient, QRadialGradient)) else QColor(value)


def gradient(a, b, stops):
    g = QLinearGradient(*a, *b)
    for at, c in stops:
        g.setColorAt(at, QColor(c))
    return g


def polygon(p, points, fill, stroke=None, width=1):
    p.setBrush(color(fill) if fill else Qt.NoBrush)
    p.setPen(QPen(color(stroke), width, Qt.SolidLine, Qt.RoundCap, Qt.RoundJoin) if stroke else Qt.NoPen)
    p.drawPolygon(QPolygonF([QPointF(*v) for v in points]))


def line(p, points, ink, width=1):
    p.setPen(QPen(color(ink), width, Qt.SolidLine, Qt.RoundCap, Qt.RoundJoin))
    p.setBrush(Qt.NoBrush)
    path = QPainterPath(QPointF(*points[0]))
    for v in points[1:]:
        path.lineTo(*v)
    p.drawPath(path)


def ellipse(p, x, y, w, h, fill, stroke=None, width=1):
    p.setPen(QPen(color(stroke), width) if stroke else Qt.NoPen)
    p.setBrush(color(fill))
    p.drawEllipse(QRectF(x, y, w, h))


def curve(p, start, controls, ink, width=1):
    path = QPainterPath(QPointF(*start))
    for v in controls:
        path.cubicTo(*v)
    p.setBrush(Qt.NoBrush)
    p.setPen(QPen(color(ink), width, Qt.SolidLine, Qt.RoundCap, Qt.RoundJoin))
    p.drawPath(path)


class Master:
    def __init__(self, root, name, width, height, retained, scale=1):
        self.root, self.name, self.w, self.h, self.scale = root, name, width, height, scale
        self.doc = Krita.instance().createDocument(width, height, name, "RGBA", "U8", "", 72.)
        self.doc.setBatchmode(True)
        self.doc.setAutosave(False)
        for n in self.doc.rootNode().childNodes():
            n.remove()
        retained.append(self.doc)
        self.layers = []
        (root / (name + "-state-before.json")).write_text(json.dumps({"version": Krita.instance().version(),
            "document": name, "width": width, "height": height, "layers": []}), encoding="utf-8")

    def layer(self, name, draw):
        image = QImage(self.w, self.h, QImage.Format_ARGB32)
        image.fill(Qt.transparent)
        p = QPainter(image)
        p.setRenderHint(QPainter.Antialiasing, True)
        p.setRenderHint(QPainter.SmoothPixmapTransform, True)
        p.scale(self.scale, self.scale)
        draw(p)
        p.end()
        node = self.doc.createNode(name, "paintlayer")
        self.doc.rootNode().addChildNode(node, None)
        ptr = image.constBits()
        ptr.setsize(image.sizeInBytes())
        assert node.setPixelData(QByteArray(bytes(ptr)), 0, 0, self.w, self.h)
        self.layers.append(name)

    def save(self, runtime_size=None):
        self.doc.refreshProjection()
        self.doc.waitForDone()
        kra = self.root / (self.name + ".kra")
        assert not kra.exists(), "Fresh master required"
        assert self.doc.saveAs(str(kra))
        preview = self.doc.thumbnail(min(1024, self.w), min(768, self.h))
        assert preview.save(str(self.root / (self.name + "-preview.png")))
        png = self.root / (self.name + ".png")
        export = self.doc
        if runtime_size:
            export = self.doc.clone()
            export.setBatchmode(True)
            export.scaleImage(*runtime_size, 72, 72, "Bicubic")
            export.refreshProjection()
            export.waitForDone()
        info = InfoObject()
        info.setProperty("alpha", True)
        info.setProperty("compression", 6)
        assert export.exportImage(str(png), info)
        record = {"name": self.name, "master": str(kra), "png": str(png), "layers": self.layers,
                  "size": [self.w, self.h], "runtime_size": runtime_size or [self.w, self.h],
                  "kra_sha256": hashlib.sha256(kra.read_bytes()).hexdigest(),
                  "png_sha256": hashlib.sha256(png.read_bytes()).hexdigest(), "retained": True}
        (self.root / (self.name + "-receipt.json")).write_text(json.dumps(record, indent=2), encoding="utf-8")
        return record


def chests(root, retained):
    master = Master(root, "realm-treasure-chests", 1536, 1536, retained, 2)
    palettes = [
        dict(edge="#293532", front="#725038", light="#b99061", side="#46372c", top="#ad7d49", metal="#72827c", shine="#c1c3a4", dark="#303b37", accent="#79b78e"),
        dict(edge="#504338", front="#b69a6e", light="#e0cc9d", side="#7a654c", top="#d1b47e", metal="#b78e46", shine="#ffe2a0", dark="#76532c", accent="#58a9b4"),
        dict(edge="#171d26", front="#414653", light="#737d86", side="#272c38", top="#505d6a", metal="#777267", shine="#b8b4a0", dark="#34343a", accent="#ffac57")]

    def each(p, fn):
        for realm, pal in enumerate(palettes):
            for state in range(3):
                p.save();p.translate(state * 256, realm * 256)
                fn(p, realm, state, pal)
                p.restore()

    def shadow(p, realm, state, pal):
        g = QRadialGradient(QPointF(132, 190), 97)
        g.setColorAt(0, QColor(8, 14, 18, 105));g.setColorAt(1, QColor(8, 14, 18, 0))
        ellipse(p, 29, 145, 207, 88, g)

    def lid(p, realm, state, pal):
        if not state:
            return
        # The back hinge is fixed in every state and every realm.
        shape = [(116, 109), (91, 34), (98, 25), (201, 72), (224, 150), (216, 156)]
        if realm == 1:
            shape = [(115, 109), (92, 38), (94, 22), (108, 19), (201, 62), (214, 78), (226, 149), (216, 156)]
        if realm == 2:
            shape = [(115, 108), (93, 39), (99, 24), (143, 37), (201, 65), (214, 82), (228, 148), (216, 157)]
        polygon(p, shape, gradient((105, 30), (213, 151), [(0, pal["light"]), (.2, pal["top"]), (1, pal["side"])]), pal["edge"], 2.5)
        polygon(p, [(120, 104), (101, 40), (195, 82), (214, 142)], pal["side"], pal["metal"], 3)
        line(p, [(101, 30), (199, 75), (220, 146)], pal["shine"], 1.3)
        for k in range(3):
            line(p, [(111 + k * 4, 54 + k * 17), (201 + k * 4, 96 + k * 17)], pal["top"], 1)
        for k in (0, 1):
            a = 119 + k * 61
            line(p, [(a - 13, 46 + k * 27), (a + 5, 109 + k * 27)], pal["metal"], 7)
            line(p, [(a - 15, 46 + k * 27), (a + 3, 109 + k * 27)], pal["shine"], 1)

    def body(p, realm, state, pal):
        polygon(p, [(40, 138), (117, 101), (222, 148), (222, 188), (141, 229), (40, 180)], pal["edge"], pal["edge"], 2)
        polygon(p, [(45, 141), (141, 186), (141, 224), (45, 177)], gradient((50, 141), (147, 227), [(0, pal["light"]), (.3, pal["front"]), (1, pal["side"])]))
        polygon(p, [(141, 186), (217, 151), (217, 185), (141, 224)], gradient((151, 181), (218, 204), [(0, pal["front"]), (1, pal["side"])]))
        polygon(p, [(45, 137), (117, 103), (217, 149), (141, 185)], pal["top"], pal["metal"], 2)
        if state:
            polygon(p, [(54, 137), (118, 109), (207, 150), (141, 179)], pal["edge"])
            polygon(p, [(68, 139), (118, 117), (195, 150), (141, 172)], gradient((120, 116), (127, 177), [(0, "#171d22"), (1, pal["side"])]))
            line(p, [(49, 139), (141, 182), (215, 149)], pal["shine"], 1.6)
        if realm == 2:
            polygon(p, [(49, 146), (87, 161), (91, 197), (50, 175)], "#353e4b")
            polygon(p, [(93, 164), (135, 186), (135, 218), (99, 195)], "#4b535e")
            polygon(p, [(149, 189), (207, 162), (214, 180), (150, 215)], "#313846")

    def surface(p, realm, state, pal):
        front = QPainterPath();front.moveTo(47, 144);front.lineTo(138, 187);front.lineTo(138, 221);front.lineTo(47, 176);front.closeSubpath()
        p.save();p.setClipPath(front)
        if realm == 0:
            for k in range(13):
                y = 143 + k * 3.3
                curve(p, (43, y), [(73, y + 7, 99, y + 30, 143, y + 43)], "#4e3b2f", .65)
                curve(p, (46, y + 1.2), [(79, y + 13, 105, y + 35, 143, y + 44)], "#d7ac72", .35)
            ellipse(p, 74, 167, 14, 7, "#604632")
            curve(p, (69, 166), [(77, 162, 88, 169, 92, 177)], "#b58956", .8)
        elif realm == 1:
            for k in range(7):
                x = 53 + k * 12;y = 148 + k * 5.6
                line(p, [(x, y + 5), (x + 4, y + 1), (x + 8, y + 9), (x + 4, y + 13), (x, y + 5)], "#e8d6ab", 1)
                line(p, [(x + 4, y + 14), (x + 4, y + 22)], "#7d674a", .8)
            line(p, [(48, 172), (138, 214)], "#d4b677", 2)
        else:
            for points in [[(53, 151), (73, 166), (69, 177), (90, 191), (87, 199)], [(111, 181), (104, 190), (120, 203), (114, 216)]]:
                line(p, points, "#663a33", 5)
                line(p, points, "#d76b37", 2)
                line(p, points, "#ffbf69", .65)
            line(p, [(73, 166), (82, 165), (88, 174)], "#f38d45", .8)
        p.restore()
        for k in (0, 1, 2):
            y = 195 + k * 8
            line(p, [(146, y), (213, y - 31)], pal["edge"], .65)

    def closed_lid(p, realm, state, pal):
        if state:
            return
        if realm == 2:
            polygon(p, [(39, 137), (48, 113), (115, 76), (170, 92), (219, 124), (225, 144), (141, 188)], pal["edge"], pal["edge"], 2)
            polygon(p, [(44, 134), (57, 112), (116, 82), (165, 98), (142, 165)], "#65717c")
            polygon(p, [(142, 165), (165, 98), (215, 127), (219, 140), (141, 182)], "#3a4351")
            polygon(p, [(57, 112), (116, 82), (165, 98), (144, 117)], "#8b9292")
            line(p, [(58, 113), (143, 164), (216, 133)], "#a2a699", 1)
            line(p, [(92, 107), (110, 122), (105, 134), (131, 152)], "#c66338", 2.5)
            line(p, [(93, 107), (110, 122), (106, 133)], "#ffbf70", .7)
        else:
            path = QPainterPath(QPointF(41, 141))
            path.cubicTo(32, 112, 75, 92, 116, 77 if realm == 0 else 69)
            path.cubicTo(148, 84, 206, 105, 219, 124)
            path.cubicTo(224, 130, 225, 139, 218, 146)
            path.lineTo(141, 185);path.closeSubpath()
            p.setBrush(gradient((125, 78), (126, 181), [(0, pal["light"]), (.4, pal["top"]), (1, pal["side"])]))
            p.setPen(QPen(QColor(pal["edge"]), 2.2));p.drawPath(path)
            p.save();p.setClipPath(path)
            for k in range(11):
                y = 84 + k * 7
                curve(p, (27, y), [(78, y - 4, 113, y + 40, 231, y + 91)], "#634932" if realm == 0 else "#b49a63", .65)
            if realm == 1:
                for k in range(9):
                    x = 56 + k * 17;y = 110 + (k - 3) * 4
                    line(p, [(x, y), (x + 7, y - 2), (x + 12, y + 4), (x + 5, y + 7), (x, y)], "#f9e5b4", 1.4)
            p.restore()
            curve(p, (43, 132), [(78, 141, 107, 162, 141, 177), (164, 168, 194, 153, 217, 141)], pal["edge"], 2)
            curve(p, (47, 115), [(61, 101, 91, 89, 117, 80), (144, 88, 191, 105, 211, 122)], pal["shine"], .8)
        for shift in (0, 66):
            curve(p, (57 + shift, 144 + shift * .43), [(43 + shift, 128 + shift * .43, 53 + shift, 110 + shift * .43, 75 + shift, 100 + shift * .43)], pal["dark"], 11 if realm != 2 else 13)
            curve(p, (57 + shift, 144 + shift * .43), [(46 + shift, 128 + shift * .43, 55 + shift, 111 + shift * .43, 75 + shift, 102 + shift * .43)], pal["metal"], 6)
            curve(p, (54 + shift, 141 + shift * .43), [(46 + shift, 125 + shift * .43, 57 + shift, 111 + shift * .43, 72 + shift, 105 + shift * .43)], pal["shine"], 1.2)

    def fittings(p, realm, state, pal):
        for u in (0.13, .82):
            x, y = 46 + 94 * u, 143 + 44 * u
            polygon(p, [(x, y), (x + 9, y + 4), (x + 9, y + 36), (x, y + 32)], pal["dark"], pal["edge"], 1)
            line(p, [(x + 2, y + 2), (x + 2, y + 29)], pal["metal"], 4)
            line(p, [(x + 1, y + 1), (x + 1, y + 28)], pal["shine"], .8)
            for v in (7, 25):
                ellipse(p, x + 1, y + v, 3, 3, pal["shine"], pal["dark"], .4)
        line(p, [(45, 177), (141, 225), (217, 185)], pal["metal"], 3)
        line(p, [(46, 175), (141, 222), (215, 183)], pal["shine"], .8)
        if realm == 1:
            ellipse(p, 93, 165, 18, 19, pal["metal"], pal["shine"], 1)
            polygon(p, [(102, 166), (108, 176), (102, 183), (97, 173)], pal["accent"], "#e9d7a1", .7)
            for side in (-1, 1):
                line(p, [(102 + side * 9, 172), (102 + side * 15, 173), (102 + side * 18, 179)], pal["metal"], 1.6)
        else:
            polygon(p, [(93, 162), (106, 168), (108, 188), (94, 183)], pal["metal"], pal["shine"], .8)
            polygon(p, [(100, 167), (105, 174), (101, 181), (97, 175)], pal["accent"], pal["dark"], .7)
            if realm == 0:
                line(p, [(101, 175), (101, 182)], "#253d31", 1.4)
        if realm == 0:
            curve(p, (49, 112), [(41, 133, 55, 144, 50, 162), (44, 173, 56, 185, 73, 190)], "#485e43", 2)
            for x, y in ((46, 127), (51, 151), (49, 170), (62, 185), (188, 149), (207, 141)):
                ellipse(p, x - 4, y - 2, 9, 4, "#6a8d56", "#344b35", .5)
                line(p, [(x - 3, y), (x + 3, y)], "#a1b078", .5)
            for k in range(32):
                x = 42 + (k * 17 % 31);y = 127 + (k * 13 % 18) + (x - 42) * .3
                ellipse(p, x, y, 2.3, 1.4, "#668257")
        if realm == 2:
            for x, y in ((170, 178), (187, 170), (203, 162)):
                line(p, [(x, y), (x + 3, y + 3), (x + 3, y + 11), (x - 1, y + 8)], "#c77943", .8)

    def contents(p, realm, state, pal):
        if state != 1:
            return
        for x, y in ((86, 140), (101, 144), (117, 151), (133, 158), (151, 156), (169, 148), (133, 140), (149, 144), (116, 137), (103, 139)):
            ellipse(p, x - 6, y - 3, 13, 7, "#7d5a31")
            ellipse(p, x - 6, y - 5, 13, 6, "#dcb56a", "#f0d98e", .6)
            line(p, [(x - 3, y - 3), (x + 2, y - 2)], "#fff0b4", .6)
        gem = ("#5eafa0", "#4eabd1", "#e97640")[realm]
        polygon(p, [(134, 141), (141, 131), (149, 141), (144, 153)], gem, "#d1dfd0", .7)
        line(p, [(141, 132), (141, 143), (144, 151)], "#f1f2cf", .7)

    for title, fn in [("Contact shadows", shadow), ("Raised lids", lid), ("Carved bodies and interiors", body),
                      ("Wood grain, inlays and obsidian cracks", surface), ("Closed lids", closed_lid),
                      ("Fittings, locks, vines and runes", fittings), ("Unclaimed treasure", contents)]:
        master.layer(title, lambda p, fn=fn: each(p, fn))
    return master.save((768, 768))


def cliff(root, retained):
    master = Master(root, "caldera-abyss-cliff", 1024, 768, retained)
    def faded(p, fn):
        fn(p)
        p.setCompositionMode(QPainter.CompositionMode_DestinationIn)
        p.fillRect(0, 0, 1024, 768, gradient((0, 0), (0, 768), [(0, "#ffffffff"), (.42, "#ffffffff"), (.78, "#a0ffffff"), (1, "#00ffffff")]))
    def base(p):
        p.fillRect(0, 0, 1024, 768, gradient((0, 0), (0, 768), [(0, "#70676b"), (.12, "#554e5b"), (.48, "#32303f"), (1, "#171a2a")]))
    def pillars(p):
        rng = random.Random(4403)
        for n in range(-1, 17):
            x = n * 65;r = rng.uniform(-22, 22)
            polygon(p, [(x, 0), (x + 66, 0), (x + 48 + r, 220), (x + 69, 440), (x + 34, 765), (x - 14, 765), (x + 6, 321)],
                    gradient((x, 0), (x + 74, 0), [(0, "#655c69"), (.24, "#4e4959"), (.72, "#3e3b4e"), (1, "#303143")]))
            line(p, [(x + 63, 0), (x + 48 + r, 220), (x + 66, 438), (x + 34, 700)], "#282a39", 1.5)
    def strata(p):
        rng = random.Random(80)
        for row in range(7):
            y = 22 + row * 91
            pts = [(x, y + rng.uniform(-30, 30)) for x in range(-25, 1060, 57)]
            line(p, pts, "#2d2d3c", 1.1)
            line(p, [(x, yy + 3) for x, yy in pts], "#675661", .65)
        for n in range(180):
            x, y = rng.uniform(0, 1024), rng.uniform(3, 540)
            line(p, [(x, y), (x + rng.uniform(2, 8), y - 2)], "#8b7781", .6)
        for x in (112, 379, 686, 864):
            pts = [(x, 20), (x + 12, 79), (x + 3, 112), (x + 20, 170), (x + 12, 216)]
            line(p, pts, "#302b3b", 5);line(p, pts, "#ad664f", 1)
    for name, fn in [("Depth and reflected ash light", base), ("Broken basalt columns", pillars), ("Strata, pores and fissures", strata)]:
        master.layer(name, lambda p, fn=fn: faded(p, fn))
    return master.save()


def panorama(root, retained):
    master = Master(root, "caldera-abyss-sky", 2048, 1024, retained)
    def air(p):
        p.fillRect(0, 0, 2048, 1024, gradient((0, 0), (0, 1024), [(0, "#111d2a"), (.35, "#343243"), (.66, "#4f414c"), (.86, "#6b4646"), (1, "#332b3e")]))
        for x, y, radius in ((410, 645, 590), (1450, 780, 670)):
            g = QRadialGradient(QPointF(x, y), radius);g.setColorAt(0, QColor(189, 95, 52, 55));g.setColorAt(1, QColor(70, 37, 44, 0));p.fillRect(0, 0, 2048, 1024, g)
    def ridges(p):
        rng = random.Random(512)
        for layer in range(4):
            y = 380 + layer * 130
            pts = [(-100, 1024), (-100, y + 90)]
            for n in range(7):
                x = n * 360 - 180;h = rng.uniform(75, 150)
                pts += [(x, y + 75), (x + 38, y + 49), (x + 61, y - h*.5), (x + 103, y - h*.55),
                        (x + 119, y - h), (x + 185, y - h*.97), (x + 251, y - h*.41),
                        (x + 288, y - h*.45), (x + 310, y + 48), (x + 360, y + 78)]
            pts += [(2200, 1024)]
            shades = [("#685660", "#423d50"), ("#5a4859", "#363345"), ("#493c50", "#292b40"), ("#393247", "#22273b")][layer]
            polygon(p, pts, gradient((0, y-170), (0, y+280), [(0, shades[0]), (1, shades[1])]))
            clip = QPainterPath(QPointF(*pts[0]))
            for point in pts[1:]:clip.lineTo(*point)
            clip.closeSubpath();p.save();p.setClipPath(clip)
            for n in range(70):
                x = n * 33 + rng.uniform(-15, 15)
                line(p, [(x, y-130), (x + rng.uniform(-12, 12), y+25), (x-13, y+170), (x+7, y+280)], QColor(25, 28, 43, 60), rng.uniform(1, 4))
            for row in range(5):
                yy = y + row * 33
                line(p, [(x, yy+rng.uniform(-15, 15)) for x in range(-100, 2200, 70)], QColor(153, 115, 116, 25), 1)
            p.restore()
    def fire(p):
        rng = random.Random(94)
        for n in range(9):
            x = rng.uniform(-80, 2100);y = rng.uniform(730, 990)
            controls = [(x+53, y-12, x+73, y+35, x+118, y+22), (x+167, y+9, x+219, y+43, x+315, y+47)]
            curve(p, (x, y), controls, QColor(140, 65, 48, 65), 18)
            curve(p, (x, y), controls, QColor(188, 96, 59, 100), 3)
            curve(p, (x, y), controls, QColor(230, 164, 103, 110), .8)
    def haze(p):
        for y, alpha in ((405, 65), (570, 72), (750, 62), (930, 44)):
            g = gradient((0, y - 130), (0, y + 130), [(0, "#004c485c"), (.5, QColor(126, 108, 119, alpha).name(QColor.HexArgb)), (1, "#004c485c")])
            p.fillRect(0, y - 130, 2048, 260, g)
    for name, fn in [("Ash sky and distant furnace glow", air), ("Distant caldera ridges", ridges), ("Deep molten fissures", fire), ("Layers of suspended ash", haze)]:
        master.layer(name, fn)
    return master.save()


def distance(root, retained):
    master = Master(root, "caldera-abyss-distance", 2048, 1024, retained)
    def crags(p):
        rng = random.Random(2028)
        for n in range(12):
            x = n * 181 + rng.uniform(-30, 30);top = rng.uniform(490, 690);w = rng.uniform(80, 160)
            pts = [(x - w, 1024), (x - w * .7, top + 160), (x - w * .5, top + 95), (x - w * .35, top + 100), (x - w * .22, top + 22), (x + w * .12, top + 18), (x + w * .28, top + 60), (x + w * .35, top + 145), (x + w * .7, top + 175), (x + w, 1024)]
            polygon(p, pts, gradient((x, top), (x, 1024), [(0, "#af3d3548"), (.5, "#be302c40"), (1, "#00282439")]))
            line(p, [(x - w * .1, top + 3), (x - w * .21, top + 125), (x - w * .13, top + 219)], "#72606c", 1)
    master.layer("Isolated pinnacles below the plateau", crags)
    return master.save()


def produce(output, retained):
    output.mkdir(parents=True, exist_ok=True)
    # Resume the first completed native master after the export type check.
    existing = output / "realm-treasure-chests.kra"
    if existing.exists() and not (output / "realm-treasure-chests.png").exists():
        original = hashlib.sha256(existing.read_bytes()).hexdigest()
        d = Krita.instance().openDocument(str(existing));retained.append(d);d.setBatchmode(True)
        export = d.clone();export.setBatchmode(True);retained.append(export)
        export.scaleImage(768, 768, 72, 72, "Bicubic");export.refreshProjection();export.waitForDone()
        info = InfoObject();info.setProperty("alpha", True);info.setProperty("compression", 6)
        target = output / "realm-treasure-chests.png"
        assert export.exportImage(str(target), info)
        assert hashlib.sha256(existing.read_bytes()).hexdigest() == original
        chest = {"name": "realm-treasure-chests", "master": str(existing), "png": str(target),
                 "layers": [n.name() for n in d.topLevelNodes()], "size": [1536, 1536], "runtime_size": [768, 768],
                 "kra_sha256": original, "png_sha256": hashlib.sha256(target.read_bytes()).hexdigest(), "resumed_export": True, "retained": True}
        (output / "realm-treasure-chests-receipt.json").write_text(json.dumps(chest, indent=2), encoding="utf-8")
    else:
        chest = chests(output, retained)
    records = [chest, cliff(output, retained), panorama(output, retained), distance(output, retained)]
    return {"assets": records, "tool": "Krita native Python / Qt painting", "external_inputs": [], "audio": False}
