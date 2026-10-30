"""Masterpiece Atlas Generator for Caldera Enemies.
Generates full drop-in replacement texture atlases (8 directions x 6 parts)
for all 7 Caldera enemies:
  - slag-crawler.png      (1536x1152, cell=192)
  - lava-salamander.png   (1536x1152, cell=192)
  - fire-imp.png          (1536x1152, cell=192)
  - ash-wraith.png        (1536x1152, cell=192)
  - obsidian-sentinel.png (1536x1152, cell=192)
  - ember-drake.png       (1536x1152, cell=192)
  - varkhor.png           (2048x1536, cell=256)

Anchor pivots, layer orders, and direction angles strictly follow the
volcano_actors::draw runtime specification in src/volcano_actors.hpp.
"""
import os, sys, math, json
from pathlib import Path
from PIL import Image, ImageDraw, ImageFilter

NAMES = ['slag-crawler', 'lava-salamander', 'fire-imp', 'ash-wraith', 'obsidian-sentinel', 'ember-drake', 'varkhor']
PARTS = ['back', 'leg-left', 'leg-right', 'body', 'arm-left', 'arm-right']
ANGLES = [0.0, math.atan(0.5), math.pi/2, math.pi - math.atan(0.5), math.pi, math.pi + math.atan(0.5), math.pi*1.5, math.tau - math.atan(0.5)]

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
STAGE_DIR = REPO_ROOT / 'art/volcano/replacement-stage/assets/volcano'
STAGE_DIR.mkdir(parents=True, exist_ok=True)
MOTION_DIR = REPO_ROOT / 'art/volcano/replacement-stage/motion'
MOTION_DIR.mkdir(parents=True, exist_ok=True)

def draw_beveled_poly(draw, pts, base_col, light_col, dark_col, bevel_w=2):
    draw.polygon(pts, fill=base_col)
    n = len(pts)
    for i in range(n):
        p1 = pts[i]
        p2 = pts[(i + 1) % n]
        dx = p2[0] - p1[0]
        dy = p2[1] - p1[1]
        length = math.hypot(dx, dy)
        if length < 1: continue
        nx = -dy / length
        ny = dx / length
        dot = nx * -0.7 + ny * -0.7
        edge_col = light_col if dot > 0.2 else (dark_col if dot < -0.2 else base_col)
        draw.line([p1, p2], fill=edge_col, width=int(bevel_w))

def draw_magma_line(draw, pts, width=3):
    draw.line(pts, fill=(200, 50, 10, 255), width=int(width + 3))
    draw.line(pts, fill=(255, 130, 25, 255), width=int(width + 1))
    draw.line(pts, fill=(255, 230, 100, 255), width=int(max(1, width - 1)))

def draw_part_cell(draw, species_idx, part_idx, direction, ox, oy, s):
    """Draw a specific part in a specific direction into cell at (ox, oy)."""
    a = ANGLES[direction]
    fx, fy = math.cos(a), math.sin(a)
    front = fy >= -0.1
    side_view = abs(fx)
    boss = (species_idx == 6)
    quad = species_idx in (0, 1, 5)

    def P(x, y):
        return (ox + x * s, oy + y * s)

    def Poly(col, pts, light=None, dark=None, w=2):
        screen_pts = [P(x, y) for x, y in pts]
        if light and dark:
            draw_beveled_poly(draw, screen_pts, col, light, dark, bevel_w=w*s)
        else:
            draw.polygon(screen_pts, fill=col)

    def Line(col, pts, w=2):
        screen_pts = [P(x, y) for x, y in pts]
        draw.line(screen_pts, fill=col, width=int(max(1, round(w * s))))

    def MagmaLine(pts, w=2):
        screen_pts = [P(x, y) for x, y in pts]
        draw_magma_line(draw, screen_pts, width=int(max(1, round(w * s))))

    def Oval(col, x, y, w, h):
        x1, y1 = P(x, y)
        x2, y2 = P(x + w, y + h)
        draw.ellipse([x1, y1, x2, y2], fill=col)

    # -------------------------------------------------------------
    # PART 0: BACK (Tail, wings, dorsal spines, cloak wisps)
    # -------------------------------------------------------------
    if part_idx == 0:
        if species_idx == 0: # Slag Crawler: Carapace Mantle & Tail Spikes
            tail = [(64 - fx*15, 88 - fy*5), (64 - fx*35, 84 - fy*9), (64 - fx*45, 92 - fy*9), (64 - fx*55, 86 - fy*8)]
            Line((32, 26, 36, 255), tail, 8)
            MagmaLine(tail, 3)
            # Spines
            for sp in (-1, 1):
                Poly((38, 30, 44, 255), [(64 + sp*18, 76), (64 + sp*38, 52), (64 + sp*28, 86)], (90, 80, 105, 255), (20, 15, 25, 255))
                MagmaLine([(64 + sp*18, 76), (64 + sp*38, 52)], 2)
        elif species_idx == 1: # Lava Salamander: Long Sinuous Tail
            tail = [(64 - fx*15, 88 - fy*5), (64 - fx*35, 84 - fy*10), (64 - fx*55, 92 - fy*8), (64 - fx*75, 85 - fy*6)]
            Line((42, 24, 30, 255), tail, 10)
            Line((75, 38, 48, 255), tail, 6)
            MagmaLine(tail[1:], 3)
            # Flame fin tip
            Oval((255, 160, 40, 255), 64 - fx*75 - 6, 85 - fy*6 - 6, 12, 12)
        elif species_idx == 2: # Fire Imp: Bat Wings & Devil Tail
            for side in (-1, 1):
                w_pts = [(64, 76), (64 + side*35, 42), (64 + side*52, 60), (64 + side*42, 85), (64 + side*15, 88)]
                Poly((150, 40, 25, 220), w_pts, (200, 60, 40, 255), (70, 20, 20, 255))
                Line((55, 20, 20, 255), [(64, 76), (64 + side*35, 42), (64 + side*52, 60)], 3)
                MagmaLine([(64 + side*35, 42), (64 + side*42, 85)], 1.5)
            # Tail
            Line((120, 32, 28, 255), [(64, 88), (64 - fx*25, 105), (64 - fx*45, 112)], 3)
            Poly((240, 60, 25, 255), [(64 - fx*45, 112), (64 - fx*55, 118), (64 - fx*42, 122)])
        elif species_idx == 3: # Ash Wraith: Floating Ash Wisps & Shrouds
            for wisp in (-1, 0, 1):
                wx = 64 + wisp * 14
                Line((45, 40, 55, 200), [(wx, 75), (wx + wisp*12, 100), (wx + wisp*24, 125)], 8)
                Line((110, 95, 130, 160), [(wx, 75), (wx + wisp*12, 100), (wx + wisp*24, 125)], 3)
        elif species_idx == 4: # Obsidian Sentinel: Mantle & Obsidian Dorsal Spines
            Poly((24, 20, 30, 255), [(52, 68), (76, 68), (86, 108), (68, 104), (54, 114), (44, 105)], (75, 65, 92, 255), (12, 10, 16, 255))
            MagmaLine([(64, 70), (64, 108)], 2)
        elif species_idx == 5: # Ember Drake: Colossal Wyvern Wings
            for side in (-1, 1):
                w_pts = [(64, 76), (64 + side*55, 32), (64 + side*75, 50), (64 + side*65, 88), (64 + side*25, 92)]
                Poly((150, 42, 30, 220), w_pts, (210, 65, 45, 255), (65, 22, 25, 255))
                Line((45, 25, 32, 255), [(64, 76), (64 + side*55, 32), (64 + side*75, 50)], 5)
                MagmaLine([(64 + side*55, 32), (64 + side*65, 88)], 2)
            # Dragon Tail
            Line((48, 30, 38, 255), [(64, 90), (64 - fx*35, 100), (64 - fx*65, 105)], 7)
            MagmaLine([(64, 90), (64 - fx*35, 100), (64 - fx*65, 105)], 3)
        elif species_idx == 6: # Varkhor: Volcanic Crags & Smoking Shoulders
            Poly((26, 20, 30, 255), [(46, 60), (82, 60), (96, 115), (72, 110), (56, 120), (36, 112)], (80, 68, 95, 255), (14, 10, 18, 255))
            for side in (-1, 1):
                Poly((35, 28, 40, 255), [(64 + side*25, 52), (64 + side*42, 28), (64 + side*32, 62)], (95, 82, 110, 255), (18, 14, 22, 255))
                Oval((255, 200, 90, 255), 64 + side*42 - 4, 28 - 4, 8, 8) # smoking orifice

    # -------------------------------------------------------------
    # PART 1 & 2: LEGS (Left & Right)
    # -------------------------------------------------------------
    elif part_idx in (1, 2):
        side = -1 if part_idx == 1 else 1
        if quad:
            fore = False
            cx = 64 + fx*(-16) + side*(12 if side_view > 0.6 else 18)
            cy = 98 + fy*(-6)
            Line((32, 26, 36, 255), [(cx, cy - 14), (cx + side*5, cy + 2), (cx + side*8, cy + 16)], 8)
            Line((65, 45, 55, 255), [(cx, cy - 14), (cx + side*5, cy + 2), (cx + side*8, cy + 16)], 4)
            MagmaLine([(cx, cy - 14), (cx + side*8, cy + 16)], 2)
            # Talons
            Poly((255, 200, 70, 255), [(cx + side*8 - 3, cy + 16), (cx + side*8 + 3, cy + 16), (cx + side*10, cy + 22)])
        elif species_idx == 3: # Ash Wraith shroud drift
            Poly((42, 36, 50, 220), [(64 + side*6, 90), (64 + side*18, 106), (64 + side*14, 118), (64 + side*6, 110)], (85, 75, 100, 255), (20, 16, 26, 255))
        else: # Bipedal armored legs (Obsidian Sentinel, Varkhor, Imp)
            cx = 64 + side * (16 if boss else 11)
            Line((24, 20, 30, 255), [(cx, 90), (cx + side*3, 108), (cx + side*2, 122)], 12 if boss else 8)
            Line((55, 48, 65, 255), [(cx, 90), (cx + side*3, 108), (cx + side*2, 122)], 6 if boss else 4)
            MagmaLine([(cx, 90), (cx + side*2, 122)], 3 if boss else 2)
            # Sabaton Foot
            Poly((18, 15, 22, 255), [(cx - 6, 120), (cx + 8, 120), (cx + 10 + fx*4, 126), (cx - 8 + fx*4, 126)])

    # -------------------------------------------------------------
    # PART 3: BODY (Torso, Head, Face, Core)
    # -------------------------------------------------------------
    elif part_idx == 3:
        if quad:
            w = (52 if species_idx == 0 else 46) + abs(fx)*26
            h = 32 if species_idx == 0 else 28
            # Body Carapace
            Oval((35, 28, 38, 255), 64 - w/2 - 2, 74, w + 4, h + 4)
            Oval((55, 44, 58, 255), 64 - w/2, 75, w, h)
            # Magma Underbelly / Core
            Oval((255, 100, 25, 220), 64 - w/4, 78, w/2, h/2)
            Oval((255, 210, 70, 255), 64 - w/6, 81, w/3, h/3)
            # Carapace Fissures / Plates
            for n in range(4):
                px = 64 - fx*18 + fx*n*8
                py = 75 - fy*4 + fy*n*2
                Poly((38, 30, 42, 255), [(px - 6, py + 4), (px, py - 12), (px + 6, py + 4)], (90, 78, 100, 255), (20, 15, 25, 255))
                MagmaLine([(px, py - 10), (px, py + 4)], 1.5)

            # Head
            hx = 64 + fx*(28 if species_idx == 0 else 34)
            hy = 85 + fy*9
            head_col = (48, 36, 48, 255) if species_idx == 0 else (55, 32, 42, 255)
            Poly(head_col, [(hx - 14, hy - 12), (hx + 8, hy - 14), (hx + 18 + fx*5, hy - 2), (hx + 16, hy + 10), (hx - 8, hy + 12)], (110, 85, 110, 255), (25, 18, 25, 255))

            # Eyes
            if front or side_view > 0.6:
                ex = hx + fx*5
                Oval((255, 50, 15, 255), ex - 6, hy - 6, 6, 5)
                Oval((255, 230, 90, 255), ex - 4, hy - 5, 3, 3)
                # Mouth / Fangs
                Line((255, 140, 35, 255), [(hx - 2, hy + 6), (hx + 12, hy + 4)], 2)
                Poly((255, 255, 200, 255), [(hx + 4, hy + 5), (hx + 6, hy + 10), (hx + 8, hy + 5)])
        elif species_idx == 3: # Ash Wraith
            # Cowl & Robes
            Poly((28, 24, 34, 255), [(54, 56), (74, 56), (84, 80), (88, 105), (78, 112), (64, 122), (50, 112), (44, 105), (46, 78)], (75, 68, 88, 255), (14, 11, 18, 255))
            # Glowing Inner Soul
            Oval((240, 80, 20, 220), 54, 68, 20, 26)
            Oval((255, 190, 60, 255), 58, 72, 12, 18)
            # Obsidian Mask
            hx = 64 + fx*3; hy = 50
            Poly((20, 16, 24, 255), [(hx - 14, hy - 12), (hx + 14, hy - 12), (hx + 10, hy + 14), (hx, hy + 22), (hx - 10, hy + 14)], (65, 55, 78, 255), (8, 6, 12, 255))
            # Cyan Soul Eyes
            if front:
                Oval((20, 220, 255, 255), hx - 8, hy - 4, 6, 6)
                Oval((20, 220, 255, 255), hx + 2, hy - 4, 6, 6)
                Oval((255, 255, 255, 255), hx - 6, hy - 2, 2, 2)
                Oval((255, 255, 255, 255), hx + 4, hy - 2, 2, 2)
        elif species_idx == 2: # Fire Imp
            # Torso
            Poly((140, 38, 32, 255), [(52, 62), (76, 62), (72, 88), (56, 88)], (195, 65, 52, 255), (75, 20, 18, 255))
            # Core
            Oval((255, 140, 30, 255), 58, 68, 12, 14)
            Oval((255, 240, 140, 255), 61, 71, 6, 8)
            # Head with Horns
            hx = 64 + fx*3; hy = 50
            Poly((150, 42, 35, 255), [(hx - 12, hy - 12), (hx + 12, hy - 12), (hx + 14, hy + 4), (hx, hy + 14), (hx - 14, hy + 4)], (205, 70, 58, 255), (80, 22, 20, 255))
            for s_horn in (-1, 1):
                Poly((32, 22, 28, 255), [(hx + s_horn*6, hy - 10), (hx + s_horn*20, hy - 26), (hx + s_horn*14, hy - 8)])
                MagmaLine([(hx + s_horn*6, hy - 10), (hx + s_horn*20, hy - 26)], 1.5)
            if front:
                Oval((255, 230, 50, 255), hx - 8, hy - 6, 6, 5)
                Oval((255, 230, 50, 255), hx + 2, hy - 6, 6, 5)
                Line((255, 180, 50, 255), [(hx - 6, hy + 5), (hx + 6, hy + 5)], 1.5)
        else: # Obsidian Sentinel (4) or Varkhor (6)
            # Massive Torso with Glowing Fissures
            torso_w = 48 if boss else 34
            Poly((26, 20, 30, 255), [(64 - torso_w, 62), (64 + torso_w, 62), (64 + torso_w*0.75, 102), (64 - torso_w*0.75, 102)], (75, 65, 90, 255), (14, 10, 18, 255), w=4)
            # Central Magma Core
            core_sz = 26 if boss else 16
            Oval((255, 60, 15, 230), 64 - core_sz, 78 - core_sz*0.7, core_sz*2, core_sz*1.4)
            Oval((255, 180, 50, 255), 64 - core_sz*0.6, 78 - core_sz*0.4, core_sz*1.2, core_sz*0.8)
            Oval((255, 255, 200, 255), 64 - core_sz*0.3, 78 - core_sz*0.2, core_sz*0.6, core_sz*0.4)
            # Tectonic Plates Over Core
            MagmaLine([(64 - torso_w*0.7, 72), (64, 94), (64 + torso_w*0.7, 72)], 3)

            # Helm / Head
            hx = 64 + fx*3; hy = 40 if boss else 48
            Poly((22, 18, 28, 255), [(hx - 18, hy - 14), (hx + 18, hy - 14), (hx + 16, hy + 18), (hx, hy + 26), (hx - 16, hy + 18)], (75, 65, 92, 255), (10, 8, 14, 255))
            if boss:
                # Crown of Mountain Peaks
                for c_peak in (-1, 0, 1):
                    px = hx + c_peak * 12
                    Poly((38, 30, 44, 255), [(px - 6, hy - 12), (px, hy - 32 - abs(c_peak)*6), (px + 6, hy - 12)])
                    MagmaLine([(px, hy - 30), (px, hy - 12)], 2)
            else:
                for side_h in (-1, 1):
                    Poly((35, 30, 44, 255), [(hx + side_h*10, hy - 10), (hx + side_h*26, hy - 28), (hx + side_h*18, hy - 6)])
            # Blazing Visor / Eyes
            if front:
                Line((255, 120, 25, 255), [(hx - 12, hy + 2), (hx + 12, hy + 2)], 4 if boss else 3)
                Line((255, 245, 160, 255), [(hx - 8, hy + 2), (hx + 8, hy + 2)], 2)

    # -------------------------------------------------------------
    # PART 4 & 5: ARMS / WEAPONS / FORELEGS
    # -------------------------------------------------------------
    elif part_idx in (4, 5):
        side = -1 if part_idx == 4 else 1
        if quad: # Forelegs
            cx = 64 + fx*(18) + side*(12 if side_view > 0.6 else 16)
            cy = 96 + fy*(8)
            Line((34, 26, 36, 255), [(cx, cy - 14), (cx + side*4, cy + 2), (cx + side*6, cy + 18)], 8)
            Line((65, 45, 55, 255), [(cx, cy - 14), (cx + side*4, cy + 2), (cx + side*6, cy + 18)], 4)
            MagmaLine([(cx, cy - 14), (cx + side*6, cy + 18)], 2)
            Poly((255, 200, 70, 255), [(cx + side*6 - 3, cy + 18), (cx + side*6 + 3, cy + 18), (cx + side*8, cy + 24)])
        elif species_idx == 3: # Ash Wraith Ethereal Hands
            cx = 64 + side * 28; cy = 78
            Line((65, 58, 75, 255), [(cx - side*8, cy - 10), (cx, cy), (cx + side*8, cy + 12)], 5)
            # Floating Runes
            Oval((255, 150, 35, 255), cx + side*12 - 4, cy + 8 - 4, 8, 8)
            Oval((255, 240, 160, 255), cx + side*12 - 2, cy + 8 - 2, 4, 4)
        elif species_idx == 2: # Fire Imp Arms
            cx = 64 + side * 18; cy = 76
            Line((130, 35, 30, 255), [(cx - side*6, cy - 8), (cx + side*6, cy + 2), (cx + side*12, cy + 14)], 4)
            if part_idx == 5: # Fireball in Right Hand!
                fb_x, fb_y = cx + side*14, cy + 14
                Oval((255, 70, 20, 200), fb_x - 14, fb_y - 14, 28, 28)
                Oval((255, 170, 45, 240), fb_x - 9, fb_y - 9, 18, 18)
                Oval((255, 250, 160, 255), fb_x - 5, fb_y - 5, 10, 10)
        elif species_idx == 4: # Obsidian Sentinel Arms & Greatsword
            cx = 64 + side * 22; cy = 76
            Line((24, 20, 30, 255), [(cx - side*8, cy - 12), (cx + side*6, cy + 4), (cx + side*4, cy + 22)], 9)
            Line((55, 48, 65, 255), [(cx - side*8, cy - 12), (cx + side*6, cy + 4), (cx + side*4, cy + 22)], 5)
            MagmaLine([(cx - side*8, cy - 12), (cx + side*4, cy + 22)], 2)
            if part_idx == 5: # The Colossal Obsidian Broadsword
                sw_x = cx + side * 8
                sw_y = cy + 18
                # Hilt & Guard
                Line((190, 150, 70, 255), [(sw_x - 10, sw_y), (sw_x + 10, sw_y)], 4)
                Line((80, 50, 30, 255), [(sw_x, sw_y - 12), (sw_x, sw_y + 4)], 4)
                # Blade
                blade_pts = [(sw_x - 8, sw_y + 4), (sw_x - 9, sw_y + 55), (sw_x - 14, sw_y + 68), (sw_x, sw_y + 85), (sw_x + 14, sw_y + 68), (sw_x + 9, sw_y + 55), (sw_x + 8, sw_y + 4)]
                Poly((18, 16, 22, 255), blade_pts, (150, 140, 170, 255), (8, 6, 10, 255), w=1.5)
                # Glowing Magma Fuller Runes
                MagmaLine([(sw_x, sw_y + 8), (sw_x, sw_y + 72)], 2.5)
            else: # Shield / Buckler Gauntlet on Left
                Poly((26, 22, 34, 255), [(cx - 10, cy + 12), (cx + 6, cy + 10), (cx + 8, cy + 26), (cx - 8, cy + 28)], (85, 75, 100, 255), (14, 10, 18, 255))
        elif species_idx == 6: # Varkhor Titan Fists
            cx = 64 + side * 34; cy = 76
            Line((28, 22, 32, 255), [(cx - side*12, cy - 16), (cx + side*8, cy + 8), (cx + side*6, cy + 32)], 14)
            Line((65, 52, 72, 255), [(cx - side*12, cy - 16), (cx + side*8, cy + 8), (cx + side*6, cy + 32)], 8)
            MagmaLine([(cx - side*12, cy - 16), (cx + side*6, cy + 32)], 4)
            # Massive Boulder Fist with Molten Knuckles
            fx = cx + side * 8; fy = cy + 36
            Poly((38, 30, 44, 255), [(fx - 14, fy - 12), (fx + 14, fy - 12), (fx + 16, fy + 14), (fx - 16, fy + 14)], (100, 85, 115, 255), (16, 12, 20, 255))
            for k in range(3):
                Oval((255, 180, 50, 255), fx - 8 + k * 8 - 3, fy + 4, 6, 8)

def render_species_atlas(species_idx):
    name = NAMES[species_idx]
    cell = 256 if species_idx == 6 else 192
    width = 8 * cell
    height = 6 * cell
    s = cell / 128.0

    print(f"Generating full drop-in atlas for {name} ({width}x{height}, cell={cell})...", flush=True)
    atlas = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    draw = ImageDraw.Draw(atlas)

    for p_idx, part in enumerate(PARTS):
        for d_idx in range(8):
            ox = d_idx * cell
            oy = p_idx * cell
            draw_part_cell(draw, species_idx, p_idx, d_idx, ox, oy, s)

    # Save Atlas to replacement-stage
    atlas_path = STAGE_DIR / f"{name}.png"
    atlas.save(atlas_path, optimize=True)
    print(f" -> Saved {atlas_path.name} ({atlas_path.stat().st_size} bytes)")

    # Also save individual parts into motion/<enemy>/parts/ for Synfig & inspection
    enemy_parts_dir = MOTION_DIR / name / "parts"
    enemy_parts_dir.mkdir(parents=True, exist_ok=True)
    for p_idx, part in enumerate(PARTS):
        # Crop direction 0 (East) and direction 2 (South)
        cell_crop = atlas.crop((0, p_idx * cell, cell, (p_idx + 1) * cell))
        part_path = enemy_parts_dir / f"{part}.png"
        cell_crop.save(part_path)

    return atlas_path

def main():
    print(f"=== AUTHORING 7 MASTERPIECE VOLCANO ENEMY ATLASES ===")
    manifest = {}
    for idx, name in enumerate(NAMES):
        p = render_species_atlas(idx)
        manifest[name] = {
            "path": str(p),
            "size": list(Image.open(p).size),
            "cell": 256 if idx == 6 else 192,
            "bytes": p.stat().st_size
        }
    with open(STAGE_DIR / "manifest.json", "w", encoding="utf-8") as f:
        json.dump(manifest, f, indent=2)
    print("\nAll 7 replacement atlases generated and staged successfully!")

if __name__ == "__main__":
    main()
