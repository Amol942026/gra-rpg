"""Paint the new directional Emberfall imp in the native Krita worker."""

import math
from pathlib import Path

from PyQt5.QtGui import QColor, QImage, QPainterPath, QPen

from krita_abyss_chests import Master, curve, ellipse, gradient, line, polygon


CELL = 192
ANGLES = (0, math.atan(.5), math.pi / 2, math.pi - math.atan(.5), math.pi,
          math.pi + math.atan(.5), 3 * math.pi / 2, math.tau - math.atan(.5))
LAYERS = ("Ogon i kolce", "Lewa noga", "Prawa noga", "Głowa, tułów i uprząż", "Lewa dłoń", "Prawa dłoń")
INK = "#24191f"
SKIN = ("#df9a69", "#a95735", "#4b272b")
SHADE_SKIN = ("#9f6347", "#753c30", "#38242a")
BASALT = ("#85837e", "#4d4244", "#211d25")
HORN = ("#b7a594", "#665455", "#2a222e")
LEATHER = ("#8b715a", "#5b443c", "#30262c")


def bezier(start, *edges):
    path = QPainterPath()
    path.moveTo(*start)
    for edge in edges:
        if len(edge) == 2:
            path.lineTo(*edge)
        else:
            path.cubicTo(*edge)
    path.closeSubpath()
    return path


def surface(p, path, palette, texture):
    box = path.boundingRect()
    p.fillPath(path, gradient((box.left(), box.top()), (box.right(), box.bottom()),
                              [(0, palette[0]), (.47, palette[1]), (1, palette[2])]))
    if texture is not None:
        p.save()
        p.setClipPath(path)
        p.setOpacity(.29)
        p.drawImage(box, texture)
        p.restore()
    p.strokePath(path, QPen(QColor(INK), .8))


def ember_line(p, points, lit=True):
    line(p, points, "#562b2c", 1.9)
    line(p, points, "#ef7941" if lit else "#b3543b", .85)
    if lit:
        line(p, points[:2], "#ffd473", .42)


def tail(p, forward, across, texture):
    x = 64 + max(-45, min(45, -forward * 47 + across * 19))
    side = 1 if x > 64 else -1
    mid = 64 + (x - 64) * .72
    body = bezier((61, 83), (mid, 97, x-side*7, 112, x, 96),
                  (x+side*7, 81, x+side*3, 77, x+side*2, 70),
                  (x+side*8, 77, x+side*12, 89, x+side*6, 99),
                  (x, 117, mid, 113, 62, 94))
    surface(p, body, SHADE_SKIN, texture)
    curve(p, (63, 90), [(mid, 103, x-side*4, 113, x+side*2, 98)], "#bd6843", .95)
    for i in range(1, 5):
        u = 64 + (x-64)*i/5
        v = 91 + i * (5.4-i*.50)
        curve(p, (u-side*.8, v-1.4), [(u+side*1.8, v-.5, u+side*1.3, v+1, u+side*.3, v+2.1)], "#663336", .85)
    surface(p, bezier((x-side*1, 82), (x-side*10, 75), (x+side*2, 62),
                      (x+side*8, 78), (x+side*4, 88)), BASALT, texture)
    ember_line(p, [(x+side*2, 67), (x+side*1, 76), (x+side*4, 81)])
    for i in range(3):
        u = 68 - forward * (i * 3 + 4)
        v = 75 + i * 5
        polygon(p, [(u-2, v+3), (u+forward*2, v-5), (u+4, v+4)], "#48383c", INK, .7)


def leg(p, sign, forward, across, texture):
    hip = 64 + sign * (7 + 4 * abs(across))
    y = 88 - sign * forward * 1.6
    knee, heel = hip + forward * 6, hip - forward * 2.5
    ankle = 103 - sign * forward * 1.6
    toe = heel + forward * 7
    sole = 115 - sign * forward * 2.1
    far = sign * forward > .30
    limb = bezier((hip-5.6, y-3), (hip-9, y+2, knee-5.5, y+8, knee-4, y+10),
                  (heel-4, ankle+2, heel-4, sole-5, toe-5, sole-4),
                  (toe-7, sole-2, toe-5, sole, toe-3, sole),
                  (toe+6, sole+.3, toe+8, sole-1, toe+5, sole-4),
                  (heel+5, sole-7, heel+5, ankle, knee+5, y+10),
                  (knee+10, y+4, hip+8, y-4, hip+5, y-4))
    surface(p, limb, SHADE_SKIN if far else SKIN, texture)
    surface(p, bezier((knee-5, y+5), (knee-2, y+2), (knee+6, y+4),
                      (knee+6, y+8), (knee, y+12), (knee-4, y+9)), BASALT, texture)
    curve(p, (hip-3, y+1), [(hip-5, y+4, knee-3, y+6, knee-3, y+8)], "#e09c6e" if not far else "#ad6f4f", 1)
    line(p, [(heel+2, sole-10), (heel+1, sole-6), (toe+4, sole-4)], "#c99165", .7)
    for i in range(3):
        claw = toe - 4 + i*3.3
        polygon(p, [(claw-1.2, sole-3), (claw+1.1, sole-3.1),
                    (claw+1+forward*2, sole), (claw-.6, sole-.5)], "#9e886f", INK, .55)


def arm(p, sign, forward, across, texture):
    x = 64 + sign * (10 + 8 * abs(across))
    y = 67 - sign * forward * 2
    elbow = x + sign * 4 - forward * 3
    wrist = x + sign * 6 + forward * 4
    far = sign * forward > .30
    skin = SHADE_SKIN if far else SKIN
    limb = bezier((x-5.4, y-4), (x-9, y+1, elbow-6, y+12, elbow-4, y+15),
                  (wrist-6, y+21, wrist-4.5, y+29, wrist-3.8, y+31),
                  (wrist-5, y+36, wrist+4, y+35, wrist+5.5, y+30),
                  (wrist+7, y+25, elbow+7, y+18, elbow+4.8, y+14),
                  (elbow+6, y+9, x+7, y-7, x+3, y-6))
    surface(p, limb, skin, texture)
    surface(p, bezier((x-7, y), (x-6, y-6), (x+2, y-8), (x+6, y-4),
                      (x+7, y+1), (x+2, y+6), (x-3, y+6)), BASALT, texture)
    curve(p, (x-2, y+6), [(elbow-3, y+8, elbow-3, y+12, elbow-1, y+15)], "#e3a070" if not far else "#ae795b", 1)
    for i in range(3):
        u = wrist + (elbow-wrist)*(.9-i*.18)
        v = y+16.5+i*2.6
        line(p, [(u-2.1,v), (u+.8,v+1.5), (u+2.4,v+.85)], "#753739", .7)
        line(p, [(u-1.5,v-.45), (u+.6,v+.58)], "#d88b51", .52)
    surface(p, bezier((wrist-5, y+25), (wrist+5, y+24), (wrist+6, y+29), (wrist-4, y+31)), LEATHER, texture)
    for i in range(3):
        a = wrist - 3.2 + 3.3 * i
        b = a + sign * (.8 + i * .25)
        surface(p, bezier((a-1.4, y+31), (b-2, y+35, b-2, y+39, b+.4, y+40),
                          (b-.6, y+37, b+3, y+34, a+1.8, y+31)), HORN, None)
    if sign == 1:
        ember_line(p, [(elbow+1, y+14), (elbow-1, y+18), (wrist+.8, y+21), (wrist, y+24)], not far)
        ellipse(p, wrist-.5, y+28, 2.2, 1.7, "#e9b369")


def body(p, forward, across, texture):
    center = 64 + forward * 3
    hx = 64 + forward * 8
    half = 13 + abs(across)*3
    front = across >= -.001
    chassis = bezier((center-5, 48), (center-6, 56, center-half, 59, center-half, 68),
                     (center-half+3, 75, center-8, 82, center-9, 91),
                     (center-4, 97, center+9, 96, center+10, 88),
                     (center+8, 80, center+half+6, 70, center+half, 66),
                     (center+half, 59, center+8, 53, center+5, 49))
    surface(p, chassis, SKIN, texture)
    if front:
        for i in range(3):
            y = 66+5*i
            curve(p, (center-10+i, y), [(center-5, y-2, center+2, y+2.5, center+9-i, y-1)], "#d29464", .85)
        ember_line(p, [(center+7, 62), (center+4, 67), (center+9, 69), (center+4, 75)])
        polygon(p, [(center-14, 61), (center-10, 58), (center+12, 84), (center+9, 88)], "#493f41", INK, .75)
        line(p, [(center-12, 61), (center+2, 77), (center+9, 84)], "#ad8c6a", .85)
        for i in range(5):
            x = center-10+i*3.5
            y = 63+i*4.3
            line(p, [(x-.4, y+.6), (x+.8, y-.5)], "#c3a07a", .57)
    else:
        for i in range(4):
            y = 60+6*i
            surface(p, bezier((center-8, y+2), (center-5, y-2), (center+3, y-2),
                              (center+7, y+2), (center+3, y+5), (center-4, y+5)), BASALT, texture)
        ember_line(p, [(center+1, 64), (center-2, 69), (center+2, 74), (center-1, 83)])
    cloth = bezier((center-10, 87), (center+10, 86), (center+14, 99),
                   (center+7, 97), (center+3, 103), (center-3, 97), (center-11, 100))
    surface(p, cloth, ("#a48470", "#675053", "#342733"), texture)
    polygon(p, [(center-12, 85), (center+12, 85), (center+11, 90), (center-11, 90)], "#3b2e30", INK, .9)
    for offset in (-8, 0, 8):
        ellipse(p, center+offset-1, 86.5, 2.3, 2.2, "#d7a162", "#654632", .4)
    curve(p, (center-7, 91), [(center-8, 94, center-6, 96, center-7, 98)], "#ae8b7a", .75)

    for sign in (-1, 1):
        root = hx + sign * (8 if abs(across)<.45 else 11)
        tip = root + sign*9 - forward*7
        alto = 10 + (3 if sign*forward > .4 else 0)
        horn = bezier((root-sign*4, 34), (root+sign*5, 28, tip, 19, tip, alto),
                      (tip+sign*6, 14, tip+sign*7, 23, root+sign*4, 35))
        surface(p, horn, HORN, texture)
        for i in range(3):
            y = 21 + i*3
            xx = tip + (root-tip)*i*.27
            line(p, [(xx, y), (xx+sign*3, y+.7)], "#a38a7d", .6)
        ear = bezier((root-sign*1, 37), (root+sign*17-forward*3, 29),
                     (root+sign*15, 37), (root+sign*11, 45), (root, 48))
        surface(p, ear, SHADE_SKIN, texture)
        polygon(p, [(root+sign*4, 44), (root+sign*13, 34), (root+sign*10, 43)], "#9d594c")

    nose = hx + forward * 9
    face = bezier((hx-10, 31), (hx-3, 27, hx+9, 27, hx+13, 34),
                  (hx+16, 42, nose+8, 43, nose+6, 48),
                  (nose+7, 55, nose+1, 60, nose-2, 60),
                  (hx-9, 58, hx-15, 50, hx-15, 44),
                  (hx-15, 38, hx-13, 34, hx-10, 31))
    surface(p, face, SKIN if front else SHADE_SKIN, texture)
    if front:
        p.save()
        p.setClipPath(face)
        polygon(p, [(hx-13, 40), (hx-4, 45), (nose-1, 48), (nose-7, 51),
                    (hx-13, 49)], "#763d33")
        polygon(p, [(hx-12, 44), (hx-7, 47), (nose-4, 49), (nose-7, 50), (hx-12, 47)], "#cf8054")
        polygon(p, [(nose-5, 46), (nose+6, 44), (nose+8, 50), (nose+4, 56),
                    (nose-4, 56), (nose-8, 53)], "#4a2c30")
        curve(p, (hx-8, 34), [(hx-2, 33, hx+4, 31, hx+9, 34)], "#eb9f6d", 1.05)
        line(p, [(hx+2, 31), (hx+.4, 34.5), (hx+3.1, 36.4)], "#70352e", .86)
        for x, y in ((hx-10, 46), (hx-7.5, 48.5), (hx+9, 37), (hx-4, 33)):
            ellipse(p, x, y, 1.15, .77, "#b67958")
        p.restore()
        eyes = (hx-6, hx+5) if abs(forward)<.76 else (hx+forward*4,)
        for x in eyes:
            polygon(p, [(x-5, 38), (x+5, 40), (x+3, 44), (x-3, 43)], "#2c2025")
            polygon(p, [(x-3.8, 40), (x+3.5, 41), (x+.8, 43.2), (x-2.7, 42.2)], "#ffb748")
            line(p, [(x-1.5, 41.3), (x+1.3, 41.5)], "#fff2b4", .83)
            line(p, [(x-5, 37.3), (x+3, 39.1)], "#d89969", 1.1)
        polygon(p, [(nose-4, 44), (nose+5, 45), (nose+5, 48), (nose-1, 48)], "#3d2628", INK, .5)
        polygon(p, [(nose-8, 50), (nose+5, 50), (nose+2, 54), (nose-4, 55)], "#271921")
        for x, length in ((nose-5.2, 3.5), (nose-1.7, 1.9), (nose+1.9, 3.3)):
            polygon(p, [(x-1, 50), (x+1.3, 50), (x+.4, 50+length)], "#dbb791")
        curve(p, (nose-6, 55), [(nose-3, 58, nose+.8, 57, nose+3, 54)], "#e09a67", .8)
    else:
        for i in range(3):
            y = 36+6*i
            line(p, [(hx-7, y-2), (hx-1, y+2), (hx+6, y)], "#a16c56", 1.15)


def produce(output, retained):
    output.mkdir(parents=True, exist_ok=False)
    candidates = list((output.parent / "concept").glob("*.png"))
    if len(candidates) != 1:
        raise ValueError("Expected one DreamShaper concept")
    concept = QImage(str(candidates[0]))
    if concept.isNull() or (concept.width(), concept.height()) != (512, 512):
        raise ValueError("Missing DreamShaper surface material")
    skin = concept.copy(251, 157, 54, 46)
    ash = concept.copy(302, 124, 39, 34)
    leather = concept.copy(261, 278, 23, 50)
    master = Master(output, "ognisty-chochlik", CELL*8, CELL*6, retained)
    for row, label in enumerate(LAYERS):
        def draw(p, part=row):
            for direction, angle in enumerate(ANGLES):
                p.save()
                p.translate(direction*CELL, part*CELL)
                p.scale(CELL/128, CELL/128)
                forward, across = math.cos(angle), math.sin(angle)
                if part == 0:
                    tail(p, forward, across, leather)
                elif part == 3:
                    body(p, forward, across, ash)
                else:
                    painter = leg if part < 3 else arm
                    painter(p, -1 if part in (1, 4) else 1, forward, across, skin)
                p.restore()
        master.layer(label, draw)
    return {"atlas": master.save(), "concept": str(candidates[0]),
            "crops": {"skin": [251, 157, 54, 46], "ash": [302, 124, 39, 34], "leather": [261, 278, 23, 50]}}


def run(args):
    produce(Path(args[0]).resolve() / (args[1] if len(args)>1 else "krita"), [])
