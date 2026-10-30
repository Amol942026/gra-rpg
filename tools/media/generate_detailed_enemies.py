"""Generator of high-detail redesigned volcanic enemy animations and previews.
Draws enhanced, layered 2.5D isometric sprites for the 7 Caldera enemies.
Outputs animated GIFs and sprite sheets to the artifact directory.
Does NOT modify game assets.
"""
import os, sys, math, json
from pathlib import Path
from PIL import Image, ImageDraw, ImageFilter

ARTIFACT_DIR = Path(r"C:\Users\Amadeusz\.gemini\antigravity\brain\7eed67d0-c66c-4472-8822-5a18cb706a2b")
STAGING_DIR = ARTIFACT_DIR / "redesigned_enemies"
STAGING_DIR.mkdir(parents=True, exist_ok=True)

FRAME_SIZE = 256
NUM_FRAMES = 16  # 16-frame smooth loops

def lerp(a, b, t):
    return a + (b - a) * t

def hex_to_rgb(hex_str):
    hex_str = hex_str.lstrip('#')
    return tuple(int(hex_str[i:i+2], 16) for i in (0, 2, 4))

def blend_color(c1, c2, t):
    return tuple(int(c1[i] + (c2[i] - c1[i]) * t) for i in range(len(c1)))

def draw_glow(base_img, draw_func, color, radius=12, intensity=1.5):
    """Render a soft additive glow pass."""
    glow_layer = Image.new("RGBA", base_img.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(glow_layer)
    draw_func(d)
    blurred = glow_layer.filter(ImageFilter.GaussianBlur(radius))
    # Composite blurred glow over base
    base_img.alpha_composite(blurred)

def add_ember_particles(draw, cx, cy, count, frame_idx, seed_offset, spread_x=40, spread_y=50):
    for i in range(count):
        s = seed_offset + i * 37.17
        speed = 0.8 + (math.sin(s * 1.3) + 1) * 0.5
        t = (frame_idx / NUM_FRAMES + s) % 1.0
        px = cx + math.sin(s * 2.5 + t * 4.0) * spread_x
        py = cy + spread_y * 0.5 - t * spread_y * 1.5
        size = 1.5 + math.sin(s * 5.0) * 1.0
        alpha = int(255 * math.sin(t * math.pi))
        color = (255, int(150 + 100 * math.sin(s)), 30, alpha)
        draw.ellipse([px - size, py - size, px + size, py + size], fill=color)

# -------------------------------------------------------------
# 1. ŻUŻLOWY PEŁZACZ (Slag Crawler)
# -------------------------------------------------------------
def render_slag_crawler_frame(frame_idx):
    img = Image.new("RGBA", (FRAME_SIZE, FRAME_SIZE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    t = frame_idx / NUM_FRAMES
    phase = t * 2 * math.pi

    cx, cy = 128, 140
    # Breathing and crawling bounce
    bob_y = math.sin(phase * 2) * 2.5
    leg_swing = math.sin(phase)

    # Shadow
    draw.ellipse([cx - 55, cy + 30, cx + 55, cy + 50], fill=(15, 10, 15, 130))

    # Legs (6 articulated basalt limbs with claw tips)
    leg_coords = [
        (-40, -10, -65, 15, -75, 38, -1),
        (40, -10, 65, 15, 75, 38, 1),
        (-42, 5, -70, 25, -82, 45, 1),
        (42, 5, 70, 25, 82, 45, -1),
        (-35, 20, -55, 35, -65, 48, -1),
        (35, 20, 55, 35, 65, 48, 1),
    ]
    for idx, (lx, ly, mx, my, ex, ey, ldir) in enumerate(leg_coords):
        swing = math.sin(phase + idx * math.pi / 3) * 6
        knee_x = cx + mx + swing * 0.5
        knee_y = cy + my + bob_y - abs(swing) * 0.5
        foot_x = cx + ex + swing
        foot_y = cy + ey

        # Segment 1 (coxa/femur)
        draw.line([(cx + lx, cy + ly + bob_y), (knee_x, knee_y)], fill=(38, 34, 42, 255), width=8)
        draw.line([(cx + lx, cy + ly + bob_y), (knee_x, knee_y)], fill=(70, 62, 75, 255), width=4)
        # Segment 2 (tibia/claw)
        draw.line([(knee_x, knee_y), (foot_x, foot_y)], fill=(30, 26, 35, 255), width=6)
        draw.line([(knee_x, knee_y), (foot_x, foot_y)], fill=(220, 90, 30, 255), width=2)
        # Claw tip
        draw.polygon([(foot_x - 3, foot_y), (foot_x + 3, foot_y), (foot_x, foot_y + 6)], fill=(255, 180, 50, 255))

    # Molten Under-Glow
    heat_pulse = (math.sin(phase * 2) + 1) * 0.5
    glow_color = (255, int(100 + 80 * heat_pulse), 20, 180)
    draw.ellipse([cx - 36, cy - 8 + bob_y, cx + 36, cy + 28 + bob_y], fill=glow_color)

    # Abdomen / Carapace Plates (Layered Pumice Rock)
    # Tier 1 (Rear mantle)
    draw.ellipse([cx - 38, cy - 18 + bob_y, cx + 38, cy + 22 + bob_y], fill=(32, 28, 36, 255))
    draw.ellipse([cx - 34, cy - 15 + bob_y, cx + 34, cy + 18 + bob_y], fill=(52, 46, 58, 255))
    # Magma fissures across carapace
    draw.line([(cx - 28, cy - 5 + bob_y), (cx, cy + 8 + bob_y), (cx + 28, cy - 5 + bob_y)], fill=(255, 120, 30, 255), width=3)
    draw.line([(cx - 15, cy + bob_y), (cx, cy + 18 + bob_y), (cx + 15, cy + bob_y)], fill=(255, 200, 70, 255), width=2)

    # Tier 2 (Thorax carapace)
    draw.polygon([
        (cx - 36, cy - 2 + bob_y), (cx - 25, cy - 28 + bob_y),
        (cx + 25, cy - 28 + bob_y), (cx + 36, cy - 2 + bob_y),
        (cx + 28, cy + 12 + bob_y), (cx - 28, cy + 12 + bob_y)
    ], fill=(42, 38, 48, 255))
    # Rock facet highlights
    draw.line([(cx - 25, cy - 28 + bob_y), (cx, cy - 8 + bob_y), (cx + 25, cy - 28 + bob_y)], fill=(95, 85, 105, 255), width=2)
    draw.line([(cx, cy - 8 + bob_y), (cx, cy + 12 + bob_y)], fill=(255, 140, 40, 255), width=2)

    # Spikes on carapace
    for side in (-1, 1):
        spx = cx + side * 24
        spy = cy - 20 + bob_y
        draw.polygon([(spx - 4, spy + 5), (spx + 4, spy + 5), (spx + side * 12, spy - 16)], fill=(28, 25, 32, 255))
        draw.line([(spx, spy + 5), (spx + side * 12, spy - 16)], fill=(255, 100, 30, 255), width=1)

    # Head & Heavy Jaws / Mandibles
    head_y = cy - 26 + bob_y
    draw.polygon([
        (cx - 16, head_y), (cx - 12, head_y - 18),
        (cx + 12, head_y - 18), (cx + 16, head_y),
        (cx, head_y + 8)
    ], fill=(32, 28, 36, 255))
    # Glowing multi-eyes (6 arachnid fire eyes)
    for side in (-1, 1):
        draw.ellipse([cx + side * 8 - 2, head_y - 10 - 2, cx + side * 8 + 2, head_y - 10 + 2], fill=(255, 220, 100, 255))
        draw.ellipse([cx + side * 5 - 1.5, head_y - 6 - 1.5, cx + side * 5 + 1.5, head_y - 6 + 1.5], fill=(255, 130, 30, 255))
        draw.ellipse([cx + side * 11 - 1.5, head_y - 7 - 1.5, cx + side * 11 + 1.5, head_y - 7 + 1.5], fill=(255, 130, 30, 255))

    # Molten Mandibles (jaw pinchers that flex)
    mand_pinch = math.sin(phase * 2) * 4
    for side in (-1, 1):
        m_base_x = cx + side * 10
        m_base_y = head_y - 2
        m_tip_x = cx + side * (20 - mand_pinch)
        m_tip_y = head_y - 18
        draw.polygon([
            (m_base_x, m_base_y),
            (cx + side * 18, head_y - 6),
            (m_tip_x, m_tip_y),
            (cx + side * 14, head_y - 12)
        ], fill=(45, 35, 45, 255))
        draw.line([(cx + side * 14, head_y - 6), (m_tip_x, m_tip_y)], fill=(255, 160, 40, 255), width=2)
        draw.ellipse([m_tip_x - 2, m_tip_y - 2, m_tip_x + 2, m_tip_y + 2], fill=(255, 240, 150, 255))

    # Embers
    add_ember_particles(draw, cx, cy - 10 + bob_y, 6, frame_idx, 101, spread_x=35, spread_y=40)
    return img

# -------------------------------------------------------------
# 2. SALAMANDRA LAWOWA (Lava Salamander)
# -------------------------------------------------------------
def render_lava_salamander_frame(frame_idx):
    img = Image.new("RGBA", (FRAME_SIZE, FRAME_SIZE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    t = frame_idx / NUM_FRAMES
    phase = t * 2 * math.pi

    cx, cy = 128, 135
    spine_wave = math.sin(phase) * 6.0
    bob_y = math.sin(phase * 2) * 2.0

    # Shadow
    draw.ellipse([cx - 60, cy + 28, cx + 60, cy + 46], fill=(15, 10, 15, 120))

    # Tail (sinuous curves)
    tail_pts = []
    for i in range(12):
        frac = i / 11.0
        w = math.sin(phase - frac * 3.0) * (14.0 * frac)
        tx = cx - 25 - frac * 65 + w
        ty = cy + 12 + frac * 10 + bob_y
        tail_pts.append((tx, ty))

    for i in range(len(tail_pts) - 1):
        thick = int(14 * (1.0 - i / len(tail_pts))) + 2
        p1, p2 = tail_pts[i], tail_pts[i+1]
        draw.line([p1, p2], fill=(45, 30, 35, 255), width=thick)
        draw.line([p1, p2], fill=(220, 80, 25, 255), width=max(1, thick - 4))
        draw.line([p1, p2], fill=(255, 180, 60, 255), width=max(1, thick - 8))
    # Fiery tail tuft
    end_pt = tail_pts[-1]
    draw.ellipse([end_pt[0] - 5, end_pt[1] - 5, end_pt[0] + 5, end_pt[1] + 5], fill=(255, 200, 80, 255))

    # 4 Legs with sharp magma claws
    legs = [
        (-25, 5, -45, 25, -55, 38, -1),
        (25, 5, 45, 25, 55, 38, 1),
        (-15, -15, -35, 10, -48, 25, 1),
        (20, -15, 40, 10, 52, 25, -1)
    ]
    for idx, (lx, ly, mx, my, ex, ey, ldir) in enumerate(legs):
        stride = math.sin(phase + idx * math.pi * 0.5) * 8
        knee_x = cx + mx + stride * 0.4
        knee_y = cy + my + bob_y - abs(stride) * 0.3
        foot_x = cx + ex + stride
        foot_y = cy + ey
        draw.line([(cx + lx, cy + ly + bob_y), (knee_x, knee_y)], fill=(45, 32, 40, 255), width=7)
        draw.line([(knee_x, knee_y), (foot_x, foot_y)], fill=(65, 40, 45, 255), width=5)
        # Claws
        for c_idx in (-3, 0, 3):
            draw.line([(foot_x, foot_y), (foot_x + c_idx + ldir * 4, foot_y + 4)], fill=(255, 170, 40, 255), width=2)

    # Fiery Ventral Glow (Belly)
    draw.ellipse([cx - 32, cy - 6 + bob_y, cx + 32, cy + 22 + bob_y], fill=(255, 110, 25, 200))
    draw.ellipse([cx - 24, cy - 2 + bob_y, cx + 24, cy + 18 + bob_y], fill=(255, 200, 60, 220))

    # Torso (Muscular Basalt Body)
    draw.polygon([
        (cx - 32, cy + 12 + bob_y), (cx - 28, cy - 16 + bob_y),
        (cx + 26, cy - 18 + bob_y), (cx + 32, cy + 10 + bob_y),
        (cx + 10, cy + 24 + bob_y), (cx - 15, cy + 24 + bob_y)
    ], fill=(42, 32, 40, 255))
    # Scales / Scutes along back
    for s_idx in range(5):
        sx = cx - 18 + s_idx * 9
        sy = cy - 14 + bob_y + math.sin(s_idx + phase) * 2
        draw.polygon([(sx, sy), (sx + 7, sy - 2), (sx + 4, sy + 6), (sx - 3, sy + 7)], fill=(68, 52, 64, 255))
        draw.line([(sx, sy), (sx + 4, sy + 6)], fill=(255, 130, 40, 255), width=2)

    # Dorsal Flame Fin / Frill
    frill_pts = []
    for f in range(7):
        fx = cx - 22 + f * 7
        fy = cy - 16 + bob_y
        fh = 10 + math.sin(phase * 2 + f) * 4
        frill_pts.append((fx, fy - fh))
    for f in range(len(frill_pts) - 1):
        draw.polygon([(cx - 22 + f*7, cy - 16 + bob_y), frill_pts[f], frill_pts[f+1], (cx - 22 + (f+1)*7, cy - 16 + bob_y)], fill=(240, 90, 25, 200))
        draw.line([frill_pts[f], frill_pts[f+1]], fill=(255, 220, 80, 240), width=2)

    # Head (Reptilian / Draconic Snout)
    hx = cx + 32 + math.cos(phase) * 3
    hy = cy - 12 + bob_y + math.sin(phase) * 2
    draw.polygon([
        (hx - 12, hy + 12), (hx - 10, hy - 14),
        (hx + 18, hy - 6), (hx + 28, hy + 4),
        (hx + 16, hy + 16), (hx - 2, hy + 16)
    ], fill=(48, 35, 45, 255))
    # Lower jaw
    draw.polygon([(hx, hy + 12), (hx + 22, hy + 6), (hx + 14, hy + 18)], fill=(38, 28, 35, 255))
    # Glowing Fire Mouth line
    draw.line([(hx + 4, hy + 10), (hx + 24, hy + 6)], fill=(255, 140, 30, 255), width=2)
    # Eye (Burning Topaz / Ruby)
    draw.ellipse([hx + 6, hy - 6, hx + 14, hy + 2], fill=(255, 40, 20, 255))
    draw.ellipse([hx + 8, hy - 4, hx + 12, hy], fill=(255, 230, 80, 255))
    draw.line([(hx + 10, hy - 5), (hx + 10, hy + 1)], fill=(20, 10, 15, 255), width=2)
    # Head Horn / Crest
    draw.polygon([(hx - 6, hy - 12), (hx - 16, hy - 24), (hx + 4, hy - 14)], fill=(32, 22, 30, 255))
    draw.line([(hx - 6, hy - 12), (hx - 16, hy - 24)], fill=(255, 120, 40, 255), width=2)

    add_ember_particles(draw, cx, cy + bob_y, 7, frame_idx, 202, spread_x=45, spread_y=35)
    return img

# -------------------------------------------------------------
# 3. OGNISTY CHOCHLIK (Fire Imp)
# -------------------------------------------------------------
def render_fire_imp_frame(frame_idx):
    img = Image.new("RGBA", (FRAME_SIZE, FRAME_SIZE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    t = frame_idx / NUM_FRAMES
    phase = t * 2 * math.pi

    cx, cy = 128, 125
    # Levitation hover
    hover_y = math.sin(phase) * 8.0
    wing_flap = math.sin(phase * 2)

    # Shadow on ground
    draw.ellipse([cx - 28, 185, cx + 28, 200], fill=(15, 10, 15, int(90 - hover_y * 3)))

    # Tail (long spaded devil tail)
    t_base_x, t_base_y = cx - 10, cy + 16 + hover_y
    tail_tip_x = cx - 35 + math.sin(phase) * 8
    tail_tip_y = cy + 30 + hover_y + math.cos(phase) * 6
    draw.line([(t_base_x, t_base_y), (cx - 24, cy + 28 + hover_y), (tail_tip_x, tail_tip_y)], fill=(120, 35, 30, 255), width=3)
    # Spade tip
    draw.polygon([
        (tail_tip_x - 7, tail_tip_y - 4), (tail_tip_x - 12, tail_tip_y + 8),
        (tail_tip_x, tail_tip_y + 2), (tail_tip_x + 3, tail_tip_y - 6)
    ], fill=(220, 60, 30, 255))

    # Wings (Bat-like with leathery orange membranes)
    for side in (-1, 1):
        wx = cx + side * 14
        wy = cy - 8 + hover_y
        w_span = side * (38 + wing_flap * 8)
        w_tip_y = wy - 35 + wing_flap * 15
        draw.polygon([
            (wx, wy),
            (wx + w_span * 0.6, wy - 38),
            (wx + w_span, w_tip_y),
            (wx + w_span * 0.85, wy - 5),
            (wx + w_span * 0.5, wy + 10),
            (wx + side * 8, wy + 16)
        ], fill=(160, 45, 25, 220))
        # Wing bones
        draw.line([(wx, wy), (wx + w_span * 0.6, wy - 38), (wx + w_span, w_tip_y)], fill=(65, 20, 20, 255), width=2)
        draw.line([(wx + w_span * 0.6, wy - 38), (wx + w_span * 0.85, wy - 5)], fill=(65, 20, 20, 255), width=2)
        draw.line([(wx + w_span * 0.6, wy - 38), (wx + w_span * 0.5, wy + 10)], fill=(255, 130, 40, 255), width=1)

    # Legs (Digitigrade imp legs tucked in flight)
    for side in (-1, 1):
        lx = cx + side * 8
        ly = cy + 18 + hover_y
        draw.line([(lx, ly), (lx + side * 6, ly + 14), (lx + side * 2, ly + 24)], fill=(110, 32, 28, 255), width=4)
        # Claws
        draw.polygon([(lx + side * 2 - 2, ly + 24), (lx + side * 2 + 2, ly + 24), (lx + side * 4, ly + 29)], fill=(240, 180, 50, 255))

    # Torso (Lean imp chest with glowing magma heart)
    draw.polygon([
        (cx - 14, cy - 14 + hover_y), (cx + 14, cy - 14 + hover_y),
        (cx + 10, cy + 16 + hover_y), (cx - 10, cy + 16 + hover_y)
    ], fill=(130, 38, 32, 255))
    # Glowing chest runes/core
    core_pulse = (math.sin(phase * 3) + 1) * 0.5
    draw.ellipse([cx - 6, cy - 6 + hover_y, cx + 6, cy + 6 + hover_y], fill=(255, int(120 + 80 * core_pulse), 30, 255))
    draw.ellipse([cx - 3, cy - 3 + hover_y, cx + 3, cy + 3 + hover_y], fill=(255, 240, 160, 255))

    # Left Hand (gesturing)
    draw.line([(cx - 12, cy - 8 + hover_y), (cx - 24, cy + hover_y), (cx - 28, cy - 12 + hover_y)], fill=(120, 35, 30, 255), width=3)
    draw.ellipse([cx - 30, cy - 15 + hover_y, cx - 24, cy - 9 + hover_y], fill=(255, 140, 40, 255))

    # Right Hand & Conjured Fireball!
    fb_x = cx + 30 + math.cos(phase * 2) * 4
    fb_y = cy - 10 + hover_y + math.sin(phase * 2) * 4
    draw.line([(cx + 12, cy - 8 + hover_y), (cx + 22, cy - 2 + hover_y), (fb_x - 4, fb_y + 4)], fill=(120, 35, 30, 255), width=3)

    # Fireball sphere with layers
    draw.ellipse([fb_x - 14, fb_y - 14, fb_x + 14, fb_y + 14], fill=(255, 80, 20, 180))
    draw.ellipse([fb_x - 10, fb_y - 10, fb_x + 10, fb_y + 10], fill=(255, 160, 40, 220))
    draw.ellipse([fb_x - 6, fb_y - 6, fb_x + 6, fb_y + 6], fill=(255, 240, 120, 255))
    # Whirling fire arcs around fireball
    arc_ang = phase * 4
    for a in range(3):
        ax = fb_x + math.cos(arc_ang + a * 2.1) * 16
        ay = fb_y + math.sin(arc_ang + a * 2.1) * 16
        draw.line([(fb_x, fb_y), (ax, ay)], fill=(255, 200, 60, 220), width=2)

    # Head (Imp face with huge obsidian horns)
    head_y = cy - 26 + hover_y
    draw.polygon([
        (cx - 12, head_y), (cx - 10, head_y - 14),
        (cx + 10, head_y - 14), (cx + 12, head_y),
        (cx, head_y + 8)
    ], fill=(140, 40, 32, 255))

    # Horns (Swept back curved horns)
    for side in (-1, 1):
        hx1, hy1 = cx + side * 8, head_y - 12
        hx2, hy2 = cx + side * 22, head_y - 28
        hx3, hy3 = cx + side * 16, head_y - 36
        draw.polygon([(hx1 - side * 3, hy1), (hx1 + side * 3, hy1), (hx2, hy2), (hx3, hy3)], fill=(32, 22, 28, 255))
        draw.line([(hx1, hy1), (hx2, hy2), (hx3, hy3)], fill=(255, 100, 30, 255), width=2)

    # Imp Face: Glowing yellow eyes, wicked grin
    draw.ellipse([cx - 8, head_y - 8, cx - 2, head_y - 2], fill=(255, 230, 60, 255))
    draw.ellipse([cx + 2, head_y - 8, cx + 8, head_y - 2], fill=(255, 230, 60, 255))
    draw.line([(cx - 5, head_y - 7), (cx - 5, head_y - 3)], fill=(20, 10, 10, 255), width=2)
    draw.line([(cx + 5, head_y - 7), (cx + 5, head_y - 3)], fill=(20, 10, 10, 255), width=2)
    # Pointed ears
    draw.polygon([(cx - 11, head_y - 4), (cx - 24, head_y - 10), (cx - 12, head_y + 2)], fill=(120, 32, 28, 255))
    draw.polygon([(cx + 11, head_y - 4), (cx + 24, head_y - 10), (cx + 12, head_y + 2)], fill=(120, 32, 28, 255))
    # Grin
    draw.line([(cx - 6, head_y + 3), (cx, head_y + 6), (cx + 6, head_y + 3)], fill=(255, 180, 50, 255), width=2)

    add_ember_particles(draw, cx, cy + hover_y, 8, frame_idx, 303, spread_x=30, spread_y=45)
    return img

# -------------------------------------------------------------
# 4. POPIELNY UPIÓR (Ash Wraith)
# -------------------------------------------------------------
def render_ash_wraith_frame(frame_idx):
    img = Image.new("RGBA", (FRAME_SIZE, FRAME_SIZE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    t = frame_idx / NUM_FRAMES
    phase = t * 2 * math.pi

    cx, cy = 128, 120
    float_y = math.sin(phase) * 6.0
    swirl = math.sin(phase * 1.5) * 5.0

    # Shadow on ground
    draw.ellipse([cx - 35, 195, cx + 35, 210], fill=(15, 10, 18, 90))

    # Tattered ethereal ash robes (wisps dispersing into smoke at bottom)
    for wisp in range(7):
        w_offset = (wisp - 3) * 11
        w_len = 50 + math.sin(phase * 2 + wisp) * 12
        w_curve = math.sin(phase + wisp * 0.8) * 14
        top_pt = (cx + w_offset * 0.6, cy + 15 + float_y)
        mid_pt = (cx + w_offset + w_curve * 0.5, cy + 45 + float_y)
        bot_pt = (cx + w_offset * 1.2 + w_curve, cy + 15 + w_len + float_y)
        draw.line([top_pt, mid_pt, bot_pt], fill=(42, 38, 50, 200), width=10)
        draw.line([top_pt, mid_pt, bot_pt], fill=(75, 68, 85, 220), width=6)
        draw.line([top_pt, mid_pt, bot_pt], fill=(140, 125, 150, 160), width=2)
        # Smoke particles at tips
        draw.ellipse([bot_pt[0] - 6, bot_pt[1] - 4, bot_pt[0] + 6, bot_pt[1] + 4], fill=(60, 52, 70, 120))

    # Inner Molten Core / Soul Glow
    core_glow = (math.sin(phase * 2) + 1) * 0.5
    draw.ellipse([cx - 24, cy - 10 + float_y, cx + 24, cy + 24 + float_y], fill=(220, int(80 + 40 * core_glow), 30, 170))
    draw.ellipse([cx - 16, cy - 4 + float_y, cx + 16, cy + 18 + float_y], fill=(255, 180, 50, 200))

    # Upper Shroud / Cowl & Shoulders
    draw.polygon([
        (cx - 36, cy - 10 + float_y), (cx - 20, cy - 36 + float_y),
        (cx + 20, cy - 36 + float_y), (cx + 36, cy - 10 + float_y),
        (cx + 26, cy + 22 + float_y), (cx - 26, cy + 22 + float_y)
    ], fill=(32, 28, 38, 255))
    # Robe folds
    draw.line([(cx - 20, cy - 36 + float_y), (cx - 8, cy + 18 + float_y)], fill=(85, 78, 98, 255), width=2)
    draw.line([(cx + 20, cy - 36 + float_y), (cx + 8, cy + 18 + float_y)], fill=(85, 78, 98, 255), width=2)

    # Skeletal / Ethereal Claw Hands holding Cinder Runes
    for side in (-1, 1):
        arm_sway = math.sin(phase + side) * 4
        h_x = cx + side * (32 + arm_sway)
        h_y = cy - 2 + float_y
        draw.line([(cx + side * 24, cy - 12 + float_y), (h_x, h_y)], fill=(65, 58, 75, 255), width=4)
        # Bony claw fingers
        for f in range(3):
            draw.line([(h_x, h_y), (h_x + side * (6 + f * 2), h_y + 4 + f * 3)], fill=(200, 185, 210, 255), width=2)
        # Floating cinder runes orbiting hands
        rune_a = phase * 3 + side * 2
        rx = h_x + math.cos(rune_a) * 12
        ry = h_y + math.sin(rune_a) * 10
        draw.polygon([(rx - 3, ry), (rx, ry - 4), (rx + 3, ry), (rx, ry + 4)], fill=(255, 160, 40, 255))

    # Obsidian Mask / Skull of the Wraith
    head_y = cy - 32 + float_y
    draw.polygon([
        (cx - 15, head_y - 12), (cx + 15, head_y - 12),
        (cx + 12, head_y + 14), (cx, head_y + 22),
        (cx - 12, head_y + 14)
    ], fill=(22, 18, 26, 255))
    # Chiseled mask facets
    draw.line([(cx, head_y - 12), (cx, head_y + 22)], fill=(55, 48, 65, 255), width=2)
    draw.line([(cx - 15, head_y - 12), (cx, head_y + 2), (cx + 15, head_y - 12)], fill=(55, 48, 65, 255), width=2)

    # Piercing Soul Eyes (Cyan / White Ghostfire inside magma cowl)
    for side in (-1, 1):
        draw.ellipse([cx + side * 7 - 3, head_y - 4, cx + side * 7 + 3, head_y + 4], fill=(40, 240, 255, 255))
        draw.ellipse([cx + side * 7 - 1.5, head_y - 2, cx + side * 7 + 1.5, head_y + 2], fill=(255, 255, 255, 255))

    # Swirling ash and cinder vortex
    add_ember_particles(draw, cx, cy + float_y, 10, frame_idx, 404, spread_x=45, spread_y=60)
    return img

# -------------------------------------------------------------
# 5. OBSYDIANOWY WOJOWNIK (Obsidian Sentinel)
# -------------------------------------------------------------
def render_obsidian_sentinel_frame(frame_idx):
    img = Image.new("RGBA", (FRAME_SIZE, FRAME_SIZE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    t = frame_idx / NUM_FRAMES
    phase = t * 2 * math.pi

    cx, cy = 128, 125
    bob_y = math.sin(phase * 2) * 2.0
    sway_x = math.sin(phase) * 2.0

    # Heavy Shadow
    draw.ellipse([cx - 48, 185, cx + 48, 205], fill=(12, 8, 15, 150))

    # Armored Greaves / Legs (Heavy Basalt Plates)
    leg_l_swing = math.sin(phase) * 4.0
    leg_r_swing = -leg_l_swing

    # Left Leg
    ll_x = cx - 18 + leg_l_swing * 0.5
    draw.polygon([
        (ll_x - 10, cy + 22 + bob_y), (ll_x + 8, cy + 22 + bob_y),
        (ll_x + 6, cy + 55), (ll_x - 12, cy + 55)
    ], fill=(25, 22, 30, 255))
    draw.line([(ll_x - 1, cy + 22 + bob_y), (ll_x - 1, cy + 55)], fill=(255, 120, 30, 255), width=2)  # magma conduit
    draw.polygon([(ll_x - 14, cy + 55), (ll_x + 10, cy + 55), (ll_x + 8, cy + 65), (ll_x - 16, cy + 65)], fill=(18, 15, 22, 255))

    # Right Leg
    rl_x = cx + 18 + leg_r_swing * 0.5
    draw.polygon([
        (rl_x - 8, cy + 22 + bob_y), (rl_x + 10, cy + 22 + bob_y),
        (rl_x + 12, cy + 55), (rl_x - 6, cy + 55)
    ], fill=(25, 22, 30, 255))
    draw.line([(rl_x + 1, cy + 22 + bob_y), (rl_x + 1, cy + 55)], fill=(255, 120, 30, 255), width=2)
    draw.polygon([(rl_x - 10, cy + 55), (rl_x + 14, cy + 55), (rl_x + 16, cy + 65), (rl_x - 8, cy + 65)], fill=(18, 15, 22, 255))

    # Cuirass / Torso (Sharply Faceted Obsidian Plates)
    draw.polygon([
        (cx - 26, cy - 18 + bob_y), (cx + 26, cy - 18 + bob_y),
        (cx + 20, cy + 22 + bob_y), (cx - 20, cy + 22 + bob_y)
    ], fill=(22, 19, 28, 255))
    # Obsidian Facet Glass Sheen
    draw.polygon([
        (cx - 26, cy - 18 + bob_y), (cx, cy - 22 + bob_y),
        (cx, cy + 18 + bob_y), (cx - 20, cy + 22 + bob_y)
    ], fill=(42, 38, 52, 255))
    # Glowing Runic Magma Seams
    draw.line([(cx, cy - 22 + bob_y), (cx, cy + 18 + bob_y)], fill=(255, 130, 30, 255), width=2)
    draw.line([(cx - 16, cy - 2 + bob_y), (cx, cy + 8 + bob_y), (cx + 16, cy - 2 + bob_y)], fill=(255, 180, 50, 255), width=2)

    # Massive Spiked Pauldrons (Shoulders)
    for side in (-1, 1):
        sh_x = cx + side * 34 + sway_x * 0.5
        sh_y = cy - 20 + bob_y
        draw.polygon([
            (sh_x - side * 14, sh_y + 10),
            (sh_x, sh_y - 12),
            (sh_x + side * 22, sh_y - 20),
            (sh_x + side * 16, sh_y + 8)
        ], fill=(30, 26, 36, 255))
        # Specular reflection edge
        draw.line([(sh_x, sh_y - 12), (sh_x + side * 22, sh_y - 20)], fill=(160, 150, 180, 255), width=2)
        draw.line([(sh_x - side * 14, sh_y + 10), (sh_x + side * 16, sh_y + 8)], fill=(255, 100, 25, 255), width=2)

    # Left Arm & Shield / Fist
    draw.polygon([
        (cx - 36, cy - 10 + bob_y), (cx - 46, cy + 10 + bob_y),
        (cx - 40, cy + 26 + bob_y), (cx - 30, cy + 12 + bob_y)
    ], fill=(24, 20, 30, 255))
    # Obsidian Gauntlet
    draw.polygon([(cx - 44, cy + 22 + bob_y), (cx - 36, cy + 22 + bob_y), (cx - 38, cy + 34 + bob_y), (cx - 46, cy + 32 + bob_y)], fill=(45, 40, 55, 255))

    # Right Arm & Colossal Obsidian Greatsword
    sword_rot = math.sin(phase) * 0.08
    arm_x = cx + 32
    draw.polygon([
        (arm_x, cy - 10 + bob_y), (arm_x + 12, cy + 4 + bob_y),
        (arm_x + 6, cy + 22 + bob_y), (arm_x - 4, cy + 12 + bob_y)
    ], fill=(24, 20, 30, 255))

    # Greatsword: Black volcanic glass blade with jagged serrations and molten core
    sw_x = arm_x + 12
    sw_y = cy + 14 + bob_y
    # Guard & Hilt
    draw.line([(sw_x - 10, sw_y), (sw_x + 10, sw_y)], fill=(180, 140, 60, 255), width=4)
    draw.line([(sw_x, sw_y - 12), (sw_x, sw_y + 4)], fill=(80, 50, 30, 255), width=4)
    # Blade (Huge 85px slab)
    blade_pts = [
        (sw_x - 8, sw_y + 4),
        (sw_x - 9, sw_y + 55),
        (sw_x - 14, sw_y + 65),  # serration
        (sw_x, sw_y + 82),      # sharp point
        (sw_x + 14, sw_y + 65),
        (sw_x + 9, sw_y + 55),
        (sw_x + 8, sw_y + 4)
    ]
    draw.polygon(blade_pts, fill=(18, 16, 22, 255))
    # Mirror reflection facet
    draw.polygon([
        (sw_x - 8, sw_y + 4), (sw_x, sw_y + 4),
        (sw_x, sw_y + 82), (sw_x - 9, sw_y + 55)
    ], fill=(36, 32, 44, 255))
    draw.line([(sw_x - 8, sw_y + 4), (sw_x - 14, sw_y + 65), (sw_x, sw_y + 82)], fill=(140, 130, 160, 255), width=2)
    # Glowing Molten Fullers / Runes down center of blade
    draw.line([(sw_x, sw_y + 8), (sw_x, sw_y + 70)], fill=(255, 140, 40, 255), width=3)
    draw.line([(sw_x, sw_y + 12), (sw_x, sw_y + 66)], fill=(255, 230, 100, 255), width=2)

    # Helm (Great Horned Barbute with Slit Visor)
    head_y = cy - 36 + bob_y
    draw.polygon([
        (cx - 14, head_y - 12), (cx + 14, head_y - 12),
        (cx + 16, head_y + 12), (cx, head_y + 20),
        (cx - 16, head_y + 12)
    ], fill=(22, 18, 26, 255))
    # Obsidian Crest Horns
    for side in (-1, 1):
        draw.polygon([
            (cx + side * 10, head_y - 8),
            (cx + side * 24, head_y - 24),
            (cx + side * 18, head_y - 6)
        ], fill=(35, 30, 42, 255))
        draw.line([(cx + side * 10, head_y - 8), (cx + side * 24, head_y - 24)], fill=(180, 170, 200, 255), width=2)

    # Blazing Visor Slit
    visor_glow = (math.sin(phase * 3) + 1) * 0.5
    draw.line([(cx - 10, head_y + 2), (cx + 10, head_y + 2)], fill=(255, int(100 + 80 * visor_glow), 20, 255), width=3)
    draw.line([(cx - 6, head_y + 2), (cx + 6, head_y + 2)], fill=(255, 240, 120, 255), width=2)

    add_ember_particles(draw, cx, cy + bob_y, 6, frame_idx, 505, spread_x=40, spread_y=55)
    return img

# -------------------------------------------------------------
# 6. ŻAROWY DRAK (Ember Drake)
# -------------------------------------------------------------
def render_ember_drake_frame(frame_idx):
    img = Image.new("RGBA", (FRAME_SIZE, FRAME_SIZE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    t = frame_idx / NUM_FRAMES
    phase = t * 2 * math.pi

    cx, cy = 128, 128
    # Flying / Hovering heave
    hover_y = math.sin(phase) * 7.0
    wing_stroke = math.sin(phase)

    # Ground shadow
    draw.ellipse([cx - 55, 192, cx + 55, 212], fill=(15, 10, 15, int(110 - hover_y * 2)))

    # Long Spiked Dragon Tail
    tail_pts = []
    for i in range(10):
        frac = i / 9.0
        w = math.sin(phase - frac * 2.5) * (18.0 * frac)
        tx = cx - 20 - frac * 75 + w
        ty = cy + 18 + frac * 8 + hover_y
        tail_pts.append((tx, ty))

    for i in range(len(tail_pts) - 1):
        thick = int(12 * (1.0 - i / len(tail_pts))) + 2
        draw.line([tail_pts[i], tail_pts[i+1]], fill=(45, 32, 40, 255), width=thick)
        draw.line([tail_pts[i], tail_pts[i+1]], fill=(220, 75, 25, 255), width=max(1, thick - 3))
    # Tail Blade / Spike
    end_pt = tail_pts[-1]
    draw.polygon([
        (end_pt[0] - 12, end_pt[1] - 4), (end_pt[0] + 4, end_pt[1] - 8),
        (end_pt[0] + 6, end_pt[1] + 6), (end_pt[0] - 8, end_pt[1] + 8)
    ], fill=(30, 22, 30, 255))
    draw.line([(end_pt[0] - 12, end_pt[1] - 4), (end_pt[0] + 6, end_pt[1] + 6)], fill=(255, 140, 40, 255), width=2)

    # Massive Dragon Wings (Spread with fiery vein patterns)
    for side in (-1, 1):
        wx = cx + side * 18
        wy = cy - 8 + hover_y
        span_x = side * (75 - abs(wing_stroke) * 12)
        tip_y = wy - 45 + wing_stroke * 22
        elbow_x = wx + side * 45
        elbow_y = wy - 35 + wing_stroke * 15

        # Membrane polygons
        draw.polygon([
            (wx, wy), (elbow_x, elbow_y), (wx + span_x, tip_y),
            (wx + span_x * 0.8, wy + 15), (wx + span_x * 0.5, wy + 25), (wx, wy + 18)
        ], fill=(150, 40, 25, 220))
        # Membrane sub-panels (translucent fiery glow)
        draw.polygon([(wx + 8, wy + 2), (elbow_x - side * 4, elbow_y + 4), (wx + span_x * 0.7, wy + 10)], fill=(255, 110, 30, 160))

        # Main wing arm bone
        draw.line([(wx, wy), (elbow_x, elbow_y), (wx + span_x, tip_y)], fill=(40, 28, 35, 255), width=4)
        draw.line([(wx, wy), (elbow_x, elbow_y), (wx + span_x, tip_y)], fill=(85, 60, 75, 255), width=2)
        # Finger struts
        draw.line([(elbow_x, elbow_y), (wx + span_x * 0.8, wy + 15)], fill=(50, 35, 45, 255), width=2)
        draw.line([(elbow_x, elbow_y), (wx + span_x * 0.5, wy + 25)], fill=(50, 35, 45, 255), width=2)
        # Lava webbing veins
        draw.line([(wx + span_x * 0.3, wy + 5), (wx + span_x * 0.6, wy + 8)], fill=(255, 190, 60, 200), width=2)

    # Hind Legs & Talons
    for side in (-1, 1):
        leg_x = cx + side * 16
        leg_y = cy + 14 + hover_y
        draw.line([(leg_x, leg_y), (leg_x + side * 8, leg_y + 18), (leg_x + side * 4, leg_y + 30)], fill=(48, 34, 42, 255), width=5)
        # 3 sharp talons
        for t_idx in (-4, 0, 4):
            draw.line([(leg_x + side * 4, leg_y + 30), (leg_x + side * 4 + t_idx, leg_y + 36)], fill=(255, 180, 50, 255), width=2)

    # Dragon Torso & Molten Ribcage
    draw.polygon([
        (cx - 24, cy - 14 + hover_y), (cx + 24, cy - 14 + hover_y),
        (cx + 18, cy + 18 + hover_y), (cx - 18, cy + 18 + hover_y)
    ], fill=(42, 30, 38, 255))
    # Glowing lava chest (breathing fire within)
    core_t = (math.sin(phase * 2) + 1) * 0.5
    draw.ellipse([cx - 12, cy - 6 + hover_y, cx + 12, cy + 12 + hover_y], fill=(255, int(90 + 60 * core_t), 20, 220))
    # Ribcage silhouettes over glow
    for rib in range(3):
        ry = cy - 2 + rib * 6 + hover_y
        draw.line([(cx - 14, ry), (cx - 3, ry + 2)], fill=(32, 22, 28, 255), width=2)
        draw.line([(cx + 14, ry), (cx + 3, ry + 2)], fill=(32, 22, 28, 255), width=2)

    # S-Curved Neck & Draconic Head
    neck_x = cx + 22 + math.cos(phase) * 3
    neck_y = cy - 22 + hover_y
    draw.polygon([
        (cx + 8, cy - 14 + hover_y), (neck_x, neck_y),
        (neck_x + 12, neck_y - 2), (cx + 20, cy - 10 + hover_y)
    ], fill=(48, 34, 44, 255))
    # Neck dorsal spines
    for sp in range(4):
        sp_x = cx + 10 + sp * 4
        sp_y = cy - 16 - sp * 3 + hover_y
        draw.polygon([(sp_x, sp_y), (sp_x - 3, sp_y - 8), (sp_x + 4, sp_y - 2)], fill=(255, 100, 30, 255))

    # Dragon Head (Horned, fearsome maw)
    hx = neck_x + 10
    hy = neck_y - 8
    draw.polygon([
        (hx - 8, hy + 10), (hx - 4, hy - 14),
        (hx + 24, hy - 6), (hx + 34, hy + 4),
        (hx + 18, hy + 14)
    ], fill=(52, 38, 48, 255))
    # Backward sweep Dragon Horns
    draw.polygon([(hx + 2, hy - 12), (hx - 14, hy - 32), (hx + 12, hy - 14)], fill=(30, 22, 28, 255))
    draw.line([(hx + 2, hy - 12), (hx - 14, hy - 32)], fill=(255, 140, 40, 255), width=2)

    # Fiery Maw & Sharp Fangs
    draw.polygon([(hx + 4, hy + 8), (hx + 30, hy + 4), (hx + 22, hy + 16)], fill=(38, 26, 34, 255))
    draw.line([(hx + 10, hy + 7), (hx + 28, hy + 5)], fill=(255, 160, 40, 255), width=2)
    # Fangs
    draw.polygon([(hx + 18, hy + 5), (hx + 20, hy + 11), (hx + 22, hy + 5)], fill=(255, 240, 180, 255))
    # Blazing Golden Reptile Eye
    draw.ellipse([hx + 10, hy - 6, hx + 18, hy + 2], fill=(255, 210, 50, 255))
    draw.ellipse([hx + 12, hy - 4, hx + 16, hy], fill=(255, 60, 20, 255))
    draw.line([(hx + 14, hy - 5), (hx + 14, hy + 1)], fill=(15, 8, 12, 255), width=2)

    add_ember_particles(draw, cx, cy + hover_y, 9, frame_idx, 606, spread_x=55, spread_y=45)
    return img

# -------------------------------------------------------------
# 7. VARKHOR, SERCE KALDERA (Volcanic Titan Boss)
# -------------------------------------------------------------
def render_varkhor_frame(frame_idx):
    img = Image.new("RGBA", (FRAME_SIZE, FRAME_SIZE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    t = frame_idx / NUM_FRAMES
    phase = t * 2 * math.pi

    cx, cy = 128, 120
    # Tremendous heavy breathing and seismic rumble
    bob_y = math.sin(phase) * 3.5
    chest_swell = (math.sin(phase) + 1) * 0.5

    # Titanic ground shadow
    draw.ellipse([cx - 65, 190, cx + 65, 216], fill=(10, 5, 12, 170))

    # Massive Basalt Column Legs
    leg_l_offset = math.sin(phase) * 2.0
    # Left Pillar Leg
    draw.polygon([
        (cx - 38, cy + 28 + bob_y), (cx - 10, cy + 28 + bob_y),
        (cx - 14, cy + 72), (cx - 46, cy + 72)
    ], fill=(24, 20, 28, 255))
    # Magma fissures along leg
    draw.line([(cx - 24, cy + 28 + bob_y), (cx - 28, cy + 72)], fill=(255, 110, 30, 255), width=3)
    draw.polygon([(cx - 50, cy + 70), (cx - 10, cy + 70), (cx - 8, cy + 82), (cx - 52, cy + 82)], fill=(16, 12, 20, 255))

    # Right Pillar Leg
    draw.polygon([
        (cx + 10, cy + 28 + bob_y), (cx + 38, cy + 28 + bob_y),
        (cx + 46, cy + 72), (cx + 14, cy + 72)
    ], fill=(24, 20, 28, 255))
    draw.line([(cx + 24, cy + 28 + bob_y), (cx + 28, cy + 72)], fill=(255, 110, 30, 255), width=3)
    draw.polygon([(cx + 10, cy + 70), (cx + 50, cy + 70), (cx + 52, cy + 82), (cx + 8, cy + 82)], fill=(16, 12, 20, 255))

    # Volcanic Shoulder Vents emitting smoke & fire
    for side in (-1, 1):
        vx = cx + side * 44
        vy = cy - 32 + bob_y
        draw.polygon([(vx - 8, vy + 14), (vx + 8, vy + 14), (vx + 5, vy - 10), (vx - 5, vy - 10)], fill=(35, 28, 38, 255))
        # Vent orifice glowing white-hot
        draw.ellipse([vx - 5, vy - 12, vx + 5, vy - 8], fill=(255, 220, 100, 255))
        # Smoke plume puff
        smoke_t = (t + side * 0.25) % 1.0
        smk_y = vy - 15 - smoke_t * 28
        smk_r = 6 + smoke_t * 10
        smk_alpha = int(180 * (1.0 - smoke_t))
        draw.ellipse([vx - smk_r, smk_y - smk_r * 0.6, vx + smk_r, smk_y + smk_r * 0.6], fill=(55, 45, 60, smk_alpha))

    # Colossal Torso (Tectonic Carapace)
    torso_w = 42 + chest_swell * 4
    draw.polygon([
        (cx - torso_w, cy - 25 + bob_y), (cx + torso_w, cy - 25 + bob_y),
        (cx + 32, cy + 30 + bob_y), (cx - 32, cy + 30 + bob_y)
    ], fill=(28, 22, 32, 255))

    # "THE HEART OF THE CALDERA" — Pulsing Central Magma Core
    core_rad = 18 + chest_swell * 6
    draw.ellipse([cx - core_rad, cy - 2 - core_rad * 0.8 + bob_y, cx + core_rad, cy - 2 + core_rad * 0.8 + bob_y], fill=(255, 60, 15, 220))
    draw.ellipse([cx - core_rad * 0.7, cy - 2 - core_rad * 0.5 + bob_y, cx + core_rad * 0.7, cy - 2 + core_rad * 0.5 + bob_y], fill=(255, 160, 40, 240))
    draw.ellipse([cx - core_rad * 0.35, cy - 2 - core_rad * 0.25 + bob_y, cx + core_rad * 0.35, cy - 2 + core_rad * 0.25 + bob_y], fill=(255, 250, 180, 255))

    # Hardened Tectonic Crust Plates overlapping the Core
    draw.polygon([(cx - 36, cy - 20 + bob_y), (cx - 14, cy - 12 + bob_y), (cx - 22, cy + 15 + bob_y), (cx - 34, cy + 5 + bob_y)], fill=(45, 36, 48, 255))
    draw.polygon([(cx + 36, cy - 20 + bob_y), (cx + 14, cy - 12 + bob_y), (cx + 22, cy + 15 + bob_y), (cx + 34, cy + 5 + bob_y)], fill=(45, 36, 48, 255))
    draw.polygon([(cx - 16, cy + 14 + bob_y), (cx + 16, cy + 14 + bob_y), (cx, cy + 28 + bob_y)], fill=(38, 30, 42, 255))

    # Massive Spiked Pauldrons (Craggy Volcanic Boulders)
    for side in (-1, 1):
        px = cx + side * 50
        py = cy - 15 + bob_y
        draw.polygon([
            (px - side * 16, py + 18),
            (px - side * 4, py - 18),
            (px + side * 24, py - 24),
            (px + side * 22, py + 12)
        ], fill=(36, 28, 40, 255))
        draw.line([(px - side * 4, py - 18), (px + side * 24, py - 24)], fill=(85, 72, 92, 255), width=2)
        draw.line([(px, py - 6), (px + side * 18, py - 4)], fill=(255, 130, 35, 255), width=2)

    # Titanic Arms & Molten Boulder Fists
    for side in (-1, 1):
        ax = cx + side * 52
        ay = cy + 10 + bob_y
        draw.polygon([(ax - 10, ay - 14), (ax + 10, ay - 14), (ax + side * 12, ay + 20), (ax - side * 4, ay + 20)], fill=(30, 24, 34, 255))
        # Boulder Fist
        fx = ax + side * 6
        fy = ay + 32
        draw.polygon([
            (fx - 14, fy - 12), (fx + 14, fy - 12),
            (fx + 16, fy + 12), (fx - 16, fy + 12)
        ], fill=(42, 32, 46, 255))
        # Glowing Molten Knuckles
        for k in range(3):
            kx = fx - 8 + k * 8
            draw.ellipse([kx - 3, fy + 4, kx + 3, fy + 10], fill=(255, 170, 50, 255))

    # Head (Volcanic Mountain Crown with Burning Runic Face)
    head_y = cy - 42 + bob_y
    draw.polygon([
        (cx - 18, head_y - 10), (cx - 10, head_y - 24),
        (cx + 10, head_y - 24), (cx + 18, head_y - 10),
        (cx + 14, head_y + 16), (cx, head_y + 24),
        (cx - 14, head_y + 16)
    ], fill=(26, 20, 30, 255))

    # Triple Basalt Crown Horns
    draw.polygon([(cx - 14, head_y - 18), (cx - 26, head_y - 42), (cx - 6, head_y - 22)], fill=(38, 30, 44, 255))
    draw.polygon([(cx + 6, head_y - 22), (cx + 26, head_y - 42), (cx + 14, head_y - 18)], fill=(38, 30, 44, 255))
    draw.polygon([(cx - 7, head_y - 22), (cx, head_y - 46), (cx + 7, head_y - 22)], fill=(48, 38, 54, 255))
    draw.line([(cx, head_y - 46), (cx, head_y - 22)], fill=(255, 150, 40, 255), width=2)

    # Face Carvings: Burning Eyes and Molten Maw
    for side in (-1, 1):
        draw.ellipse([cx + side * 9 - 3, head_y - 4, cx + side * 9 + 3, head_y + 4], fill=(255, 230, 70, 255))
        draw.ellipse([cx + side * 9 - 1.5, head_y - 2, cx + side * 9 + 1.5, head_y + 2], fill=(255, 255, 200, 255))
    # Jagged Maw of Fire
    draw.polygon([
        (cx - 10, head_y + 8), (cx - 5, head_y + 14),
        (cx, head_y + 10), (cx + 5, head_y + 14), (cx + 10, head_y + 8),
        (cx + 6, head_y + 18), (cx - 6, head_y + 18)
    ], fill=(255, 120, 20, 255))
    draw.polygon([(cx - 2, head_y + 11), (cx + 2, head_y + 11), (cx, head_y + 16)], fill=(255, 240, 160, 255))

    add_ember_particles(draw, cx, cy - 10 + bob_y, 14, frame_idx, 707, spread_x=65, spread_y=70)
    return img

# -------------------------------------------------------------
# MAIN BATCH EXECUTION
# -------------------------------------------------------------
ENEMIES = [
    ("slag_crawler", "Żużlowy pełzacz", "Pancernik kaldery", render_slag_crawler_frame),
    ("lava_salamander", "Salamandra lawowa", "Ognisty gad", render_lava_salamander_frame),
    ("fire_imp", "Ognisty chochlik", "Demon żaru", render_fire_imp_frame),
    ("ash_wraith", "Popielny upiór", "Duch pogorzeliska", render_ash_wraith_frame),
    ("obsidian_sentinel", "Obsydianowy wojownik", "Strażnik kuźni", render_obsidian_sentinel_frame),
    ("ember_drake", "Żarowy drak", "Skrzydlata bestia", render_ember_drake_frame),
    ("varkhor", "Varkhor, Serce Kaldery", "Władca wulkanu — Boss", render_varkhor_frame),
]

def main():
    print(f"Generating enhanced animations for {len(ENEMIES)} Caldera enemies...")
    manifest = []

    for key, name_pl, role, render_func in ENEMIES:
        print(f"Rendering {name_pl} ({key})...", flush=True)
        frames = []
        for f in range(NUM_FRAMES):
            frame_img = render_func(f)
            frames.append(frame_img)

        # Save animated GIF directly in ARTIFACT_DIR for direct markdown viewing!
        gif_path = ARTIFACT_DIR / f"{key}_animated.gif"
        frames[0].save(
            gif_path,
            save_all=True,
            append_images=frames[1:],
            duration=65,  # ~15 FPS smooth loop
            loop=0,
            disposal=2
        )

        # Also save static high-res preview frame
        preview_path = ARTIFACT_DIR / f"{key}_preview.png"
        frames[0].save(preview_path)

        # Build horizontal sprite strip
        strip = Image.new("RGBA", (FRAME_SIZE * NUM_FRAMES, FRAME_SIZE), (0, 0, 0, 0))
        for idx, f_img in enumerate(frames):
            strip.paste(f_img, (idx * FRAME_SIZE, 0))
        strip_path = STAGING_DIR / f"{key}_strip.png"
        strip.save(strip_path)

        manifest.append({
            "key": key,
            "name": name_pl,
            "role": role,
            "gif": str(gif_path.name),
            "preview": str(preview_path.name),
            "frames": NUM_FRAMES,
            "frame_size": FRAME_SIZE,
        })
        print(f" -> Saved {gif_path.name} ({os.path.getsize(gif_path)} bytes)")

    # Save manifest
    manifest_path = ARTIFACT_DIR / "redesigned_manifest.json"
    with open(manifest_path, "w", encoding="utf-8") as f:
        json.dump(manifest, f, indent=2, ensure_ascii=False)
    print("All animations generated successfully!")

if __name__ == "__main__":
    main()
