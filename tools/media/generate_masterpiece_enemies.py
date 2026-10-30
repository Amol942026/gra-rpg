"""Masterpiece generator of high-detail volcanic enemies for Caldera.
Renders 512x512 multi-layer anti-aliased sprites with volumetric shading,
obsidian facets, deep magma fissures, multi-pass bloom, and downscales with Lanczos
to produce stunning 256x256 animated GIFs.
"""
import os, sys, math, json
from pathlib import Path
from PIL import Image, ImageDraw, ImageFilter
import numpy as np

ARTIFACT_DIR = Path(r"C:\Users\Amadeusz\.gemini\antigravity\brain\7eed67d0-c66c-4472-8822-5a18cb706a2b")
STAGING_DIR = ARTIFACT_DIR / "masterpiece_enemies"
STAGING_DIR.mkdir(parents=True, exist_ok=True)

SUPER_SIZE = 512
FINAL_SIZE = 256
NUM_FRAMES = 20  # 20 frames for buttery smooth motion (~16 FPS)

def hex_to_rgb(hex_str):
    hex_str = hex_str.lstrip('#')
    return tuple(int(hex_str[i:i+2], 16) for i in (0, 2, 4))

def blend_colors(c1, c2, t):
    t = max(0.0, min(1.0, t))
    return tuple(int(c1[i] + (c2[i] - c1[i]) * t) for i in range(len(c1)))

def draw_beveled_poly(draw, points, base_col, light_col, dark_col, bevel_w=4):
    """Draw a polygon with volumetric shading and chiseled bevel edges."""
    draw.polygon(points, fill=base_col)
    n = len(points)
    for i in range(n):
        p1 = points[i]
        p2 = points[(i + 1) % n]
        dx = p2[0] - p1[0]
        dy = p2[1] - p1[1]
        length = math.hypot(dx, dy)
        if length < 1:
            continue
        # Normal pointing outward
        nx = -dy / length
        ny = dx / length
        # Light coming from top-left (-0.7, -0.7)
        light_dot = nx * -0.7 + ny * -0.7
        edge_col = light_col if light_dot > 0.2 else (dark_col if light_dot < -0.2 else base_col)
        draw.line([p1, p2], fill=edge_col, width=int(bevel_w))

def draw_magma_fissure(draw, points, width=6):
    """Draw a deep magma crack with intense multi-tier core."""
    draw.line(points, fill=(180, 40, 10, 255), width=int(width + 6))
    draw.line(points, fill=(255, 110, 20, 255), width=int(width + 2))
    draw.line(points, fill=(255, 200, 60, 255), width=int(max(2, width - 2)))
    draw.line(points, fill=(255, 255, 200, 255), width=int(max(1, width - 5)))

def add_embers(draw, cx, cy, count, frame_idx, seed=1, spread_x=80, spread_y=120):
    for i in range(count):
        s = seed * 19.3 + i * 47.19
        t = (frame_idx / NUM_FRAMES + s) % 1.0
        drift_x = math.sin(s * 3.1 + t * 5.0) * spread_x
        drift_y = spread_y * 0.4 - t * spread_y * 1.6
        px = cx + drift_x
        py = cy + drift_y
        sz = 2.0 + math.sin(s * 7.0) * 1.5
        alpha = int(255 * math.sin(t * math.pi))
        col = (255, int(140 + 100 * math.sin(s * 2.0)), int(30 + 30 * math.sin(s)), alpha)
        draw.ellipse([px - sz, py - sz, px + sz, py + sz], fill=col)

def post_bloom(img, radius=12, intensity=0.7):
    """Generate subtle bloom for incandescent volcanic areas."""
    # Extract bright regions
    arr = np.array(img).astype(np.float32)
    # Luminance estimate
    lum = (arr[:, :, 0] * 0.3 + arr[:, :, 1] * 0.59 + arr[:, :, 2] * 0.11) * (arr[:, :, 3] / 255.0)
    bright_mask = np.clip((lum - 140) / 115.0, 0, 1)
    bright_arr = arr.copy()
    bright_arr[:, :, 3] *= bright_mask
    bright_img = Image.fromarray(bright_arr.astype(np.uint8))
    blurred = bright_img.filter(ImageFilter.GaussianBlur(radius))
    
    # Composite blurred glow onto image with intensity
    res = Image.new("RGBA", img.size, (0, 0, 0, 0))
    res.alpha_composite(img)
    # Blend with blurred
    glow_arr = np.array(blurred).astype(np.float32) * intensity
    res_arr = np.array(res).astype(np.float32)
    out_arr = np.clip(res_arr + glow_arr, 0, 255).astype(np.uint8)
    return Image.fromarray(out_arr)

# -------------------------------------------------------------
# 1. ŻUŻLOWY PEŁZACZ (Slag Crawler) — Masterpiece Edition
# -------------------------------------------------------------
def render_slag_crawler(frame_idx):
    img = Image.new("RGBA", (SUPER_SIZE, SUPER_SIZE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    t = frame_idx / NUM_FRAMES
    phase = t * 2 * math.pi
    cx, cy = 256, 275

    bob = math.sin(phase * 2) * 5.0
    sway = math.sin(phase) * 3.0

    # Ground Shadow with radial falloff
    draw.ellipse([cx - 130, cy + 65, cx + 130, cy + 115], fill=(12, 8, 14, 150))
    draw.ellipse([cx - 100, cy + 72, cx + 100, cy + 108], fill=(8, 5, 10, 180))

    # 6 Articulated Heavy Basalt Legs
    legs = [
        (-80, -20, -145, 30, -170, 85, -1),
        (80, -20, 145, 30, 170, 85, 1),
        (-85, 15, -155, 55, -185, 102, 1),
        (85, 15, 155, 55, 185, 102, -1),
        (-75, 45, -125, 75, -150, 110, -1),
        (75, 45, 125, 75, 150, 110, 1),
    ]
    for idx, (lx, ly, mx, my, ex, ey, ldir) in enumerate(legs):
        step = math.sin(phase + idx * math.pi / 3) * 12
        knee_x = cx + mx + step * 0.4
        knee_y = cy + my + bob - abs(step) * 0.5
        foot_x = cx + ex + step
        foot_y = cy + ey

        # Thigh plate (chiseled basalt)
        draw.line([(cx + lx, cy + ly + bob), (knee_x, knee_y)], fill=(32, 28, 38, 255), width=18)
        draw.line([(cx + lx, cy + ly + bob), (knee_x, knee_y)], fill=(58, 52, 68, 255), width=10)
        draw.line([(cx + lx, cy + ly + bob), (knee_x, knee_y)], fill=(85, 78, 100, 255), width=4)
        # Joint cap (molten hinge)
        draw.ellipse([knee_x - 9, knee_y - 9, knee_x + 9, knee_y + 9], fill=(240, 100, 25, 255))
        draw.ellipse([knee_x - 5, knee_y - 5, knee_x + 5, knee_y + 5], fill=(255, 210, 70, 255))
        # Shin plate
        draw.line([(knee_x, knee_y), (foot_x, foot_y)], fill=(24, 20, 30, 255), width=14)
        draw.line([(knee_x, knee_y), (foot_x, foot_y)], fill=(45, 38, 52, 255), width=8)
        draw.line([(knee_x, knee_y), (foot_x, foot_y)], fill=(220, 80, 20, 255), width=3)  # glowing vein
        # Molten Spiked Claw
        draw.polygon([(foot_x - 8, foot_y), (foot_x + 8, foot_y), (foot_x + ldir * 5, foot_y + 16)], fill=(255, 180, 50, 255))
        draw.polygon([(foot_x - 4, foot_y + 2), (foot_x + 4, foot_y + 2), (foot_x + ldir * 5, foot_y + 14)], fill=(255, 255, 200, 255))

    # Molten Radiant Under-Glow (illuminates the ground)
    heat = (math.sin(phase * 2) + 1) * 0.5
    draw.ellipse([cx - 85, cy - 20 + bob, cx + 85, cy + 65 + bob], fill=(255, int(90 + 50 * heat), 20, 200))
    draw.ellipse([cx - 60, cy - 5 + bob, cx + 60, cy + 50 + bob], fill=(255, 180, 50, 220))

    # Massive 3-Tier Chitinous Carapace (Segmented Volcanic Pumice)
    # Tier 1 (Rear mantle)
    rear_pts = [
        (cx - 85, cy - 35 + bob), (cx - 65, cy - 65 + bob),
        (cx + 65, cy - 65 + bob), (cx + 85, cy - 35 + bob),
        (cx + 70, cy + 30 + bob), (cx - 70, cy + 30 + bob)
    ]
    draw_beveled_poly(draw, rear_pts, (35, 30, 42, 255), (75, 68, 88, 255), (18, 15, 24, 255), bevel_w=6)
    # Tier 2 (Thoracic mantle)
    mid_pts = [
        (cx - 75, cy - 5 + bob), (cx - 55, cy - 50 + bob),
        (cx + 55, cy - 50 + bob), (cx + 75, cy - 5 + bob),
        (cx + 60, cy + 40 + bob), (cx - 60, cy + 40 + bob)
    ]
    draw_beveled_poly(draw, mid_pts, (45, 40, 55, 255), (95, 88, 112, 255), (24, 20, 32, 255), bevel_w=6)
    # Tier 3 (Front shield)
    front_pts = [
        (cx - 60, cy + 15 + bob), (cx - 45, cy - 30 + bob),
        (cx + 45, cy - 30 + bob), (cx + 60, cy + 15 + bob),
        (cx + 40, cy + 45 + bob), (cx - 40, cy + 45 + bob)
    ]
    draw_beveled_poly(draw, front_pts, (55, 48, 66, 255), (115, 105, 135, 255), (32, 26, 40, 255), bevel_w=5)

    # Intricate Glowing Magma Fissures
    draw_magma_fissure(draw, [(cx - 55, cy - 25 + bob), (cx - 15, cy + bob), (cx + 10, cy - 15 + bob), (cx + 60, cy - 30 + bob)], width=4)
    draw_magma_fissure(draw, [(cx - 35, cy + 10 + bob), (cx, cy + 30 + bob), (cx + 35, cy + 10 + bob)], width=5)
    draw_magma_fissure(draw, [(cx, cy - 50 + bob), (cx - 10, cy - 20 + bob), (cx, cy + 10 + bob)], width=4)

    # Spiny Basalt Outcroppings (Dorsal Horn Spikes)
    for side in (-1, 1):
        sp_x = cx + side * 50
        sp_y = cy - 40 + bob
        draw.polygon([(sp_x - 10, sp_y + 10), (sp_x + 10, sp_y + 10), (sp_x + side * 28, sp_y - 35)], fill=(26, 22, 32, 255))
        draw.line([(sp_x - 10, sp_y + 10), (sp_x + side * 28, sp_y - 35)], fill=(120, 110, 140, 255), width=3)
        draw.line([(sp_x + 10, sp_y + 10), (sp_x + side * 28, sp_y - 35)], fill=(255, 120, 30, 255), width=2)

    # Armored Head & Mandibles
    head_y = cy - 55 + bob
    head_pts = [
        (cx - 32, head_y + 15), (cx - 24, head_y - 28),
        (cx + 24, head_y - 28), (cx + 32, head_y + 15),
        (cx, head_y + 25)
    ]
    draw_beveled_poly(draw, head_pts, (38, 32, 46, 255), (90, 82, 105, 255), (20, 16, 26, 255), bevel_w=5)

    # 6 Burning Arachnid Eyes (Gem-like fire clusters)
    for side in (-1, 1):
        # Primary eye
        ex = cx + side * 16
        ey = head_y - 12
        draw.ellipse([ex - 5, ey - 5, ex + 5, ey + 5], fill=(255, 60, 20, 255))
        draw.ellipse([ex - 3, ey - 3, ex + 3, ey + 3], fill=(255, 230, 90, 255))
        draw.ellipse([ex - 1, ey - 1, ex + 1, ey + 1], fill=(255, 255, 255, 255))
        # Secondary eyes
        draw.ellipse([ex + side * 8 - 3, ey + 6 - 3, ex + side * 8 + 3, ey + 6 + 3], fill=(255, 140, 30, 255))
        draw.ellipse([ex + side * 8 - 1.5, ey + 6 - 1.5, ex + side * 8 + 1.5, ey + 6 + 1.5], fill=(255, 240, 150, 255))
        draw.ellipse([ex - side * 5 - 3, ey + 8 - 3, ex - side * 5 + 3, ey + 8 + 3], fill=(255, 140, 30, 255))

    # Heavy Pincer Mandibles (Flexing and dripping embers)
    pinch = math.sin(phase * 2) * 8.0
    for side in (-1, 1):
        m_bx = cx + side * 20
        m_by = head_y + 8
        m_tx = cx + side * (42 - pinch)
        m_ty = head_y - 28
        # Mandible claw
        mand_pts = [
            (m_bx, m_by),
            (cx + side * 36, head_y - 5),
            (m_tx, m_ty),
            (cx + side * 28, head_y - 18)
        ]
        draw_beveled_poly(draw, mand_pts, (48, 38, 52, 255), (120, 110, 135, 255), (25, 20, 30, 255), bevel_w=3)
        # Inner serrated teeth
        for t_i in range(3):
            tx = cx + side * (26 + t_i * 4)
            ty = head_y - 10 - t_i * 4
            draw.polygon([(tx, ty), (tx + side * 3, ty + 2), (tx - side * 4, ty + 6)], fill=(255, 220, 120, 255))
        # Tip glow
        draw.ellipse([m_tx - 4, m_ty - 4, m_tx + 4, m_ty + 4], fill=(255, 240, 140, 255))

    # Floating Embers and Heat Vents
    add_embers(draw, cx, cy - 20 + bob, 14, frame_idx, seed=12, spread_x=70, spread_y=85)
    return post_bloom(img, radius=10, intensity=0.65)

# -------------------------------------------------------------
# 2. SALAMANDRA LAWOWA (Lava Salamander) — Masterpiece Edition
# -------------------------------------------------------------
def render_lava_salamander(frame_idx):
    img = Image.new("RGBA", (SUPER_SIZE, SUPER_SIZE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    t = frame_idx / NUM_FRAMES
    phase = t * 2 * math.pi
    cx, cy = 256, 270

    bob = math.sin(phase * 2) * 4.0

    # Ground Shadow
    draw.ellipse([cx - 130, cy + 60, cx + 130, cy + 105], fill=(12, 8, 14, 140))

    # Long Sinuous Spined Tail
    tail_pts = []
    for i in range(16):
        frac = i / 15.0
        wave = math.sin(phase - frac * 3.5) * (32.0 * frac)
        tx = cx - 50 - frac * 140 + wave
        ty = cy + 25 + frac * 20 + bob
        tail_pts.append((tx, ty))

    for i in range(len(tail_pts) - 1):
        thick = int(28 * (1.0 - i / len(tail_pts))) + 4
        p1, p2 = tail_pts[i], tail_pts[i+1]
        draw.line([p1, p2], fill=(38, 22, 28, 255), width=thick + 4)
        draw.line([p1, p2], fill=(68, 32, 40, 255), width=thick)
        draw.line([p1, p2], fill=(230, 80, 25, 255), width=max(2, thick - 6))
        draw.line([p1, p2], fill=(255, 190, 60, 255), width=max(1, thick - 12))
    # Blazing Tail Fin / Flame Tuft
    end_p = tail_pts[-1]
    for flame in range(5):
        fa = phase * 3 + flame * 1.3
        fx = end_p[0] + math.cos(fa) * 12
        fy = end_p[1] + math.sin(fa) * 10
        draw.polygon([(end_p[0] - 6, end_p[1]), (fx, fy - 16), (end_p[0] + 6, end_p[1])], fill=(255, 160, 40, 220))
    draw.ellipse([end_p[0] - 8, end_p[1] - 8, end_p[0] + 8, end_p[1] + 8], fill=(255, 240, 140, 255))

    # 4 Splayed Muscular Dragon Legs with Sharp Claws
    legs = [
        (-50, 10, -90, 50, -115, 80, -1),
        (50, 10, 90, 50, 115, 80, 1),
        (-35, -30, -75, 20, -100, 50, 1),
        (40, -30, 80, 20, 105, 50, -1)
    ]
    for idx, (lx, ly, mx, my, ex, ey, ldir) in enumerate(legs):
        stride = math.sin(phase + idx * math.pi * 0.5) * 16
        knee_x = cx + mx + stride * 0.4
        knee_y = cy + my + bob - abs(stride) * 0.4
        foot_x = cx + ex + stride
        foot_y = cy + ey

        draw.line([(cx + lx, cy + ly + bob), (knee_x, knee_y)], fill=(42, 26, 32, 255), width=16)
        draw.line([(cx + lx, cy + ly + bob), (knee_x, knee_y)], fill=(75, 42, 50, 255), width=10)
        draw.line([(knee_x, knee_y), (foot_x, foot_y)], fill=(32, 20, 26, 255), width=12)
        draw.line([(knee_x, knee_y), (foot_x, foot_y)], fill=(220, 85, 30, 255), width=4)
        # 3 Curved Magma Talons
        for c_idx in (-6, 0, 6):
            draw.line([(foot_x, foot_y), (foot_x + c_idx + ldir * 8, foot_y + 10)], fill=(255, 190, 60, 255), width=4)
            draw.line([(foot_x, foot_y), (foot_x + c_idx + ldir * 8, foot_y + 10)], fill=(255, 255, 200, 255), width=2)

    # Fiery Ventral Underbelly (Incandescent Liquid Magma)
    draw.ellipse([cx - 70, cy - 15 + bob, cx + 70, cy + 45 + bob], fill=(255, 70, 15, 220))
    draw.ellipse([cx - 50, cy - 5 + bob, cx + 50, cy + 35 + bob], fill=(255, 170, 45, 240))
    draw.ellipse([cx - 30, cy + 5 + bob, cx + 30, cy + 25 + bob], fill=(255, 240, 140, 255))

    # Scaled Muscular Body (Layered Reptilian Scales)
    body_pts = [
        (cx - 65, cy + 25 + bob), (cx - 55, cy - 35 + bob),
        (cx + 50, cy - 40 + bob), (cx + 65, cy + 20 + bob),
        (cx + 25, cy + 48 + bob), (cx - 30, cy + 48 + bob)
    ]
    draw_beveled_poly(draw, body_pts, (48, 28, 36, 255), (105, 55, 70, 255), (25, 14, 20, 255), bevel_w=6)

    # Diamond-Patterned Dorsal Scales
    for r in range(4):
        for c_idx in range(6):
            sc_x = cx - 45 + c_idx * 16 + (r % 2) * 8
            sc_y = cy - 25 + r * 14 + bob
            sc_pts = [(sc_x, sc_y - 6), (sc_x + 7, sc_y), (sc_x, sc_y + 6), (sc_x - 7, sc_y)]
            draw.polygon(sc_pts, fill=(72, 38, 50, 255))
            draw.line([(sc_x, sc_y - 6), (sc_x + 7, sc_y)], fill=(130, 70, 90, 255), width=1)
            draw.line([(sc_x, sc_y), (sc_x, sc_y + 6)], fill=(255, 120, 35, 255), width=1)

    # Blazing Dorsal Frill / Flame Crest (Individual Flame Tongues)
    for f in range(11):
        fx = cx - 45 + f * 11
        fy = cy - 35 + bob
        fh = 22 + math.sin(phase * 2.5 + f * 0.8) * 9
        # Flame tongue
        draw.polygon([(fx - 6, fy), (fx + math.sin(phase + f) * 4, fy - fh), (fx + 6, fy)], fill=(255, 80, 20, 200))
        draw.polygon([(fx - 3, fy), (fx + math.sin(phase + f) * 3, fy - fh * 0.7), (fx + 3, fy)], fill=(255, 200, 60, 240))
        draw.line([(fx, fy), (fx, fy - fh * 0.5)], fill=(255, 255, 200, 255), width=2)

    # Long Draconic Head with Armored Crest
    head_x = cx + 65 + math.cos(phase) * 5
    head_y = cy - 25 + bob + math.sin(phase) * 3
    head_pts = [
        (head_x - 25, head_y + 22), (head_x - 20, head_y - 25),
        (head_x + 35, head_y - 12), (head_x + 58, head_y + 8),
        (head_x + 35, head_y + 30), (head_x - 5, head_y + 30)
    ]
    draw_beveled_poly(draw, head_pts, (55, 32, 42, 255), (120, 65, 82, 255), (28, 16, 22, 255), bevel_w=5)

    # Lower Jaw & Glowing Molten Maw
    draw.polygon([(head_x, head_y + 24), (head_x + 45, head_y + 12), (head_x + 30, head_y + 35)], fill=(38, 22, 30, 255))
    draw.line([(head_x + 10, head_y + 20), (head_x + 50, head_y + 12)], fill=(255, 140, 35, 255), width=4)
    draw.line([(head_x + 15, head_y + 20), (head_x + 45, head_y + 14)], fill=(255, 240, 150, 255), width=2)
    # Razor Sharp Fangs
    for tooth in (20, 32, 44):
        draw.polygon([(head_x + tooth - 2, head_y + 14), (head_x + tooth + 2, head_y + 14), (head_x + tooth, head_y + 22)], fill=(255, 255, 220, 255))

    # Backward Swept Basalt Crest Horns
    draw.polygon([(head_x - 12, head_y - 22), (head_x - 38, head_y - 48), (head_x + 10, head_y - 26)], fill=(32, 18, 25, 255))
    draw.line([(head_x - 12, head_y - 22), (head_x - 38, head_y - 48)], fill=(140, 80, 100, 255), width=3)
    draw.line([(head_x + 10, head_y - 26), (head_x - 38, head_y - 48)], fill=(255, 110, 35, 255), width=2)

    # Fiery Slit-Eye (Ruby & Gold)
    draw.ellipse([head_x + 12, head_y - 12, head_x + 28, head_y + 4], fill=(255, 30, 10, 255))
    draw.ellipse([head_x + 15, head_y - 9, head_x + 25, head_y + 1], fill=(255, 220, 60, 255))
    draw.line([(head_x + 20, head_y - 11), (head_x + 20, head_y + 3)], fill=(20, 8, 12, 255), width=3)

    add_embers(draw, cx + 20, cy + bob, 15, frame_idx, seed=23, spread_x=90, spread_y=75)
    return post_bloom(img, radius=12, intensity=0.7)

# -------------------------------------------------------------
# 3. OGNISTY CHOCHLIK (Fire Imp) — Masterpiece Edition
# -------------------------------------------------------------
def render_fire_imp(frame_idx):
    img = Image.new("RGBA", (SUPER_SIZE, SUPER_SIZE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    t = frame_idx / NUM_FRAMES
    phase = t * 2 * math.pi
    cx, cy = 256, 250

    hover = math.sin(phase) * 16.0
    wing_stroke = math.sin(phase * 2)

    # Pulsing Ground Shadow (fades when high)
    s_rad = 60 - hover * 1.5
    draw.ellipse([cx - s_rad, 380, cx + s_rad, 410], fill=(12, 8, 14, int(110 - hover * 3)))

    # Long Spaded Demon Tail (Figure-8 whip)
    tail_pts = []
    for i in range(14):
        frac = i / 13.0
        tx = cx - 20 - frac * 55 + math.sin(phase + frac * 4.0) * 18
        ty = cy + 30 + hover + frac * 45 + math.cos(phase + frac * 3.0) * 12
        tail_pts.append((tx, ty))
    for i in range(len(tail_pts) - 1):
        draw.line([tail_pts[i], tail_pts[i+1]], fill=(140, 35, 30, 255), width=int(8 * (1 - i/14) + 3))
    # Barbed Spade Tip
    tip = tail_pts[-1]
    draw.polygon([
        (tip[0] - 14, tip[1] - 8), (tip[0] - 24, tip[1] + 16),
        (tip[0], tip[1] + 6), (tip[0] + 6, tip[1] - 12)
    ], fill=(240, 60, 25, 255))
    draw.line([(tip[0] - 14, tip[1] - 8), (tip[0] - 24, tip[1] + 16)], fill=(255, 210, 80, 255), width=2)

    # Massive Gargoyle Bat Wings (Translucent fiery membrane folds)
    for side in (-1, 1):
        wx = cx + side * 28
        wy = cy - 15 + hover
        span = side * (85 + wing_stroke * 16)
        tip_y = wy - 75 + wing_stroke * 32
        elbow_x = wx + side * 48
        elbow_y = wy - 65 + wing_stroke * 20

        # Wing membrane polygons
        draw.polygon([
            (wx, wy), (elbow_x, elbow_y), (wx + span, tip_y),
            (wx + span * 0.85, wy - 5), (wx + span * 0.5, wy + 25), (wx + side * 12, wy + 32)
        ], fill=(160, 42, 28, 210))
        draw.polygon([
            (wx + side * 8, wy), (elbow_x - side * 6, elbow_y + 8), (wx + span * 0.7, wy + 10)
        ], fill=(255, 110, 30, 150))
        # Sturdy wing bones with thumb spur
        draw.line([(wx, wy), (elbow_x, elbow_y), (wx + span, tip_y)], fill=(55, 20, 22, 255), width=6)
        draw.line([(wx, wy), (elbow_x, elbow_y), (wx + span, tip_y)], fill=(120, 45, 40, 255), width=3)
        draw.polygon([(elbow_x - 3, elbow_y - 3), (elbow_x + side * 10, elbow_y - 12), (elbow_x + 3, elbow_y + 3)], fill=(255, 200, 80, 255))  # claw spur
        # Wing web struts
        draw.line([(elbow_x, elbow_y), (wx + span * 0.85, wy - 5)], fill=(65, 25, 28, 255), width=3)
        draw.line([(elbow_x, elbow_y), (wx + span * 0.5, wy + 25)], fill=(65, 25, 28, 255), width=3)

    # Lean Athletic Imp Torso & Legs
    torso_pts = [
        (cx - 28, cy - 25 + hover), (cx + 28, cy - 25 + hover),
        (cx + 20, cy + 32 + hover), (cx - 20, cy + 32 + hover)
    ],
    draw_beveled_poly(draw, torso_pts[0], (140, 38, 32, 255), (195, 65, 52, 255), (75, 20, 18, 255), bevel_w=4)
    # Magma Heart Core
    core_t = (math.sin(phase * 3) + 1) * 0.5
    draw.ellipse([cx - 12, cy - 10 + hover, cx + 12, cy + 14 + hover], fill=(255, int(110 + 90 * core_t), 25, 255))
    draw.ellipse([cx - 6, cy - 4 + hover, cx + 6, cy + 8 + hover], fill=(255, 250, 160, 255))

    # Digitigrade Imp Legs (Tucked in hover)
    for side in (-1, 1):
        lx = cx + side * 15
        ly = cy + 32 + hover
        draw.line([(lx, ly), (lx + side * 12, ly + 28), (lx + side * 5, ly + 50)], fill=(120, 32, 28, 255), width=8)
        draw.line([(lx, ly), (lx + side * 12, ly + 28), (lx + side * 5, ly + 50)], fill=(175, 55, 45, 255), width=4)
        for t_idx in (-4, 0, 4):
            draw.polygon([(lx + side * 5 + t_idx, ly + 50), (lx + side * 5 + t_idx + 4, ly + 60), (lx + side * 5 + t_idx - 2, ly + 52)], fill=(255, 200, 70, 255))

    # Left Hand (Gesturing with crackling sparks)
    draw.line([(cx - 24, cy - 15 + hover), (cx - 48, cy + hover), (cx - 58, cy - 25 + hover)], fill=(130, 35, 30, 255), width=6)
    draw.ellipse([cx - 62, cy - 30 + hover, cx - 50, cy - 18 + hover], fill=(255, 140, 40, 255))

    # Right Hand & The Solar Magma Orb (Spectacular swirling plasma sphere)
    orb_x = cx + 62 + math.cos(phase * 2) * 8
    orb_y = cy - 20 + hover + math.sin(phase * 2) * 8
    draw.line([(cx + 24, cy - 15 + hover), (cx + 45, cy - 5 + hover), (orb_x - 8, orb_y + 8)], fill=(130, 35, 30, 255), width=6)

    # Multi-layer Orb with Solar Flare Rings
    draw.ellipse([orb_x - 30, orb_y - 30, orb_x + 30, orb_y + 30], fill=(255, 60, 15, 160))
    draw.ellipse([orb_x - 22, orb_y - 22, orb_x + 22, orb_y + 22], fill=(255, 140, 30, 210))
    draw.ellipse([orb_x - 14, orb_y - 14, orb_x + 14, orb_y + 14], fill=(255, 220, 80, 245))
    draw.ellipse([orb_x - 7, orb_y - 7, orb_x + 7, orb_y + 7], fill=(255, 255, 220, 255))
    # Orbiting Plasma Arcs
    for a in range(4):
        ang = phase * 4 + a * (math.pi / 2)
        ax1 = orb_x + math.cos(ang) * 34
        ay1 = orb_y + math.sin(ang) * 34
        ax2 = orb_x + math.cos(ang + 0.8) * 38
        ay2 = orb_y + math.sin(ang + 0.8) * 38
        draw.line([(ax1, ay1), (ax2, ay2)], fill=(255, 230, 100, 220), width=3)

    # Imp Head (Expressive, wicked face with curved horns)
    head_y = cy - 52 + hover
    head_pts = [
        (cx - 24, head_y + 2), (cx - 20, head_y - 28),
        (cx + 20, head_y - 28), (cx + 24, head_y + 2),
        (cx, head_y + 18)
    ]
    draw_beveled_poly(draw, head_pts, (150, 42, 35, 255), (205, 70, 58, 255), (80, 22, 20, 255), bevel_w=4)

    # Curved Ram Horns with Molten Inscriptions
    for side in (-1, 1):
        h1 = (cx + side * 14, head_y - 24)
        h2 = (cx + side * 44, head_y - 56)
        h3 = (cx + side * 30, head_y - 72)
        draw.polygon([h1, (cx + side * 20, head_y - 18), h2, h3], fill=(32, 22, 28, 255))
        draw.line([h1, h2, h3], fill=(85, 60, 75, 255), width=3)
        draw.line([(cx + side * 22, head_y - 30), (cx + side * 36, head_y - 52)], fill=(255, 120, 30, 255), width=2)

    # Piercing Cat Eyes & Wicked Fang Grin
    draw.ellipse([cx - 16, head_y - 16, cx - 4, head_y - 4], fill=(255, 230, 50, 255))
    draw.ellipse([cx + 4, head_y - 16, cx + 16, head_y - 4], fill=(255, 230, 50, 255))
    draw.line([(cx - 10, head_y - 15), (cx - 10, head_y - 5)], fill=(20, 10, 10, 255), width=3)
    draw.line([(cx + 10, head_y - 15), (cx + 10, head_y - 5)], fill=(20, 10, 10, 255), width=3)
    # Grin
    draw.line([(cx - 12, head_y + 6), (cx, head_y + 12), (cx + 12, head_y + 6)], fill=(255, 180, 50, 255), width=3)
    draw.polygon([(cx - 6, head_y + 7), (cx - 4, head_y + 13), (cx - 2, head_y + 8)], fill=(255, 255, 220, 255))
    draw.polygon([(cx + 2, head_y + 8), (cx + 4, head_y + 13), (cx + 6, head_y + 7)], fill=(255, 255, 220, 255))

    add_embers(draw, cx, cy + hover, 16, frame_idx, seed=34, spread_x=65, spread_y=90)
    return post_bloom(img, radius=11, intensity=0.7)

# -------------------------------------------------------------
# 4. POPIELNY UPIÓR (Ash Wraith) — Masterpiece Edition
# -------------------------------------------------------------
def render_ash_wraith(frame_idx):
    img = Image.new("RGBA", (SUPER_SIZE, SUPER_SIZE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    t = frame_idx / NUM_FRAMES
    phase = t * 2 * math.pi
    cx, cy = 256, 240

    fly_y = math.sin(phase) * 12.0

    # Ground Shadow
    draw.ellipse([cx - 70, 395, cx + 70, 425], fill=(10, 8, 14, 110))

    # Tattered Volumetric Robes dissolving into multi-layer Ash Clouds
    for wisp in range(11):
        w_offset = (wisp - 5) * 18
        w_len = 95 + math.sin(phase * 2 + wisp) * 22
        w_curve = math.sin(phase + wisp * 0.7) * 26
        top_p = (cx + w_offset * 0.5, cy + 30 + fly_y)
        mid_p = (cx + w_offset + w_curve * 0.5, cy + 90 + fly_y)
        bot_p = (cx + w_offset * 1.3 + w_curve, cy + 30 + w_len + fly_y)

        draw.line([top_p, mid_p, bot_p], fill=(32, 28, 40, 190), width=20)
        draw.line([top_p, mid_p, bot_p], fill=(62, 55, 75, 220), width=12)
        draw.line([top_p, mid_p, bot_p], fill=(120, 108, 135, 170), width=4)
        # Smoke puffs at edges
        draw.ellipse([bot_p[0] - 12, bot_p[1] - 8, bot_p[0] + 12, bot_p[1] + 8], fill=(55, 48, 65, 130))

    # Inner Glowing Magma Soul Core
    core_pulse = (math.sin(phase * 2) + 1) * 0.5
    draw.ellipse([cx - 45, cy - 20 + fly_y, cx + 45, cy + 50 + fly_y], fill=(230, int(70 + 50 * core_pulse), 20, 180))
    draw.ellipse([cx - 30, cy - 10 + fly_y, cx + 30, cy + 40 + fly_y], fill=(255, 170, 50, 210))

    # Heavy Gothic Cowl and Mantle
    shroud_pts = [
        (cx - 72, cy - 20 + fly_y), (cx - 40, cy - 72 + fly_y),
        (cx + 40, cy - 72 + fly_y), (cx + 72, cy - 20 + fly_y),
        (cx + 52, cy + 45 + fly_y), (cx - 52, cy + 45 + fly_y)
    ]
    draw_beveled_poly(draw, shroud_pts, (28, 24, 34, 255), (75, 68, 88, 255), (14, 11, 18, 255), bevel_w=7)
    # Deep Cloth Folds
    draw.line([(cx - 40, cy - 72 + fly_y), (cx - 16, cy + 38 + fly_y)], fill=(18, 15, 24, 255), width=5)
    draw.line([(cx - 40, cy - 72 + fly_y), (cx - 16, cy + 38 + fly_y)], fill=(85, 78, 100, 255), width=2)
    draw.line([(cx + 40, cy - 72 + fly_y), (cx + 16, cy + 38 + fly_y)], fill=(18, 15, 24, 255), width=5)
    draw.line([(cx + 40, cy - 72 + fly_y), (cx + 16, cy + 38 + fly_y)], fill=(85, 78, 100, 255), width=2)

    # Skeletal Floating Hands & Orbiting Obsidian Runic Obelisks
    for side in (-1, 1):
        h_x = cx + side * (68 + math.sin(phase + side) * 8)
        h_y = cy - 5 + fly_y
        draw.line([(cx + side * 48, cy - 25 + fly_y), (h_x, h_y)], fill=(65, 58, 75, 255), width=7)
        # Bony claw fingers
        for f in range(4):
            fx = h_x + side * (10 + f * 4)
            fy = h_y + 8 + f * 5
            draw.line([(h_x, h_y), (fx, fy)], fill=(210, 195, 225, 255), width=3)
            draw.ellipse([fx - 2, fy - 2, fx + 2, fy + 2], fill=(255, 255, 255, 255))
        # Orbiting Runic Shards
        rune_ang = phase * 3 + side * 2.5
        rx = h_x + math.cos(rune_ang) * 25
        ry = h_y + math.sin(rune_ang) * 20
        draw.polygon([(rx - 6, ry), (rx, ry - 10), (rx + 6, ry), (rx, ry + 10)], fill=(20, 16, 24, 255))
        draw.line([(rx, ry - 8), (rx, ry + 8)], fill=(255, 140, 35, 255), width=2)

    # Chiseled Obsidian Death Mask
    mask_y = cy - 65 + fly_y
    mask_pts = [
        (cx - 30, mask_y - 25), (cx + 30, mask_y - 25),
        (cx + 24, mask_y + 30), (cx, mask_y + 45),
        (cx - 24, mask_y + 30)
    ]
    draw_beveled_poly(draw, mask_pts, (18, 15, 22, 255), (60, 52, 72, 255), (8, 6, 12, 255), bevel_w=5)
    # Mask facets
    draw.line([(cx, mask_y - 25), (cx, mask_y + 45)], fill=(75, 68, 88, 255), width=3)
    draw.line([(cx - 30, mask_y - 25), (cx, mask_y + 5), (cx + 30, mask_y - 25)], fill=(75, 68, 88, 255), width=2)

    # Piercing Soul Eyes (Ethereal Cyan Ghostfire with trails)
    for side in (-1, 1):
        ex = cx + side * 14
        ey = mask_y - 6
        draw.ellipse([ex - 7, ey - 7, ex + 7, ey + 7], fill=(20, 220, 255, 255))
        draw.ellipse([ex - 4, ey - 4, ex + 4, ey + 4], fill=(180, 250, 255, 255))
        draw.ellipse([ex - 1.5, ey - 1.5, ex + 1.5, ey + 1.5], fill=(255, 255, 255, 255))
        # Ghostly smoke trail drifting upwards
        draw.line([(ex, ey - 5), (ex + side * 6, ey - 22 + math.sin(phase * 3 + side) * 4)], fill=(40, 240, 255, 150), width=3)

    add_embers(draw, cx, cy + fly_y, 18, frame_idx, seed=45, spread_x=85, spread_y=110)
    return post_bloom(img, radius=12, intensity=0.7)

# -------------------------------------------------------------
# 5. OBSYDIANOWY WOJOWNIK (Obsidian Sentinel) — Masterpiece Edition
# -------------------------------------------------------------
def render_obsidian_sentinel(frame_idx):
    img = Image.new("RGBA", (SUPER_SIZE, SUPER_SIZE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    t = frame_idx / NUM_FRAMES
    phase = t * 2 * math.pi
    cx, cy = 256, 250

    bob = math.sin(phase * 2) * 4.0
    sway = math.sin(phase) * 3.0

    # Ground Shadow
    draw.ellipse([cx - 100, 370, cx + 100, 410], fill=(10, 8, 12, 170))

    # Heavy Obsidian Greaves & Sabatons
    leg_l = math.sin(phase) * 8.0
    leg_r = -leg_l

    # Left Leg
    ll_x = cx - 36 + leg_l * 0.4
    draw.polygon([(ll_x - 18, cy + 45 + bob), (ll_x + 16, cy + 45 + bob), (ll_x + 12, cy + 110), (ll_x - 22, cy + 110)], fill=(20, 18, 25, 255))
    draw.line([(ll_x - 3, cy + 45 + bob), (ll_x - 3, cy + 110)], fill=(255, 120, 30, 255), width=4)  # magma conduit
    draw.polygon([(ll_x - 26, cy + 110), (ll_x + 18, cy + 110), (ll_x + 14, cy + 130), (ll_x - 30, cy + 130)], fill=(14, 12, 18, 255))
    # Right Leg
    rl_x = cx + 36 + leg_r * 0.4
    draw.polygon([(rl_x - 16, cy + 45 + bob), (rl_x + 18, cy + 45 + bob), (rl_x + 22, cy + 110), (rl_x - 12, cy + 110)], fill=(20, 18, 25, 255))
    draw.line([(rl_x + 3, cy + 45 + bob), (rl_x + 3, cy + 110)], fill=(255, 120, 30, 255), width=4)
    draw.polygon([(rl_x - 18, cy + 110), (rl_x + 26, cy + 110), (rl_x + 30, cy + 130), (rl_x - 14, cy + 130)], fill=(14, 12, 18, 255))

    # Fluted Obsidian Cuirass (Mirror facets with blue-purple specular sheen)
    cuirass_pts = [
        (cx - 52, cy - 36 + bob), (cx + 52, cy - 36 + bob),
        (cx + 40, cy + 45 + bob), (cx - 40, cy + 45 + bob)
    ]
    draw_beveled_poly(draw, cuirass_pts, (22, 18, 28, 255), (70, 60, 85, 255), (10, 8, 14, 255), bevel_w=7)
    # Glass Sheen Facets
    draw.polygon([(cx - 52, cy - 36 + bob), (cx, cy - 44 + bob), (cx, cy + 36 + bob), (cx - 40, cy + 45 + bob)], fill=(38, 32, 48, 255))
    draw.line([(cx, cy - 44 + bob), (cx, cy + 36 + bob)], fill=(160, 145, 190, 255), width=3)  # specular glint
    # Glowing Molten Power Inscriptions
    draw.line([(cx - 32, cy - 4 + bob), (cx, cy + 16 + bob), (cx + 32, cy - 4 + bob)], fill=(255, 130, 30, 255), width=4)
    draw.line([(cx - 24, cy - 4 + bob), (cx, cy + 16 + bob), (cx + 24, cy - 4 + bob)], fill=(255, 230, 100, 255), width=2)

    # Spiked Pauldrons (Razor-edged glass shoulders)
    for side in (-1, 1):
        sh_x = cx + side * 68 + sway * 0.5
        sh_y = cy - 40 + bob
        pauldron_pts = [
            (sh_x - side * 28, sh_y + 20),
            (sh_x, sh_y - 25),
            (sh_x + side * 44, sh_y - 40),
            (sh_x + side * 32, sh_y + 16)
        ]
        draw_beveled_poly(draw, pauldron_pts, (28, 24, 35, 255), (95, 85, 115, 255), (12, 10, 16, 255), bevel_w=5)
        draw.line([(sh_x, sh_y - 25), (sh_x + side * 44, sh_y - 40)], fill=(200, 190, 225, 255), width=3)  # razor gleam
        draw.line([(sh_x - side * 28, sh_y + 20), (sh_x + side * 32, sh_y + 16)], fill=(255, 110, 30, 255), width=3)

    # Left Arm & Spiked Gauntlet
    draw.polygon([(cx - 72, cy - 20 + bob), (cx - 92, cy + 20 + bob), (cx - 80, cy + 52 + bob), (cx - 60, cy + 24 + bob)], fill=(24, 20, 30, 255))
    # Gauntlet
    draw.polygon([(cx - 88, cy + 44 + bob), (cx - 72, cy + 44 + bob), (cx - 76, cy + 68 + bob), (cx - 92, cy + 64 + bob)], fill=(45, 40, 55, 255))
    draw.ellipse([cx - 86, cy + 50 + bob, cx - 74, cy + 62 + bob], fill=(255, 140, 40, 255))  # knuckle power core

    # Right Arm & The Colossal Obsidian Greatsword
    arm_x = cx + 64
    draw.polygon([(arm_x, cy - 20 + bob), (arm_x + 24, cy + 8 + bob), (arm_x + 12, cy + 44 + bob), (arm_x - 8, cy + 24 + bob)], fill=(24, 20, 30, 255))

    # Monumental Obsidian Broadsword
    sw_x = arm_x + 24
    sw_y = cy + 28 + bob
    # Ornate Crossguard & Pommel
    draw.line([(sw_x - 20, sw_y), (sw_x + 20, sw_y)], fill=(190, 150, 70, 255), width=8)
    draw.line([(sw_x, sw_y - 24), (sw_x, sw_y + 8)], fill=(85, 55, 35, 255), width=7)
    draw.ellipse([sw_x - 7, sw_y - 30, sw_x + 7, sw_y - 18], fill=(255, 140, 40, 255))  # pommel ruby
    # Massive Blade (170px faceted slab with serrations)
    blade_pts = [
        (sw_x - 16, sw_y + 8),
        (sw_x - 18, sw_y + 110),
        (sw_x - 28, sw_y + 130),  # serration spike
        (sw_x, sw_y + 165),       # sharp tip
        (sw_x + 28, sw_y + 130),
        (sw_x + 18, sw_y + 110),
        (sw_x + 16, sw_y + 8)
    ]
    draw.polygon(blade_pts, fill=(16, 14, 20, 255))
    # Mirror reflection side
    draw.polygon([(sw_x - 16, sw_y + 8), (sw_x, sw_y + 8), (sw_x, sw_y + 165), (sw_x - 18, sw_y + 110)], fill=(36, 32, 45, 255))
    draw.line([(sw_x - 16, sw_y + 8), (sw_x - 28, sw_y + 130), (sw_x, sw_y + 165)], fill=(180, 170, 210, 255), width=2)  # razor edge
    # Fullers & Glowing Inscribed Lava Runes
    draw.line([(sw_x, sw_y + 15), (sw_x, sw_y + 140)], fill=(255, 120, 30, 255), width=6)
    draw.line([(sw_x, sw_y + 20), (sw_x, sw_y + 135)], fill=(255, 240, 120, 255), width=3)

    # Horned Barbute Greathelm
    head_y = cy - 72 + bob
    helm_pts = [
        (cx - 28, head_y - 25), (cx + 28, head_y - 25),
        (cx + 32, head_y + 25), (cx, head_y + 40),
        (cx - 32, head_y + 25)
    ]
    draw_beveled_poly(draw, helm_pts, (22, 18, 28, 255), (75, 65, 92, 255), (10, 8, 14, 255), bevel_w=6)
    # Swept Obsidian Crest Horns
    for side in (-1, 1):
        draw.polygon([(cx + side * 20, head_y - 16), (cx + side * 48, head_y - 48), (cx + side * 36, head_y - 12)], fill=(35, 30, 44, 255))
        draw.line([(cx + side * 20, head_y - 16), (cx + side * 48, head_y - 48)], fill=(190, 180, 215, 255), width=3)

    # Incandescent Visor Slit (Blasting furnace fire)
    visor_glow = (math.sin(phase * 3) + 1) * 0.5
    draw.line([(cx - 20, head_y + 5), (cx + 20, head_y + 5)], fill=(255, int(80 + 100 * visor_glow), 20, 255), width=6)
    draw.line([(cx - 14, head_y + 5), (cx + 14, head_y + 5)], fill=(255, 245, 160, 255), width=3)

    add_embers(draw, cx, cy + bob, 14, frame_idx, seed=56, spread_x=80, spread_y=110)
    return post_bloom(img, radius=11, intensity=0.65)

# -------------------------------------------------------------
# 6. ŻAROWY DRAK (Ember Drake) — Masterpiece Edition
# -------------------------------------------------------------
def render_ember_drake(frame_idx):
    img = Image.new("RGBA", (SUPER_SIZE, SUPER_SIZE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    t = frame_idx / NUM_FRAMES
    phase = t * 2 * math.pi
    cx, cy = 256, 255

    hover = math.sin(phase) * 14.0
    wing_stroke = math.sin(phase)

    # Ground Shadow
    draw.ellipse([cx - 110, 385, cx + 110, 425], fill=(12, 8, 14, int(130 - hover * 2)))

    # Long Spiked Dragon Tail
    tail_pts = []
    for i in range(16):
        frac = i / 15.0
        wave = math.sin(phase - frac * 3.0) * (36.0 * frac)
        tx = cx - 40 - frac * 150 + wave
        ty = cy + 35 + frac * 16 + hover
        tail_pts.append((tx, ty))
    for i in range(len(tail_pts) - 1):
        thick = int(24 * (1.0 - i / len(tail_pts))) + 4
        draw.line([tail_pts[i], tail_pts[i+1]], fill=(45, 30, 38, 255), width=thick + 4)
        draw.line([tail_pts[i], tail_pts[i+1]], fill=(75, 42, 52, 255), width=thick)
        draw.line([tail_pts[i], tail_pts[i+1]], fill=(230, 85, 28, 255), width=max(2, thick - 6))
    # Axe-Blade Tail Tip
    end_p = tail_pts[-1]
    draw.polygon([
        (end_p[0] - 25, end_p[1] - 8), (end_p[0] + 8, end_p[1] - 16),
        (end_p[0] + 12, end_p[1] + 12), (end_p[0] - 16, end_p[1] + 16)
    ], fill=(28, 20, 26, 255))
    draw.line([(end_p[0] - 25, end_p[1] - 8), (end_p[0] + 12, end_p[1] + 12)], fill=(255, 140, 40, 255), width=4)

    # Massive Dragon Wings (Translucent webbed membranes with glowing magma veins)
    for side in (-1, 1):
        wx = cx + side * 36
        wy = cy - 16 + hover
        span_x = side * (150 - abs(wing_stroke) * 24)
        tip_y = wy - 90 + wing_stroke * 44
        elbow_x = wx + side * 90
        elbow_y = wy - 70 + wing_stroke * 30

        # Membrane polygons
        draw.polygon([
            (wx, wy), (elbow_x, elbow_y), (wx + span_x, tip_y),
            (wx + span_x * 0.8, wy + 30), (wx + span_x * 0.5, wy + 50), (wx, wy + 36)
        ], fill=(160, 42, 30, 220))
        draw.polygon([(wx + 16, wy + 4), (elbow_x - side * 8, elbow_y + 8), (wx + span_x * 0.7, wy + 20)], fill=(255, 110, 30, 160))

        # Main wing arm bone
        draw.line([(wx, wy), (elbow_x, elbow_y), (wx + span_x, tip_y)], fill=(40, 26, 32, 255), width=8)
        draw.line([(wx, wy), (elbow_x, elbow_y), (wx + span_x, tip_y)], fill=(85, 55, 70, 255), width=4)
        # Finger struts
        draw.line([(elbow_x, elbow_y), (wx + span_x * 0.8, wy + 30)], fill=(55, 34, 44, 255), width=4)
        draw.line([(elbow_x, elbow_y), (wx + span_x * 0.5, wy + 50)], fill=(55, 34, 44, 255), width=4)
        # Magma webbing veins
        draw.line([(wx + span_x * 0.3, wy + 10), (wx + span_x * 0.6, wy + 16)], fill=(255, 190, 60, 200), width=3)

    # Hind Talons
    for side in (-1, 1):
        lx = cx + side * 32
        ly = cy + 28 + hover
        draw.line([(lx, ly), (lx + side * 16, ly + 36), (lx + side * 8, ly + 60)], fill=(48, 32, 40, 255), width=10)
        for t_idx in (-8, 0, 8):
            draw.line([(lx + side * 8, ly + 60), (lx + side * 8 + t_idx, ly + 72)], fill=(255, 190, 50, 255), width=4)

    # Dragon Torso & Molten Blast Furnace Ribcage
    torso_pts = [
        (cx - 48, cy - 28 + hover), (cx + 48, cy - 28 + hover),
        (cx + 36, cy + 36 + hover), (cx - 36, cy + 36 + hover)
    ]
    draw_beveled_poly(draw, torso_pts, (48, 30, 38, 255), (105, 60, 75, 255), (25, 15, 20, 255), bevel_w=6)
    # Glowing lava chest (breathing fire within)
    core_t = (math.sin(phase * 2) + 1) * 0.5
    draw.ellipse([cx - 24, cy - 12 + hover, cx + 24, cy + 24 + hover], fill=(255, int(90 + 70 * core_t), 20, 230))
    draw.ellipse([cx - 14, cy - 4 + hover, cx + 14, cy + 16 + hover], fill=(255, 220, 70, 255))
    # Ribcage silhouettes
    for rib in range(4):
        ry = cy - 4 + rib * 11 + hover
        draw.line([(cx - 28, ry), (cx - 6, ry + 4)], fill=(32, 20, 26, 255), width=4)
        draw.line([(cx + 28, ry), (cx + 6, ry + 4)], fill=(32, 20, 26, 255), width=4)

    # S-Curved Neck & Dragon Head
    neck_x = cx + 44 + math.cos(phase) * 6
    neck_y = cy - 44 + hover
    draw.polygon([(cx + 16, cy - 28 + hover), (neck_x, neck_y), (neck_x + 24, neck_y - 4), (cx + 40, cy - 20 + hover)], fill=(48, 32, 42, 255))
    for sp in range(5):
        sp_x = cx + 20 + sp * 8
        sp_y = cy - 32 - sp * 6 + hover
        draw.polygon([(sp_x, sp_y), (sp_x - 6, sp_y - 16), (sp_x + 8, sp_y - 4)], fill=(255, 110, 30, 255))

    # Dragon Head (Horned, fearsome maw)
    hx = neck_x + 20
    hy = neck_y - 16
    head_pts = [
        (hx - 16, hy + 20), (hx - 8, hy - 28),
        (hx + 48, hy - 12), (hx + 68, hy + 8),
        (hx + 36, hy + 28)
    ]
    draw_beveled_poly(draw, head_pts, (55, 36, 48, 255), (120, 75, 95, 255), (28, 18, 24, 255), bevel_w=5)
    # Swept Dragon Horns
    draw.polygon([(hx + 4, hy - 24), (hx - 28, hy - 64), (hx + 24, hy - 28)], fill=(30, 20, 26, 255))
    draw.line([(hx + 4, hy - 24), (hx - 28, hy - 64)], fill=(255, 140, 40, 255), width=4)

    # Fiery Maw & Dagger Fangs
    draw.polygon([(hx + 8, hy + 16), (hx + 60, hy + 8), (hx + 44, hy + 32)], fill=(38, 24, 32, 255))
    draw.line([(hx + 20, hy + 14), (hx + 56, hy + 10)], fill=(255, 160, 40, 255), width=5)
    draw.polygon([(hx + 36, hy + 10), (hx + 40, hy + 22), (hx + 44, hy + 10)], fill=(255, 255, 200, 255))
    # Blazing Golden Dragon Eye
    draw.ellipse([hx + 20, hy - 12, hx + 36, hy + 4], fill=(255, 210, 50, 255))
    draw.ellipse([hx + 24, hy - 8, hx + 32, hy], fill=(255, 60, 20, 255))
    draw.line([(hx + 28, hy - 10), (hx + 28, hy + 2)], fill=(15, 8, 12, 255), width=3)

    add_embers(draw, cx, cy + hover, 18, frame_idx, seed=67, spread_x=105, spread_y=90)
    return post_bloom(img, radius=13, intensity=0.72)

# -------------------------------------------------------------
# 7. VARKHOR, SERCE KALDERA (Volcanic Titan Boss) — Masterpiece Edition
# -------------------------------------------------------------
def render_varkhor(frame_idx):
    img = Image.new("RGBA", (SUPER_SIZE, SUPER_SIZE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    t = frame_idx / NUM_FRAMES
    phase = t * 2 * math.pi
    cx, cy = 256, 240

    bob = math.sin(phase) * 6.0
    chest_swell = (math.sin(phase) + 1) * 0.5

    # Titanic Ground Shadow
    draw.ellipse([cx - 135, 380, cx + 135, 430], fill=(8, 4, 10, 190))

    # Massive Basalt Column Pillar Legs
    for side in (-1, 1):
        lx = cx + side * 48
        draw.polygon([(lx - 26, cy + 55 + bob), (lx + 26, cy + 55 + bob), (lx + side * 10 + 32, cy + 140), (lx + side * 10 - 32, cy + 140)], fill=(24, 20, 28, 255))
        draw_magma_fissure(draw, [(lx, cy + 55 + bob), (lx + side * 6, cy + 140)], width=5)
        # Giant Tectonic Foot
        draw.polygon([(lx + side * 10 - 40, cy + 136), (lx + side * 10 + 40, cy + 136), (lx + side * 10 + 44, cy + 160), (lx + side * 10 - 44, cy + 160)], fill=(16, 12, 20, 255))

    # Active Volcanic Shoulder Vents emitting Smoke & Glowing Cinders
    for side in (-1, 1):
        vx = cx + side * 88
        vy = cy - 65 + bob
        vent_pts = [(vx - 16, vy + 28), (vx + 16, vy + 28), (vx + 10, vy - 20), (vx - 10, vy - 20)]
        draw_beveled_poly(draw, vent_pts, (35, 28, 38, 255), (85, 75, 95, 255), (18, 14, 22, 255), bevel_w=5)
        # Boiling Vent Core
        draw.ellipse([vx - 10, vy - 24, vx + 10, vy - 16], fill=(255, 230, 110, 255))
        # Volumetric Smoke Puff Column
        for s_idx in range(4):
            st = (t + side * 0.2 + s_idx * 0.25) % 1.0
            smk_y = vy - 25 - st * 65
            smk_r = 12 + st * 22
            smk_alpha = int(190 * (1.0 - st))
            draw.ellipse([vx - smk_r, smk_y - smk_r * 0.6, vx + smk_r, smk_y + smk_r * 0.6], fill=(55, 45, 62, smk_alpha))

    # Colossal Torso (Tectonic Mountain Carapace)
    torso_w = 84 + chest_swell * 8
    torso_pts = [
        (cx - torso_w, cy - 50 + bob), (cx + torso_w, cy - 50 + bob),
        (cx + 64, cy + 60 + bob), (cx - 64, cy + 60 + bob)
    ]
    draw_beveled_poly(draw, torso_pts, (28, 22, 32, 255), (80, 68, 92, 255), (14, 10, 18, 255), bevel_w=8)

    # "THE HEART OF THE CALDERA" — Pulsing Central Magma Crucible
    core_rad = 36 + chest_swell * 12
    draw.ellipse([cx - core_rad, cy - 5 - core_rad * 0.8 + bob, cx + core_rad, cy - 5 + core_rad * 0.8 + bob], fill=(255, 50, 10, 220))
    draw.ellipse([cx - core_rad * 0.7, cy - 5 - core_rad * 0.5 + bob, cx + core_rad * 0.7, cy - 5 + core_rad * 0.5 + bob], fill=(255, 150, 35, 240))
    draw.ellipse([cx - core_rad * 0.35, cy - 5 - core_rad * 0.25 + bob, cx + core_rad * 0.35, cy - 5 + core_rad * 0.25 + bob], fill=(255, 255, 200, 255))

    # Tectonic Crust Plates Overlapping the Core
    draw_beveled_poly(draw, [(cx - 72, cy - 40 + bob), (cx - 28, cy - 24 + bob), (cx - 44, cy + 30 + bob), (cx - 68, cy + 10 + bob)], (45, 36, 48, 255), (95, 82, 105, 255), (20, 15, 24, 255), bevel_w=5)
    draw_beveled_poly(draw, [(cx + 72, cy - 40 + bob), (cx + 28, cy - 24 + bob), (cx + 44, cy + 30 + bob), (cx + 68, cy + 10 + bob)], (45, 36, 48, 255), (95, 82, 105, 255), (20, 15, 24, 255), bevel_w=5)
    draw_beveled_poly(draw, [(cx - 32, cy + 28 + bob), (cx + 32, cy + 28 + bob), (cx, cy + 56 + bob)], (38, 30, 42, 255), (85, 72, 92, 255), (18, 14, 22, 255), bevel_w=4)

    # Spiked Volcanic Crag Pauldrons
    for side in (-1, 1):
        px = cx + side * 100
        py = cy - 30 + bob
        paul_pts = [
            (px - side * 32, py + 36),
            (px - side * 8, py - 36),
            (px + side * 48, py - 48),
            (px + side * 44, py + 24)
        ]
        draw_beveled_poly(draw, paul_pts, (36, 28, 40, 255), (100, 85, 110, 255), (16, 12, 20, 255), bevel_w=6)
        draw.line([(px, py - 12), (px + side * 36, py - 8)], fill=(255, 130, 35, 255), width=4)

    # Titanic Boulder Arms & Knuckle Fists
    for side in (-1, 1):
        ax = cx + side * 104
        ay = cy + 20 + bob
        draw.polygon([(ax - 20, ay - 28), (ax + 20, ay - 28), (ax + side * 24, ay + 40), (ax - side * 8, ay + 40)], fill=(30, 24, 34, 255))
        # Giant Fist with Magma Knuckles
        fx = ax + side * 12
        fy = ay + 64
        draw_beveled_poly(draw, [(fx - 28, fy - 24), (fx + 28, fy - 24), (fx + 32, fy + 24), (fx - 32, fy + 24)], (42, 32, 46, 255), (110, 92, 120, 255), (20, 15, 24, 255), bevel_w=5)
        for k in range(3):
            kx = fx - 16 + k * 16
            draw.ellipse([kx - 6, fy + 8, kx + 6, fy + 20], fill=(255, 170, 50, 255))
            draw.ellipse([kx - 3, fy + 11, kx + 3, fy + 17], fill=(255, 255, 200, 255))

    # Mountain Crown Head (Triple Basalt Spires)
    head_y = cy - 84 + bob
    head_pts = [
        (cx - 36, head_y - 20), (cx - 20, head_y - 48),
        (cx + 20, head_y - 48), (cx + 36, head_y - 20),
        (cx + 28, head_y + 32), (cx, head_y + 48),
        (cx - 28, head_y + 32)
    ]
    draw_beveled_poly(draw, head_pts, (26, 20, 30, 255), (75, 62, 85, 255), (12, 10, 16, 255), bevel_w=6)

    # Triple Basalt Crown Horns
    draw.polygon([(cx - 28, head_y - 36), (cx - 52, head_y - 84), (cx - 12, head_y - 44)], fill=(38, 30, 44, 255))
    draw.polygon([(cx + 12, head_y - 44), (cx + 52, head_y - 84), (cx + 28, head_y - 36)], fill=(38, 30, 44, 255))
    draw.polygon([(cx - 14, head_y - 44), (cx, head_y - 92), (cx + 14, head_y - 44)], fill=(48, 38, 54, 255))
    draw.line([(cx, head_y - 92), (cx, head_y - 44)], fill=(255, 160, 40, 255), width=4)

    # Face Carvings: Burning Eyes and Molten Maw
    for side in (-1, 1):
        ex = cx + side * 18
        ey = head_y - 8
        draw.ellipse([ex - 6, ey - 8, ex + 6, ey + 8], fill=(255, 230, 70, 255))
        draw.ellipse([ex - 3, ey - 4, ex + 3, ey + 4], fill=(255, 255, 220, 255))
    # Jagged Maw of Fire
    draw.polygon([
        (cx - 20, head_y + 16), (cx - 10, head_y + 28),
        (cx, head_y + 20), (cx + 10, head_y + 28), (cx + 20, head_y + 16),
        (cx + 12, head_y + 36), (cx - 12, head_y + 36)
    ], fill=(255, 120, 20, 255))
    draw.polygon([(cx - 4, head_y + 22), (cx + 4, head_y + 22), (cx, head_y + 32)], fill=(255, 240, 160, 255))

    add_embers(draw, cx, cy - 20 + bob, 26, frame_idx, seed=78, spread_x=130, spread_y=140)
    return post_bloom(img, radius=14, intensity=0.75)

# -------------------------------------------------------------
# MAIN BATCH
# -------------------------------------------------------------
ENEMIES = [
    ("slag_crawler", "Żużlowy pełzacz", "Pancernik kaldery", render_slag_crawler),
    ("lava_salamander", "Salamandra lawowa", "Ognisty gad", render_lava_salamander),
    ("fire_imp", "Ognisty chochlik", "Demon żaru", render_fire_imp),
    ("ash_wraith", "Popielny upiór", "Duch pogorzeliska", render_ash_wraith),
    ("obsidian_sentinel", "Obsydianowy wojownik", "Strażnik kuźni", render_obsidian_sentinel),
    ("ember_drake", "Żarowy drak", "Skrzydlata bestia", render_ember_drake),
    ("varkhor", "Varkhor, Serce Kaldery", "Władca wulkanu — Boss", render_varkhor),
]

def main():
    print(f"Generating MASTERPIECE EDITION animations ({SUPER_SIZE}x{SUPER_SIZE} -> {FINAL_SIZE}x{FINAL_SIZE} Lanczos)...")
    manifest = []

    for key, name_pl, role, render_func in ENEMIES:
        print(f"Rendering {name_pl} ({key})...", flush=True)
        final_frames = []
        for f in range(NUM_FRAMES):
            super_img = render_func(f)
            # High quality Lanczos downscale for razor-sharp antialiasing
            final_img = super_img.resize((FINAL_SIZE, FINAL_SIZE), Image.Resampling.LANCZOS)
            final_frames.append(final_img)

        # Save animated GIF directly in ARTIFACT_DIR
        gif_path = ARTIFACT_DIR / f"{key}_animated.gif"
        final_frames[0].save(
            gif_path,
            save_all=True,
            append_images=final_frames[1:],
            duration=50,  # 20 FPS ultra smooth loop
            loop=0,
            disposal=2
        )

        # Save static high-res preview frame
        preview_path = ARTIFACT_DIR / f"{key}_preview.png"
        final_frames[0].save(preview_path)

        manifest.append({
            "key": key,
            "name": name_pl,
            "role": role,
            "gif": str(gif_path.name),
            "preview": str(preview_path.name),
            "frames": NUM_FRAMES,
            "frame_size": FINAL_SIZE,
        })
        print(f" -> Saved {gif_path.name} ({os.path.getsize(gif_path)} bytes)")

    # Save manifest
    manifest_path = ARTIFACT_DIR / "masterpiece_manifest.json"
    with open(manifest_path, "w", encoding="utf-8") as f:
        json.dump(manifest, f, indent=2, ensure_ascii=False)
    print("Masterpiece animations complete!")

if __name__ == "__main__":
    main()
