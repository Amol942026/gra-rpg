"""Stitch the painted yaw views into one continuous, opaque 2D material."""
import numpy as np
from PIL import Image, ImageDraw
from scipy import ndimage as ndi

from marsh_rebuild_paint import cycle, register


STRIP_WIDTH = 1024


def sample(channels, coordinates):
    return np.stack([ndi.map_coordinates(channel, coordinates, order=1,
                                         mode="nearest", prefilter=False)
                     for channel in channels])


def seam_path(cost):
    """Follow matching paint while keeping the junction continuous down the strip."""
    height, width = cost.shape
    columns = np.arange(width)
    choices = np.clip(columns + np.array([-1, 0, 1])[:, None], 0, width-1)
    roll_cost = np.array([[.9], [0.], [.9]])
    ranks = np.empty((height, width), dtype=np.int8)
    score = cost[0]
    for y in range(1, height):
        search = score[choices] + roll_cost
        ranks[y] = np.argmin(search, axis=0)
        score = cost[y] + np.min(search, axis=0)
    path = np.empty(height, dtype=int)
    path[-1] = int(np.argmin(score))
    for y in range(height-1, 0, -1):
        path[y-1] = choices[ranks[y, path[y]], path[y]]
    return path


def cloak_paint(color, grid, heading):
    """Fill the location of the metal prop with neighbouring cloth."""
    bounds = {0: (116, 140, -24, 0), 45: (101, 134, -32, 0),
              90: (85, 119, 0, 42), 135: (70, 103, 32, 0), 180: (64, 88, 24, 0)}
    if heading not in bounds:
        return color
    left, right, dx, dy = bounds[heading]
    x, y = grid[1], grid[0]
    # Fill the side views' bell patch with colour from the adjacent cloak.
    inset = np.minimum(np.minimum(x - left, right - x), np.minimum(y - 39, 77 - y))
    weight = np.clip(inset / 3, 0., 1.)[..., None]
    cloth = sample(color.transpose(2, 0, 1), (y + dy, x + dx)).transpose(1, 2, 0)
    return color + (cloth - color) * weight


def stitch(paintings, headings, base, grid, keeper=False):
    longitude = np.arange(STRIP_WIDTH) * (2 * np.pi / STRIP_WIDTH)
    rows = grid[0, :, 0]
    sources, visibility = [], []
    for heading, image in zip(headings, paintings):
        left, right, signed, color = register(image)
        center = (left + right) * .5
        radius = (right - left) * .5
        offset = longitude - np.deg2rad(heading)
        x = center[:, None] + .97*radius[:, None]*np.sin(offset)
        at = np.broadcast_to(rows[:, None], x.shape), x
        if keeper:
            color = cloak_paint(color, grid, heading)
        sources.append(sample(color.transpose(2, 0, 1), at).transpose(1, 2, 0))
        visibility.append(sample(signed[None], at)[0])

    height = len(rows)
    y = np.arange(height)
    columns = np.arange(STRIP_WIDTH)
    seams = []
    correction = np.zeros((height, STRIP_WIDTH, 3), dtype="f4")
    for i, left in enumerate(sources):
        following = (i+1) % len(sources)
        right = sources[following]
        beginning = headings[i]
        ending = headings[following] if following else headings[0] + 360
        center = (beginning + ending) * (.5 * STRIP_WIDTH / 360)
        span = max(2, round((ending-beginning)*(STRIP_WIDTH/360)*.26))
        window = np.arange(round(center)-span, round(center)+span+1) % STRIP_WIDTH
        difference = left[:, window] - right[:, window]
        visible = (visibility[i][:, window] > 0) & (visibility[following][:, window] > 0)
        # Bias towards an unbroken paint junction near the intended heading.
        mismatch = np.mean(np.abs(difference), axis=2)
        offset_cost = np.linspace(-1., 1., len(window))**2 * 5
        cut = window[seam_path(mismatch * np.where(visible, 1., .28) + offset_cost)]
        seams.append(cut)
        gap = (columns[None, :] - cut[:, None] + STRIP_WIDTH//2) % STRIP_WIDTH - STRIP_WIDTH//2
        # Match the slow change in tone at the join. The texture itself
        # stays from the selected drawing on either side of the junction.
        shift = np.clip(ndi.gaussian_filter1d((left[y, cut] - right[y, cut])*.5, 4, axis=0), -25, 25)
        width = max(2, span)
        falloff = np.clip(1 - np.abs(gap+.5)/width, 0, 1)**2
        correction += shift[:, None, :] * (np.sign(gap+.5)*falloff)[..., None]

    # Assign the full interval between seams (including the east wrap).
    owner = np.sum(columns >= np.asarray(seams)[..., None], axis=0) % len(sources)
    material = np.stack(sources)[owner, y[:, None], columns]
    material = np.clip(material + correction, 0., 255.)
    edges = [register(frame)[:2] for frame in base]
    centers = cycle(np.asarray([(a+b)*.5 for a,b in edges]))
    radii = cycle(np.asarray([(b-a)*.5 for a,b in edges]))
    return np.concatenate((material, material[:, :1]), axis=1).transpose(2, 0, 1), centers, radii


def surface_angle(material, grid, yaw, alpha):
    """Fit the sides of the material to the actual painted silhouette."""
    _, centers, radii = material
    x = grid[1] - centers(yaw)[:, None]
    visible = alpha >= .5
    original_radius = .97 * radii(yaw)
    fits = []
    for sign in (-1, 1):
        extent = np.max(np.where(visible, sign*x, 0.), axis=1)
        rounded = ndi.gaussian_filter1d(extent, 1.2)
        fits.append(np.maximum.reduce((original_radius, extent, rounded)) + .8)
    lateral = x / np.where(x < 0, fits[0][:, None], fits[1][:, None])
    return np.arcsin(.965 * np.clip(lateral, -1., 1.)) / (2 * np.pi)


def project(material, grid, yaw, alpha):
    strip = material[0]
    angle = surface_angle(material, grid, yaw, alpha)
    longitude = ((yaw / 360 + angle) % 1.) * STRIP_WIDTH
    return sample(strip, (grid[0], longitude)).transpose(1, 2, 0)


def painted_bell(front):
    """Cut out the original bell and paint the volume of its lower opening."""
    height, width = front.shape[:2]
    scale = 4

    def mask(points):
        shape = Image.new("L", (width*scale, height*scale))
        draw = ImageDraw.Draw(shape)
        if len(points) == 2:
            draw.ellipse([tuple(round(v*scale) for v in p) for p in points], fill=255)
        else:
            draw.polygon([tuple(round(v*scale) for v in p) for p in points], fill=255)
        return np.asarray(shape.resize((width, height), Image.Resampling.LANCZOS), dtype="f4") / 255.

    body = mask([(97, 43), (106, 43), (110, 48), (111, 58), (115, 67),
                 (113, 71), (102, 74), (91, 71), (89, 68), (94, 58), (94, 49)])
    rim = mask([(89, 65), (115, 75)])
    opening = mask([(92, 68), (112, 73)])
    # An oval rim and a dark inner wall distinguish the mouth from the body.
    paint = front[..., :3].astype("f4")
    paint = paint*(1-rim[..., None]) + rim[..., None]*np.float32((115, 99, 67))
    paint = paint*(1-opening[..., None]) + opening[..., None]*np.float32((43, 39, 29))
    alpha = np.maximum(body, rim)
    return np.dstack((paint*alpha[..., None], alpha)).transpose(2, 0, 1)


