"""Traced joints and registration for the four additional jackal paintings."""

import math

import numpy as np
from PIL import Image

from jackal_skin_bake import HEIGHT, POSES as BASE_POSES, TEXELS, WIDTH, prepare


NEIGHBORS = {8: (0, 1), 9: (1, 2), 10: (6, 7), 11: (7, 0)}
ANGLES = {view: math.atan2(.52*math.sin(yaw), math.cos(yaw))
          for view, yaw in zip(NEIGHBORS, (math.pi/8, 3*math.pi/8, -3*math.pi/8, -math.pi/8))}

# Hip and stifle, hock, base of toes, tip. In front: scapula, shoulder,
# elbow, carpus, tip. Near and far legs retain the original anatomical order.
POSES = {
    8: ((150, 0, 700, 490), (
        ((286,205),(299,282),(237,354),(232,418),(253,452)),
        ((326,205),(352,274),(294,359),(313,413),(344,435)),
        ((469,174),(501,221),(484,287),(483,414),(510,452)),
        ((504,185),(540,231),(526,286),(521,398),(553,435)),
    ), ((499,190),(604,96),(660,132))),
    9: ((965, 0, 1276, 490), (
        ((1065,194),(1071,278),(1033,343),(1031,405),(1036,431)),
        ((1111,196),(1128,269),(1080,337),(1088,393),(1115,419)),
        ((1107,190),(1135,233),(1136,296),(1138,423),(1140,459)),
        ((1185,175),(1207,234),(1203,290),(1198,414),(1215,450)),
    ), ((1157,219),(1203,108),(1242,153))),
    10: ((978, 504, 1245, 1023), (
        ((1128,727),(1127,812),(1112,899),(1114,947),(1129,986)),
        ((1050,735),(1036,820),(1014,900),(1012,947),(1009,986)),
        ((1166,705),(1170,756),(1154,817),(1158,891),(1177,932)),
        ((1094,710),(1085,763),(1080,819),(1081,883),(1082,930)),
    ), ((1141,641),(1153,583),(1200,617))),
    11: ((203, 504, 654, 1023), (
        ((377,723),(359,824),(324,900),(337,950),(355,986)),
        ((305,751),(286,834),(253,911),(251,951),(258,978)),
        ((502,682),(527,747),(506,794),(502,899),(529,942)),
        ((429,697),(458,766),(448,817),(441,893),(451,935)),
    ), ((504,648),(572,584),(619,614))),
}

BELLY = {
    8: ((150,395),(198,405),(222,384),(247,343),(277,310),(306,285),(344,252),
        (370,257),(410,279),(456,289),(498,276),(542,263),(575,224),(700,224)),
    9: ((965,344),(1009,355),(1024,343),(1048,298),(1081,274),(1106,265),
        (1137,275),(1166,286),(1196,281),(1222,276),(1276,256)),
    10: ((978,813),(1022,818),(1043,833),(1054,907),(1075,949),(1091,919),(1105,842),
         (1134,830),(1154,807),(1183,803),(1245,789)),
    11: ((203,904),(241,934),(265,944),(287,909),(315,868),(343,825),(372,790),
         (403,777),(449,786),(491,782),(522,769),(552,753),(654,741)),
}

TAIL = {
    8: ((291,231), ((158,296),(206,286),(250,262),(298,237),(355,213),(420,209))),
    9: ((1051,237), ((179,1057),(224,1051),(277,1035),(332,1021),(365,1016))),
    11: ((327,755), ((662,336),(712,336),(757,326),(807,316),(866,282),(945,264))),
}


def bounds(image):
    box = image.getchannel("A").point(lambda a: 255 if a >= 110 else 0).getbbox()
    if not box:
        raise ValueError("Brak sylwetki w źródle widoku")
    return np.asarray(box)/TEXELS


def register(source, reference, painted, view):
    left, right = NEIGHBORS[view]
    cell, bones, landmarks, _ = prepare(source, POSES[view], left)
    old = []
    anchors = []
    for direction in (left, right):
        x, y = (direction%4)*WIDTH*TEXELS, (direction//4)*HEIGHT*TEXELS
        old.append(bounds(painted.crop((x, y, x+WIDTH*TEXELS, y+HEIGHT*TEXELS))))
        _, _, face, _ = prepare(reference, BASE_POSES[direction], direction)
        anchors.append(face[[0,2]].mean(axis=0))
    target = np.mean(old, axis=0)
    area = bounds(cell)
    # Calibrate the drawing once. All the exported poses then use this same size.
    scale = (target[2]-target[0])/(area[2]-area[0])
    x_origin = 80*(1-scale)
    mismatch = np.mean(anchors, axis=0)[0] - (landmarks[[0,2],0].mean()*scale+x_origin)
    shift = np.array((x_origin+np.clip(mismatch,-4,4),np.clip(target[1]-area[1],-2,2)))
    cell = cell.transform(cell.size, Image.Transform.AFFINE,
                          (1/scale,0,-shift[0]*TEXELS/scale,0,1,-shift[1]*TEXELS),
                          Image.Resampling.BICUBIC)
    return cell,bones*np.array((scale,1))+shift
