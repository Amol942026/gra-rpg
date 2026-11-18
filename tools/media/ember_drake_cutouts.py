"""Trace and backpaint the Ember Drake's five painted views for the six-part rig.

Coordinates are in the review plates' space; original full-resolution paint is
sampled before antialiased downsampling. Horizontal mirrors supply the remaining
three views. The empty pixels behind each joint are filled from that same part.
"""
from pathlib import Path
import argparse

import numpy as np
from PIL import Image, ImageChops, ImageDraw, ImageEnhance, ImageOps


CELL = 192
ROWS = ("far-wing", "diagonal-a", "diagonal-b", "armour-tail", "near-wing", "head")
ORDER = (0, 1, 2, 3, 4, 5)
REAR_ORDER = (0, 1, 2, 5, 3, 4)


def points(spec):
    return [tuple(map(float, q.split(","))) for q in spec.split()]


# Four limbs are traced separately before grouping into diagonal stance pairs.
# Outer boundaries extend into the original transparent margin. Internal lines
# follow the painted cut between the shoulder plates, patella and breast scales.
SE = {
    "far-wing": ["315,8 625,8 625,123 560,164 550,222 495,188 468,184 426,199 414,217 393,235 384,260 376,280 353,274 325,243 314,205"],
    "near-wing": ["0,0 443,0 444,128 371,178 342,230 348,267 364,279 369,312 337,334 321,345 298,351 290,321 249,314 220,320 148,369 134,336 0,395"],
    "head": ["410,185 448,195 454,146 547,141 620,160 649,256 649,415 548,415 516,358 496,323 482,328 469,370 452,382 438,361 432,330 425,310 411,292 399,283 392,269 412,273 402,252 416,254 409,235 427,237 418,215 438,217"],
    "front-near": ["308,411 326,427 344,430 354,418 366,403 379,395 390,402 389,430 393,452 403,474 407,501 455,539 478,598 345,616 337,533 323,497 318,462"],
    "front-far": ["428,432 451,427 476,414 491,395 504,395 516,422 536,458 540,496 604,532 622,580 486,581 480,524 458,494 440,473"],
    "hind-near": ["133,504 161,486 176,463 184,447 193,431 194,417 200,408 211,427 222,443 241,447 240,464 218,479 201,497 199,511 229,538 239,586 117,593"],
    "hind-far": ["256,472 277,458 285,449 298,429 307,433 319,452 327,473 323,493 356,517 365,561 247,565"],
}

SOUTH = {
    "far-wing": ["42,0 281,0 282,69 266,98 251,116 244,141 238,166 237,183 244,199 237,226 226,239 218,248 209,258 199,268 194,275 174,280 171,260 157,253 138,257 126,267 103,288 103,325 48,344"],
    "near-wing": ["320,0 530,0 530,345 472,341 447,306 418,278 402,264 390,251 369,240 356,233 350,215 351,198 359,174 362,146 355,117 343,94 329,83"],
    "head": ["225,90 278,82 300,96 322,85 350,85 366,113 353,161 367,169 355,175 363,188 351,192 360,204 348,200 348,222 337,222 340,237 326,232 316,248 302,270 296,275 279,253 271,233 263,236 260,222 249,231 250,207 238,213 247,193 236,196 245,181 233,181 238,166 225,146"],
    "front-near": ["194,333 207,324 216,315 224,301 234,290 249,287 255,305 263,322 265,344 262,350 273,382 260,409 267,442 289,481 283,517 180,518 192,442 202,417 204,383"],
    "front-far": ["335,326 340,304 352,290 365,303 378,316 391,327 399,338 390,364 388,385 388,415 398,444 415,465 418,523 316,523 314,458 329,422 326,397 329,368"],
    "hind-near": ["150,378 157,358 168,337 174,318 186,295 195,293 202,305 200,324 202,345 205,362 205,388 218,406 216,442 138,444 136,388"],
    "hind-far": ["384,306 398,295 413,304 427,329 428,350 440,377 457,398 464,446 386,446 382,413 391,386 387,368 385,350 391,328"],
}

NORTH = {
    "far-wing": ["566,0 787,0 790,48 773,71 754,104 756,134 763,157 762,181 753,198 745,212 737,235 716,248 701,265 691,278 679,285 660,310 574,338"],
    "near-wing": ["826,0 1034,0 1042,340 964,346 922,298 903,265 889,249 870,239 864,222 853,202 845,189 842,164 850,146 850,110 847,83 831,68"],
    "head": ["744,40 792,29 806,34 827,29 856,44 866,97 859,125 840,128 824,138 808,146 779,139 760,132 748,118"],
    "front-near": ["654,376 670,356 688,335 697,308 704,300 714,319 718,340 720,354 723,373 712,391 702,419 654,417"],
    "front-far": ["874,302 900,299 915,311 927,346 928,366 955,382 962,414 908,417 897,396 888,364 874,340"],
    "hind-near": ["690,325 701,309 714,302 728,293 746,288 758,304 761,335 770,359 766,392 750,448 684,455 682,418 710,388 701,356"],
    "hind-far": ["838,307 853,294 868,289 883,299 898,320 908,341 900,368 894,393 916,419 937,451 847,456 836,407 842,375 833,348"],
}

NE = {
    "far-wing": ["129,0 457,0 455,51 423,68 406,97 400,115 390,127 377,144 359,150 338,161 319,175 298,196 285,214 204,250 135,255"],
    "near-wing": ["397,139 423,110 448,79 466,51 501,50 511,67 505,94 508,115 518,130 530,145 538,151 542,164 529,160 515,159 501,167 490,184 485,199 469,212 446,232 419,254 401,259 398,230 399,207 396,193 401,173 391,159"],
    "head": ["491,6 592,3 616,57 615,117 581,128 558,159 545,163 533,150 519,137 510,125 507,109 503,91 502,78 505,58 493,46"],
    "front-near": ["495,191 504,179 514,166 525,167 538,179 549,202 558,216 580,219 583,258 531,259 522,237 507,227"],
    "front-far": ["417,244 431,229 447,216 455,209 462,225 459,247 465,260 488,268 492,292 421,294 417,268"],
    "hind-near": ["272,339 284,326 288,310 294,294 309,289 322,293 332,302 336,314 318,333 309,350 304,365 318,370 347,377 359,419 258,424 259,385"],
    "hind-far": ["369,272 374,249 389,242 400,245 409,255 412,268 406,287 418,295 442,302 455,342 377,344 365,307"],
}

PROFILE_BOX = (2, 0, 648, 322)
PROFILE_SCALE = (.18,.235)
PROFILE_ANCHOR = (330,305)
PROFILE = {
    # In true profile the second membrane is mostly occluded. Its separate
    # rear silhouette is recovered from the same far-wing painting used in SE.
    "far-wing": ["160,108 231,105 297,94 356,87 394,81 419,67 418,87 422,115 439,135 449,149 438,153 404,157 350,160 294,169 230,154 172,184"],
    "near-wing": ["142,0 497,0 502,56 433,76 418,102 414,114 428,133 439,140 451,149 452,167 429,172 406,160 390,165 371,171 353,178 340,159 325,153 298,151 282,184 274,161 245,152 219,167 188,182 149,207"],
    "head": ["429,108 453,98 472,96 469,62 512,55 568,80 622,108 647,134 648,180 592,188 548,165 533,170 521,182 507,190 498,206 490,200 481,187 473,172 465,161 450,151 431,147 448,139 437,133 452,128 443,122 461,115"],
    "front-near": ["367,215 380,226 391,228 405,217 418,207 429,209 429,232 422,248 425,263 434,282 461,293 469,323 381,323 376,285 379,250"],
    "front-far": ["433,237 455,232 471,224 482,214 486,224 482,243 476,258 485,271 497,283 530,288 536,314 456,316 446,293 438,269"],
    "hind-near": ["159,290 177,258 188,246 211,230 227,216 237,215 248,225 246,240 236,253 214,261 205,271 210,285 235,294 248,317 158,322"],
    "hind-far": ["236,250 257,247 279,229 294,224 301,233 292,249 277,258 275,275 292,282 317,287 326,309 240,313"],
}

# Ellipses cover the hidden part of a joint. Backpainting all the way around a
# horn or a detached tail point would make the surrounding wing move with it.
JOINTS = {
    "SE": (((421,258),(365,305)), ((446,350),), ((367,417),(472,417),(212,444),(298,459))),
    "E": (((392,154),(435,159)), ((474,167),), ((414,222),(464,228),(222,232),(274,235))),
    "S": (((228,243),(366,243)), ((298,247),), ((228,326),(367,326),(186,338),(410,338))),
    "N": (((735,238),(869,240)), ((804,125),), ((735,326),(873,327),(704,334),(899,339))),
    "NE": (((393,135),(432,215)), ((543,140),), ((519,185),(447,226),(308,302),(394,260))),
}

# The far membrane is visible between the two horns and above the serrated
# neck. These contours follow its actual rim and the horn tip's concave edge.
SE_REVEALED_WING = [
    "326,214 345,181 380,149 421,120 478,106 533,83 578,76 608,81 615,123 "
    "581,147 554,163 536,188 529,203 519,199 511,198 498,191 488,180 482,172 "
    "480,157 476,166 474,175 475,188 464,185 471,195 455,195 443,197 418,195 "
    "429,203 436,209 443,212 424,217 403,226 395,232 416,230 409,237 392,248 "
    "385,255 402,251 396,259 380,270 376,278 362,270 352,266 344,256 "
    "349,242 351,224 350,211 344,204 338,205",
    # Tip and curved outside of the broad far wing, surrounded by transparent sky.
    "540,84 590,70 615,76 611,119 595,120 583,118 568,125 553,134 538,141",
]

SOUTH_WING_EDGE = (
    "40,1 274,1 274,78 267,91 259,98 251,108 245,121 241,131 241,137 "
    "228,140 233,144 241,150 236,155 227,172 241,168 237,176 230,187 "
    "232,194 241,188 244,186 240,201 239,205 246,201 245,215 242,226 "
    "237,231 225,236 215,244 199,259 185,271 175,282 168,275 157,250 "
    "140,250 117,263 102,300 92,330 45,341"
)


def rejoin_paint_chips(labels, alpha):
    """Return clipped slivers to the connected anatomical piece they touch."""
    owners=np.array(labels)
    solid=np.asarray(alpha)>65
    for row in (0,1,2,3,4,5):
        surface=(owners==row)&solid
        # Connected horizontal runs keep this offline repair independent from
        # computer-vision packages and preserve detached pigmented antialiasing.
        parents=[]
        spans=[]
        prior=[]
        def root(i):
            while parents[i]!=i:
                parents[i]=parents[parents[i]]
                i=parents[i]
            return i
        for y,scan in enumerate(surface):
            edges=np.flatnonzero(np.diff(scan,prepend=False,append=False))
            current=[]
            for left,right in edges.reshape(-1,2):
                n=len(parents);parents.append(n)
                for a,b,old in prior:
                    if b<left:continue
                    if a>right:break
                    parents[root(old)]=root(n)
                spans.append((y,left,right,n))
                current.append((left,right,n))
            prior=current
        groups={}
        for y,left,right,n in spans:
            groups.setdefault(root(n),[]).append((y,left,right))
        for group in groups.values():
            if sum(b-a for _,a,b in group)>2400:continue
            votes=np.zeros(6,dtype=int)
            for y,left,right in group:
                for yy in range(max(0,y-2),min(owners.shape[0],y+3)):
                    s=slice(max(0,left-2),min(owners.shape[1],right+2))
                    vals=owners[yy,s][solid[yy,s]]
                    votes+=np.bincount(vals,minlength=6)
            votes[row]=0
            if votes.max()<4:continue
            owner=int(votes.argmax())
            for y,left,right in group:
                s=slice(max(0,left-1),min(owners.shape[1],right+1))
                for yy in range(max(0,y-1),min(owners.shape[0],y+2)):
                    section=owners[yy,s]
                    section[section==row]=owner
    return Image.fromarray(owners)


def extend_joint(tile, original_alpha, amount, joint_mask):
    # Propagate authentic pigment behind the joint without dilating its outside edge.
    pixels = np.array(tile)
    allowed = np.asarray(original_alpha)
    for distance in (3, 2) + (1,) * max(0, amount-5):
        free = (pixels[..., 3] == 0) & (allowed > 20) & joint_mask
        old = pixels.copy()
        for dx, dy in ((0,-distance), (-distance,0), (distance,0), (0,distance)):
            neighbour = np.roll(old, (dy, dx), axis=(0,1))
            use = free & (neighbour[..., 3] > 100)
            pixels[use] = neighbour[use]
            free[use] = False
    pixels[..., 3] = np.minimum(pixels[..., 3], allowed)
    return Image.fromarray(pixels)


def parts(source, width, box, forms, view):
    unit = source.width / width
    zoom = 2
    origin = (box[0], box[1])
    crop = tuple(round(v*unit) for v in box)
    paint = source.crop(crop).resize((round((box[2]-box[0])*zoom),round((box[3]-box[1])*zoom)), Image.Resampling.LANCZOS)
    # Source sheets have stray near-transparent pigment around some tips.
    alpha = paint.getchannel("A").point(lambda a: 0 if a < 10 else a)
    paint.putalpha(alpha)
    dims = paint.size
    labels = Image.new("L", dims, 3)
    masks = {}
    for key, contours in forms.items():
        local = Image.new("L", dims)
        brush=ImageDraw.Draw(local)
        for contour in contours:
            brush.polygon([((x-origin[0])*zoom,(y-origin[1])*zoom) for x,y in points(contour)],255)
        masks[key] = local
    for key, row in (("far-wing",0),("near-wing",4),("head",5),("hind-far",2),("front-far",1),("hind-near",1),("front-near",2)):
        labels.paste(row, (0,0), masks[key])
    final=ImageDraw.Draw(labels)
    def relabel(poly,row):
        final.polygon([((x-origin[0])*zoom,(y-origin[1])*zoom) for x,y in poly],row)
    if view=="SE":
        for contour in SE_REVEALED_WING:
            relabel(points(contour),0)
        # The left membrane ends in a downward hook; the isolated point to
        # its right belongs to the curled tail below, even in a high wingbeat.
        relabel(points("28,375 52,353 113,334 136,339 135,405 25,413"),3)
        values=np.array(labels)
        yy,xx=np.ogrid[:dims[1],:dims[0]]
        shoulder_plate=(xx>(382-origin[0])*zoom)&(yy<(277-origin[1])*zoom)
        values[(values==3)&shoulder_plate]=5
        labels=Image.fromarray(values)
    if view=="S":
        left_edge=points(SOUTH_WING_EDGE)
        relabel(left_edge,0)
        relabel([(597-x,y) for x,y in left_edge],4)
    if view=="N":
        relabel(points("565,0 786,0 785,52 779,54 775,44 770,44 762,49 753,57 746,73 741,99 738,115 699,121 601,127"),0)
        relabel(points("823,0 1042,0 1014,130 873,130 866,102 864,82 858,59 850,49 844,46 836,44 831,49 825,51"),4)
        relabel(points("907,450 928,443 973,439 977,472 945,503 899,513"),3)
    labels=rejoin_paint_chips(labels,alpha)
    wings,skull,legs=JOINTS[view]
    result=[]
    for row in range(6):
        mask=labels.point(lambda v:255 if v==row else 0)
        tile=paint.copy()
        tile.putalpha(ImageChops.multiply(mask,alpha))
        frontier=Image.new("L",dims)
        pen=ImageDraw.Draw(frontier)
        sites=(wings[row//4],) if row in (0,4) else skull if row==5 else legs if row in (1,2) else wings+skull+legs
        for x,y in sites:
            bx,by=(x-origin[0])*zoom,(y-origin[1])*zoom
            radius=(58 if row in (3,5) else 49)*zoom
            pen.ellipse((bx-radius,by-radius,bx+radius,by+radius),255)
        result.append(extend_joint(tile,alpha,round((11 if row in (0,3,4,5) else 13)*zoom),np.asarray(frontier)>0))
    if view=="NE":
        # The near membrane hides the chest between forequarters and spine in
        # the back-quarter reference. Continue the real row of overlapping
        # scutes under that hinge so it can unfold above a solid rib cage.
        mask=Image.new("L",dims)
        brush=ImageDraw.Draw(mask)
        bridge=points("357,192 381,154 416,133 449,127 477,125 502,129 520,148 "
                      "516,174 490,191 460,208 425,231 387,250 365,252")
        brush.polygon([((x-origin[0])*zoom,(y-origin[1])*zoom) for x,y in bridge],255)
        donor=paint.transform(dims,Image.Transform.AFFINE,(1,0,-85*zoom,0,1,34*zoom),Image.Resampling.BICUBIC)
        donor.putalpha(ImageChops.multiply(ImageChops.multiply(mask,alpha),donor.getchannel("A")))
        donor=ImageEnhance.Brightness(donor).enhance(.78)
        donor.alpha_composite(result[3])
        result[3]=donor
    return result


def register(pieces, box, scale, anchor, origin, flip=False):
    # All rest feet are placed by the same model-space ground origin, independent
    # of transparent padding and the lower curled tip in the rear-facing sheet.
    result=[]
    for tile in pieces:
        sx,sy=scale if isinstance(scale,tuple) else (scale,scale)
        dims=(round((box[2]-box[0])*sx*CELL/128),round((box[3]-box[1])*sy*CELL/128))
        clean=tile.resize(dims,Image.Resampling.LANCZOS)
        frame=Image.new("RGBA",(CELL,CELL))
        pos=(round((origin[0]+(box[0]-anchor[0])*sx)*CELL/128),
             round((origin[1]+(box[1]-anchor[1])*sy)*CELL/128))
        frame.alpha_composite(clean,pos)
        result.append(ImageOps.mirror(frame) if flip else frame)
    return result


def main(root):
    sources = {n: Image.open(root / name).convert("RGBA") for n,name in {
        "SE":"sunburst-concept.png","turn":"sunburst-turnaround.png",
        "NE":"sunburst-rear-right.png","E":"sunburst-profile.png"}.items()}
    # Width, crop in review coordinates, tracer, nominal units/pixel,
    # painted ground reference, destination ground reference.
    specs={
        "SE":("SE",650,(0,0,650,650),SE,.177,(320,585),(62,114)),
        "S":("turn",1100,(40,1,541,538),SOUTH,.185,(298,494),(64,114)),
        "N":("turn",1100,(566,1,1046,545),NORTH,.185,(804,442),(64,107)),
        "NE":("NE",650,(12,1,620,430),NE,.200,(325,397),(62,113)),
        "E":("E",650,PROFILE_BOX,PROFILE,PROFILE_SCALE,PROFILE_ANCHOR,(64,114)),
    }
    target=root/"cutouts"
    target.mkdir(parents=True,exist_ok=True)
    views={}
    for label,(key,width,box,forms,scale,anchor,origin) in specs.items():
        raw=parts(sources[key],width,box,forms,label)
        views[label]=register(raw,box,scale,anchor,origin)
    # The second wing is occluded in the profile painting. Restore the membrane
    # behind its visible root so a wingbeat can uncover it without a cut edge.
    wing=views["E"][4].transform((CELL,CELL),Image.Transform.AFFINE,
        (1/.92,0,-2.5,0,1/.89,-6),Image.Resampling.BICUBIC)
    backwing=ImageEnhance.Brightness(wing).enhance(.79)
    backwing.alpha_composite(views["E"][0])
    views["E"][0]=backwing
    directions=(("E",0),("SE",0),("S",0),("SE",1),("E",1),("NE",1),("N",0),("NE",0))
    layers=[Image.new("RGBA",(CELL*8,CELL*6)) for _ in ROWS]
    gallery=Image.new("RGBA",(CELL*4,CELL*2),(37,42,48,255))
    for i,(view,mirror) in enumerate(directions):
        composed=Image.new("RGBA",(CELL,CELL))
        for row in (REAR_ORDER if i>=5 else ORDER):
            frame=views[view][row]
            if mirror: frame=ImageOps.mirror(frame)
            layers[row].paste(frame,(i*CELL,row*CELL))
            composed.alpha_composite(frame)
        gallery.alpha_composite(composed,((i%4)*CELL,(i//4)*CELL))
    for layer,row in zip(layers,ROWS):layer.save(target/(row+".png"))
    gallery.convert("RGB").save(target/"review.png")


if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("root",type=Path)
    main(parser.parse_args().root)
