"""Compose directional sprites from painted 2D body and limb pieces."""
import math

import numpy as np
from PIL import Image, ImageDraw
from scipy import ndimage as ndi

from marsh_surface_paint import project, sample, stitch


def smooth(value):
    t = np.clip(value, 0., 1.)
    return t*t*(3 - 2*t)


def piece(image, corners, pivot):
    mask = Image.new("L", (image.shape[1]*3, image.shape[0]*3))
    ImageDraw.Draw(mask).polygon([(3*x, 3*y) for x, y in corners], fill=255)
    coverage = np.asarray(mask.resize((image.shape[1], image.shape[0]),
                                      Image.Resampling.LANCZOS), dtype="f4") / 255.
    coverage *= image[..., 3]/255.
    rgba = np.dstack((image[..., :3]*coverage[..., None], coverage))
    return rgba.transpose(2, 0, 1), pivot


def position(layer, grid, x, y, width=1., tilt=0.):
    image, pivot = layer
    dx, dy = grid[1] - x, grid[0] - y
    c, s = math.cos(tilt), math.sin(tilt)
    sx = (dx*c+dy*s)/width + pivot[0]
    sy = dy*c-dx*s + pivot[1]
    return sample(image, (sy, sx)).transpose(1, 2, 0)


def composite(alpha, paint, limbs):
    solid = np.dstack((paint*alpha[..., None], alpha))
    # Only the part behind the trunk is masked. A sole below the body stays
    # opaque as its joint crosses from the front to the back of the actor.
    for depth, layer in sorted(limbs, key=lambda entry: entry[0]):
        visible = 1 - alpha * (1 - smooth((depth + 5.) / 10.))
        limb = layer * visible[..., None]
        solid = solid * (1-limb[..., 3:4]) + limb
    color = np.divide(solid[..., :3], solid[..., 3:4], out=np.zeros_like(paint),
                      where=solid[..., 3:4] > .00001)
    return np.clip(np.rint(np.dstack((color, solid[..., 3]*255))), 0, 255).astype("u1")


def horizontal_profile(image, rows, bottom):
    opaque = image[..., 3] >= 128
    occupied = np.flatnonzero(opaque.any(axis=1))
    source_rows = np.linspace(occupied[0], min(bottom, occupied[-1]), len(rows))
    x = np.arange(image.shape[1])
    left = np.min(np.where(opaque, x, image.shape[1]), axis=1)[occupied]
    right = np.max(np.where(opaque, x, 0), axis=1)[occupied]
    center = np.interp(source_rows, occupied, (left+right)*.5)
    radius = np.interp(source_rows, occupied, (right-left+1)*.5)
    return ndi.gaussian_filter1d(center, 1.1), ndi.gaussian_filter1d(radius, 1.1)


class ToadTurn:
    """A fixed-height painted body and four independently placed feet."""
    def __init__(self, paintings, grid):
        self.grid = grid
        self.surface = stitch(paintings, list(range(0, 360, 45)), paintings, grid)
        self.rows = np.arange(25, 83)
        self.side_centers, self.side_radius = horizontal_profile(paintings[0], self.rows, 76)
        _, self.front_radius = horizontal_profile(paintings[2], self.rows, 74)
        _, self.back_radius = horizontal_profile(paintings[6], self.rows, 74)
        self.feet = (
            (27., piece(paintings[0], [(83,64),(95,63),(103,70),(102,80),(111,87),(109,91),
                                       (93,90),(88,85),(86,77)], (97,88)), 13.),
            (-24., piece(paintings[0], [(35,62),(50,61),(59,67),(62,78),(57,85),(63,90),
                                        (43,91),(31,86),(27,75)], (46,88)), 19.),
        )

    def frame(self, yaw):
        angle = math.radians(yaw)
        c, s = math.cos(angle), math.sin(angle)
        front = (1+s)*.5
        across = self.back_radius + (self.front_radius-self.back_radius)*front
        half = np.hypot(self.side_radius*c, across*s)
        center = 70 + (self.side_centers-70)*c
        # The cap and underside taper in the same rows in every view.
        taper = np.sqrt(smooth((self.rows-22.)/8.)*smooth((85.-self.rows)/15.))
        alpha = np.zeros_like(self.grid[0])
        alpha[self.rows] = np.clip(.5 + (half*taper)[:, None] -
                                  np.abs(self.grid[1, self.rows]-center[:, None]), 0., 1.)
        paint = project(self.surface, self.grid, yaw, alpha)
        limbs = []
        for along, sprite, spacing in self.feet:
            for side in (-spacing, spacing):
                depth = along*s-side*c
                sign = 1. if side>0 else -1.
                width = (.78+.22*c*c)*sign
                limbs.append((depth, position(sprite, self.grid, 70+along*c+side*s,
                                              84.+.23*depth, width, .34*sign*s)))
        return composite(alpha, paint, limbs)


def cloak_core(alpha, grid, yaw):
    row = grid[0]
    shoulders = smooth((row-48.)/15.)
    cuffs = 1-smooth((row-128.)/16.)
    c = math.cos(math.radians(yaw))
    half = 39. - 5.*c*c
    limit = 90. - (90.-half)*shoulders*cuffs
    return np.minimum(alpha, np.clip(.5 + limit - np.abs(grid[1]-(102.+3.*c)), 0., 1.))


class KeeperTurn:
    """Keep the sleeves on the shoulder sockets instead of the cloak UVs."""
    def __init__(self, paintings, grid):
        self.grid = grid
        angles = list(range(0, 360, 45))
        trunk = []
        for yaw, image in zip(angles, paintings):
            body = image.copy()
            core = cloak_core(body[..., 3]/255., grid, yaw)
            body[..., 3] = np.rint(255*core).astype("u1")
            trunk.append(body)
        self.surface = stitch(trunk, angles, trunk, grid, keeper=True)
        first = [(65,48),(78,52),(83,63),(82,78),(75,94),(76,112),(75,129),
                 (70,141),(60,139),(54,128),(50,109),(52,80),(56,60)]
        self.sleeves = [
            piece(paintings[2], first, (68,58)),
            piece(paintings[2], [(203-x,y) for x,y in first], (135,58)),
        ]

    def frame(self, yaw, original_alpha):
        angle = math.radians(yaw)
        c, s = math.cos(angle), math.sin(angle)
        alpha = cloak_core(original_alpha, self.grid, yaw)
        paint = project(self.surface, self.grid, yaw, alpha)
        sleeves = []
        for sign, sleeve in zip((-1,1), self.sleeves):
            depth = -sign*c
            shifted = position(sleeve, self.grid, 102.+7.*c+sign*34.*s,
                               58.+8.*depth, .77+.23*s*s)
            sleeves.append((18.*depth, shifted))
        return composite(alpha, paint, sleeves)
