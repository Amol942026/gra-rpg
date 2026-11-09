"""Paint four reusable effect cutouts inside the local Krita worker."""
import math
import random

from PyQt5.QtCore import Qt
from PyQt5.QtGui import QColor, QPainterPath

from krita_abyss_chests import Master, ellipse, gradient, line, polygon
from krita_spell_art import glow


def ribbon(p, origin, curves, fill):
    path = QPainterPath()
    path.moveTo(*origin)
    for curve in curves:
        path.cubicTo(*curve)
    path.closeSubpath()
    p.setPen(Qt.NoPen)
    p.setBrush(fill)
    p.drawPath(path)


def slash(p):
    glow(p, 0, 18, 225, '#7d53ed', 126)
    for i in range(6):
        y = i * 11
        tint = QColor(113+i*17, 58+i*19, 238, 64+i*26)
        ribbon(p, (-218, 128-y), [(-102, -169-y, 106, -193-y, 225, -81-y),
                                 (118, -117-y, -44, -54-y, -218, 128-y)], tint)
    ribbon(p, (-225, 111), [(-119, -171, 119, -182, 232, -82),
                            (132, -143, -20, -97, -225, 111)],
           gradient((-180, 100), (115, -155), [(0, '#724fcb'), (.62, '#e7ceff'), (1, '#a5a0ff')]))
    for i in range(11):
        x = -149+i*31
        y = -90-30*math.sin(i*.31)
        line(p, [(x-10, y+11), (x+10, y+2), (x+21, y-4)], QColor(248, 220, 255, 176), 2)


def breath(p):
    glow(p, 35, 1, 215, '#ff681a', 153)
    palette = ['#53242a', '#9e321e', '#cf4618', '#f26b21', '#ffad45', '#fff0a4']
    for i, color in enumerate(palette):
        for n in range(3):
            lift = (n-1)*(97-i*9)
            r = 85-i*9
            ribbon(p, (-229, 5+n*4), [(-121, -20, -13, -r+lift*.2, 70, -r+lift*.63),
                                      (140, -r+lift, 150, -r*.55+lift, 235-i*9, lift-17),
                                      (159, r*.25+lift, 98, r*.45+lift*.7, 56, r*.78+lift*.38),
                                      (-13, r*.43, -144, 22, -229, 5+n*4)],
                   gradient((-208, -10), (209, lift), [(0, color), (.58, color), (1, QColor(color).lighter(131))]))
    for i in range(10):
        x = -167+i*33
        y = math.sin(i*.93)*(11+i*6)
        line(p, [(x-21, y+4), (x-5, y+8), (x+14, y)], QColor(255, 218, 119, 155), 3)


def eruption(p):
    glow(p, 0, 73, 169, '#ff5a1c', 155)
    glow(p, 0, 163, 130, '#ffba52', 190)
    ellipse(p, -174, 125, 348, 100, gradient((-110, 180), (102, 166), [(0, '#642f25'), (.55, '#fa751d'), (1, '#6d2227')]), '#ffb952', 4)
    ellipse(p, -147, 139, 294, 61, '#722727', '#d9531f', 3)
    for i in range(11):
        x = (i-5)*20
        tip = (i-5)*24+math.sin(i)*26
        top = -217+abs(i-5)*24
        ribbon(p, (x-30, 178), [(x-69, 51, tip-70, top+88, tip, top),
                                (tip-17, top+95, x+67, -14, x+31, 178),
                                (x+2, 205, x-15, 202, x-30, 178)],
               gradient((x, 179), (tip, top), [(0, '#fff3b3'), (.24, '#ffbe49'), (.65, '#f36224'), (1, QColor(175, 51, 25, 112))]))
    for i in range(12):
        a = i*math.tau/12
        x, y = math.cos(a)*146, 172+math.sin(a)*34
        polygon(p, [(x-16, y+8), (x-6, y-13), (x+14, y-11), (x+21, y+2), (x+9, y+11)], '#422d3c', '#ffc675', 2)


def cinder(p):
    glow(p, 61, 0, 152, '#ff7c28', 165)
    for n in range(7):
        lift = (n-3)*16
        ribbon(p, (-222+n*8, 21+lift), [(-121, -21+lift, 2, -69+lift*.3, 63, -72),
                                        (154, -64, 154, 48, 62, 70),
                                        (-31, 53+lift*.4, -93, -9+lift, -222+n*8, 21+lift)],
               gradient((-208, 0), (106, 0), [(0, QColor(185, 62, 28, 0)), (.54, '#e45920'), (1, '#ffdb83')]))
    rock = [(7, -42), (54, -82), (116, -61), (152, -12), (128, 51), (74, 75), (14, 45), (-3, 8)]
    polygon(p, rock, gradient((18, 58), (129, -59), [(0, '#362535'), (.48, '#59313b'), (1, '#9c5351')]), '#ffe69a', 4)
    for fissure in [[(54,-82),(61,-30),(35,-4),(14,45)],[(61,-30),(106,-23),(152,-12)],[(35,-4),(71,16),(86,71)],[(71,16),(117,25),(128,51)]]:
        line(p, fissure, '#e75227', 10)
        line(p, fissure, '#ffde82', 3.6)
    glow(p, 87, -13, 42, '#fff0b0', 182)


def produce(output, retained):
    output.mkdir(parents=True, exist_ok=False)
    master = Master(output, 'caldera-attack-paints', 2048, 512, retained)
    painters = [slash, breath, eruption, cinder]

    def paint(p):
        for index, draw in enumerate(painters):
            p.save()
            p.translate(256+512*index, 256)
            draw(p)
            p.restore()

    def sparks(p):
        for index in range(4):
            rng = random.Random(18091+index)
            p.save()
            p.translate(256+512*index, 256)
            for _ in range(58):
                x = rng.uniform(-217, 218)
                y = rng.uniform(-207, 206) if index == 2 else rng.uniform(-133, 133)
                if index == 2 and abs(x) > 60+(y+210)*.26:
                    continue
                if index == 1 and abs(y) > 18+(x+218)*.34:
                    continue
                size = rng.uniform(1.1, 3.8)
                ink = QColor(223, 183, 255, 168) if index == 0 else QColor(255, 196, 91, 182)
                line(p, [(x-size*1.4, y+size*1.1), (x+size, y-size*1.4)], ink, size)
            p.restore()

    master.layer('Painted silhouette and material', paint)
    master.layer('Glowing fractures and airborne particles', sparks)
    return {'assets': [master.save()], 'roles': ['slash', 'breath', 'eruption', 'cinder'], 'audio': False}
