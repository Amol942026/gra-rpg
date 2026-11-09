"""Seamless volcanic trail material painted inside the local Krita worker."""
import math
import random
from PyQt5.QtCore import Qt, QPointF
from PyQt5.QtGui import QColor, QPainterPath, QPen
from krita_abyss_chests import Master, polygon, line, ellipse, gradient


def wrapped(painter, draw):
    for y in (-512, 0, 512):
        for x in (-512, 0, 512):
            painter.save()
            painter.translate(x, y)
            draw(painter)
            painter.restore()


def produce(output, retained):
    output.mkdir(parents=True, exist_ok=False)
    master = Master(output, "caldera-worn-basalt-trail", 1024, 1024, retained, 2)

    def bed(p):
        p.fillRect(0, 0, 512, 512, QColor("#746c67"))
        rng = random.Random(81032)
        # Tiny translucent native brush marks form compacted ash and fine grit.
        for _ in range(16000):
            x, y = rng.uniform(0, 512), rng.uniform(0, 512)
            c = QColor(rng.choice(("#b7a18c", "#4a4548", "#8f8175", "#62585a")))
            c.setAlpha(rng.randint(18, 75))
            size = rng.uniform(.35, 1.7)
            wrapped(p, lambda q: ellipse(q, x, y, size, size * .55, c))
    master.layer("Compacted volcanic ash and fine grains", bed)

    stones = []
    rng = random.Random(37109)
    for _ in range(125):
        x, y, radius = rng.uniform(0, 512), rng.uniform(0, 512), rng.uniform(2.3, 11)
        n = rng.randint(5, 8)
        points = [(x + math.cos(i * math.tau / n) * radius * rng.uniform(.65, 1.15),
                   y + math.sin(i * math.tau / n) * radius * rng.uniform(.45, .8)) for i in range(n)]
        stones.append((x, y, radius, points, rng.choice(("#57565a", "#656169", "#807573", "#484d54", "#8c7d70"))))

    def plates(p):
        for x, y, r, points, ink in stones:
            def draw(q):
                polygon(q, [(a + 1.1, b + 1.4) for a, b in points], QColor(32, 34, 39, 80))
                path = QPainterPath()
                previous = points[-1]
                path.moveTo((previous[0] + points[0][0]) / 2, (previous[1] + points[0][1]) / 2)
                for i, point in enumerate(points):
                    following = points[(i + 1) % len(points)]
                    path.quadTo(QPointF(*point), QPointF((point[0] + following[0]) / 2, (point[1] + following[1]) / 2))
                path.closeSubpath()
                q.setPen(Qt.NoPen)
                q.setBrush(gradient((x-r, y-r), (x+r, y+r), [(0, "#a99b89"), (.32, ink), (1, "#46464f")]))
                q.drawPath(path)
            wrapped(p, draw)
    master.layer("Rounded weathered basalt chips and slabs", plates)

    def detail(p):
        rng = random.Random(831)
        for x, y, r, points, ink in stones:
            if r < 6:
                continue
            def draw(q):
                line(q, [(x-r*.4, y-r*.12), (x-r*.05, y), (x+r*.25, y+r*.3)], QColor(31, 34, 43, 120), .65)
                line(q, [(x-r*.4, y-r*.18), (x-r*.05, y-r*.07)], QColor(206, 187, 151, 80), .55)
                if rng.random() < .3:
                    ellipse(q, x+r*.1, y-r*.13, .9, .65, QColor(205, 154, 95, 120))
            wrapped(p, draw)
    master.layer("Hairline fractures, pores and weathered edges", detail)

    def pebbles(p):
        rng = random.Random(4822)
        for _ in range(850):
            x, y, r = rng.uniform(0, 512), rng.uniform(0, 512), rng.uniform(.6, 2.2)
            c = rng.choice(("#494a51", "#a99b86", "#857a70", "#5b5556"))
            wrapped(p, lambda q: ellipse(q, x, y, r, r*.65, c))
    master.layer("Scattered dark gravel and muted warm mineral flecks", pebbles)
    record = master.save(runtime_size=(512, 512))
    return {"assets": [record], "tool": "Krita native Python / Qt painting", "external_inputs": [], "seamless_period": 512, "audio": False}
