"""Cut the original directional paintings into the warrior's six registered Synfig layers."""
import argparse
import json
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageOps
from scipy import ndimage

from caldera_corpse_paint import cell_paint
from ember_drake_synfig import NAMES


CELL = 240
PARTS = ("tabard", "greave-a", "greave-b", "cuirass-and-helm", "shield-arm", "sword-arm")
# Landmarks are painted in 256-unit plates. Parts are traced at original resolution.
# Masks overlap at the shoulder and skirt so the layers can actually separate.
KEY = {
    "E": ((0,0), (97,233), {
        0:"63,118 81,115 101,135 119,129 139,119 140,162 130,194 114,174 83,192 65,179",
        1:"67,160 91,160 100,172 100,202 101,228 111,240 132,244 146,252 57,252",
        2:"98,161 125,157 130,182 129,212 141,227 164,234 161,247 99,244 88,219",
        4:"115,71 139,59 162,55 181,66 177,138 159,166 145,163 126,142 122,111",
        5:"52,47 91,40 111,47 128,59 130,78 121,90 117,108 115,127 118,136 112,141 129,145 136,133 147,139 146,150 219,176 234,198 194,194 131,167 121,184 111,177 111,168 91,170 82,162 81,154 75,151 75,129 73,119 75,108 69,99 66,84 56,79"},
        ((94,133),(84,158),(113,157),(100,128),(132,77),(92,65))),
    "SE": ((1,0),(126,230),{
        0:"88,110 130,118 161,108 181,173 155,177 135,164 125,178 111,169 91,174 77,156",
        1:"86,154 110,149 127,165 112,188 100,208 101,244 97,254 61,254 67,214 71,201 74,184 74,176",
        2:"129,152 150,145 169,149 183,179 179,187 174,193 175,211 202,218 201,235 160,238 140,223 128,211 131,192",
        4:"150,44 171,41 182,47 194,59 200,62 218,65 221,95 217,137 199,192 188,202 181,177 164,152 159,127 159,89 169,75 161,70 150,68",
        5:"56,59 74,43 92,37 111,43 118,53 123,65 111,77 100,82 91,93 80,98 81,109 81,121 71,135 65,143 70,155 63,168 53,164 48,159 8,224 0,232 0,197 36,152 42,145 37,131 44,123 52,127 54,107 52,97 56,84 62,79"},
        ((125,135),(105,159),(149,156),(127,131),(170,66),(90,69))),
    "S": ((2,0),(134,229),{
        0:"87,114 113,120 133,117 153,119 174,112 187,162 174,171 160,162 145,170 136,180 125,171 106,168 87,174 80,166",
        1:"83,157 104,151 124,156 126,172 114,195 110,208 114,235 115,251 62,252 63,228 70,219 74,196 74,174",
        2:"143,156 164,149 181,154 186,174 192,198 194,215 208,232 212,251 155,251 148,229 149,205 141,188",
        4:"156,46 173,39 189,45 201,53 205,64 221,69 230,91 234,118 228,159 216,194 207,187 184,167 166,138 156,101 158,86 166,76",
        5:"56,50 91,34 109,36 119,48 117,65 103,74 100,80 91,85 86,97 83,106 83,118 74,129 69,140 72,154 63,160 56,157 21,223 4,235 13,201 44,150 43,143 45,130 51,126 54,128 55,112 54,96 59,82 54,77",
        },((133,133),(108,157),(167,157),(134,130),(180,64),(87,63))),
    "NE": ((0,1),(109,235),{
        0:"75,111 91,117 117,120 146,115 157,162 141,185 126,176 115,192 102,178 83,176 64,191 65,151",
        1:"62,156 91,156 105,164 89,190 81,214 82,231 72,236 38,234 44,215 48,202 50,185",
        2:"105,158 132,150 146,160 148,178 155,206 155,225 170,235 165,249 105,254 100,237 103,222 111,206 98,182",
        4:"28,52 61,40 79,35 94,43 99,63 90,74 84,83 79,98 79,134 62,161 45,174 34,163 25,135 21,95",
        5:"108,42 128,39 147,44 164,56 170,71 168,85 156,89 155,98 166,119 171,131 177,138 184,136 190,139 193,152 245,171 256,180 256,188 239,191 185,167 179,170 174,164 169,157 156,157 146,150 144,137 134,126 131,110 126,98 117,90 107,81 107,69"},
        ((107,128),(77,156),(123,156),(107,131),(64,59),(139,66))),
    "N": ((1,1),(122,226),{
        0:"87,111 111,117 134,117 152,113 176,148 183,166 162,171 143,162 133,176 116,172 99,166 75,177 70,158",
        1:"78,156 99,148 117,159 116,182 109,204 108,223 110,240 59,247 55,234 69,213 65,190",
        2:"128,157 152,150 169,153 174,176 173,191 178,214 190,228 190,247 145,246 138,231 141,207 133,191",
        4:"22,50 46,40 68,34 80,36 86,49 81,62 76,70 75,83 69,102 64,117 54,157 39,198 23,186 18,152 13,108",
        5:"153,40 170,39 189,49 205,64 205,77 198,84 189,84 191,94 196,102 195,122 202,130 208,128 215,134 218,147 255,185 256,205 244,202 207,168 195,171 185,162 183,149 171,139 177,131 171,116 168,107 162,101 164,86 154,77 146,65"},
        ((122,134),(96,155),(148,155),(122,131),(62,62),(183,67))),
}

WEST = {
    "SW": ((0,0),(134,234),{
        0:"93,98 123,108 145,111 162,106 176,140 161,154 145,144 136,159 125,144 112,154 93,146 78,158",
        1:"78,143 98,143 111,150 118,162 109,180 104,199 105,218 95,230 57,238 47,226 68,210 76,179",
        2:"125,146 147,139 168,145 180,171 181,185 188,215 202,241 195,250 153,251 147,230 145,207 140,195 125,172",
        4:"153,44 178,33 196,36 209,49 216,67 213,78 229,100 227,137 213,169 204,191 185,181 157,151 146,122 145,96 151,80 164,67",
        5:"84,67 103,50 119,43 125,45 125,55 122,63 122,68 117,79 112,82 107,89 103,99 101,106 94,112 88,118 91,122 88,136 78,141 71,136 21,204 7,218 12,193 60,132 57,125 61,117 68,114 73,117 77,106 83,102 88,94 91,84 89,78"},
        ((133,117),(99,152),(150,153),(135,121),(183,62),(108,65))),
    "W": ((1,0),(129,234),{
        0:"90,112 117,116 139,114 153,103 170,139 157,165 141,159 124,179 106,163 88,166",
        1:"89,157 112,151 130,160 135,178 126,210 120,226 103,239 70,241 71,229 86,216 92,195",
        2:"118,161 141,153 163,157 160,177 160,201 153,225 144,244 107,247 99,240 109,225 117,217 115,191",
        5:"77,62 91,56 100,60 112,72 106,83 94,96 91,106 88,119 87,126 91,139 86,149 77,145 20,162 0,165 0,153 67,123 69,116 71,112 75,115 77,98 72,85",
        4:"99,47 121,31 139,28 159,36 171,53 173,71 171,79 171,95 174,117 171,142 157,170 134,197 116,182 98,160 83,135 80,113 86,94 94,85 96,76 85,70"},
        ((124,124),(111,165),(140,164),(129,120),(136,61),(98,71))),
    "NW": ((2,0),(101,232),{
        0:"73,101 96,110 124,111 145,106 167,139 157,154 143,150 130,162 115,150 104,166 89,150 66,162",
        1:"59,148 86,146 103,157 107,178 95,208 97,227 84,243 37,245 41,230 62,217 67,193",
        2:"116,148 143,141 164,148 167,173 165,193 171,210 170,225 139,234 122,226 124,202 114,186",
        5:"126,43 147,36 166,42 177,51 184,67 180,80 172,82 183,90 190,105 190,116 198,120 205,117 211,128 212,137 248,167 256,180 236,174 200,150 191,151 180,142 174,134 167,125 171,120 159,105 153,88 141,81 130,67",
        4:"35,59 52,38 79,29 95,33 101,46 99,59 93,72 82,81 85,94 85,117 80,143 63,163 44,190 32,175 20,148 15,123 12,100 17,87 28,79"},
        ((108,122),(83,158),(148,157),(108,124),(68,59),(157,63))),
}

# Extend the outside of each trace into the transparent surround. This keeps
# the complete sword edge and toe with their owners, including antialiasing.
WEAPON_BOUNDARIES = {
 "E": "0,0 56,34 84,39 105,42 121,51 128,63 128,77 120,84 118,99 117,113 114,127 117,140 128,146 137,130 160,133 256,152 256,214 143,172 132,166 120,184 109,179 108,168 91,171 75,163 63,143 55,116 0,114",
 "SE":"0,30 71,32 97,37 109,42 120,56 117,69 108,76 96,84 88,87 85,103 77,114 73,130 66,140 71,153 63,168 53,164 46,158 0,241 0,190 40,144 39,129 51,120 53,105 53,96 57,82",
 "S":"0,35 79,31 104,31 110,41 119,48 117,64 107,71 101,79 91,85 88,96 81,109 77,124 70,133 69,141 74,151 66,164 56,159 0,249 0,191 42,145 42,133 51,125 55,125 55,107 55,94 62,79 51,70",
 "SW":"0,24 99,23 112,43 123,51 125,63 123,76 119,86 120,96 115,103 109,116 105,124 101,136 97,140 101,149 92,154 84,145 0,232 0,141 75,120 87,113 88,96 91,86 87,78",
 "NE":"105,43 124,36 155,31 256,15 256,204 181,171 175,166 166,159 153,159 144,150 143,137 135,129 129,108 126,99 119,92 109,81 106,69",
 "N":"152,39 166,37 199,32 256,16 256,221 204,173 195,174 183,163 181,151 171,140 174,130 170,117 168,109 161,101 164,87 154,78 146,65",
 "NW":"132,42 156,33 181,42 238,34 256,217 198,151 190,153 177,146 171,136 165,132 160,122 164,117 156,103 153,90 145,82 133,70",
}
for _direction,_poly in WEAPON_BOUNDARIES.items():
    (WEST if _direction in WEST else KEY)[_direction][2][5]=_poly


def clean_source(plate):
    rgba=np.asarray(plate.convert("RGBA")).copy()
    labels,n=ndimage.label(rgba[:,:,3]>28)
    counts=np.bincount(labels.ravel()); counts[0]=0
    body=labels==counts.argmax()
    # Preserve fine edges connected to the character; remove only isolated sheet debris.
    rgba[:,:,3]=np.where(ndimage.binary_dilation(body,iterations=3),rgba[:,:,3],0)
    rgb=rgba[:,:,:3].astype(float)/255
    # Lift the reflected skylight on the dark glass to match the Caldera's
    # battle exposure. Deep cuts and amber seams retain their local contrast.
    rgba[:,:,:3]=np.uint8(np.clip(255*(rgb**.91),0,255))
    return rgba


def split(plate, polygons, name):
    arr=clean_source(plate)
    w=arr.shape[1]
    labels=np.zeros(arr.shape[:2],dtype=np.uint8)+3
    for p,coords in polygons.items():
        vertices=[tuple(float(x)*w/256 for x in pair.split(",")) for pair in coords.split()]
        area=Image.new("L",(w,w));ImageDraw.Draw(area).polygon(vertices,fill=255)
        labels[np.array(area)>0]=p
    alpha=arr[:,:,3]
    # The exposed section of each blade must remain on the weapon arm when it
    # swings away. The grip and tip are traced separately in all eight paintings.
    grips={"E":(113,155,216,187),"SE":(53,143,3,213),"S":(60,141,12,215),
           "SW":(86,133,23,199),"W":(81,135,17,154),"NW":(184,132,232,163),
           "N":(205,148,252,195),"NE":(177,149,245,182)}
    gx,gy,tx,ty=grips[name]
    blade=Image.new("L",(w,w))
    ImageDraw.Draw(blade).line((gx*w/256,gy*w/256,tx*w/256,ty*w/256),fill=255,width=round(w*.073))
    labels[np.array(blade)>0]=5
    # Untraced fringes below the tassets belong to the closest greave,
    # blade or shield. Backpainting a disconnected boot on the chest would
    # show it standing still next to that same boot during the stride.
    y=np.indices(labels.shape)[0]*256/w
    loose=(labels==3)&(alpha>4)&(y>157)
    neighbors=(labels!=3)&(alpha>4)
    _,nearest=ndimage.distance_transform_edt(~neighbors,return_indices=True)
    labels[loose]=labels[tuple(nearest)][loose]
    for p in (3,1,2,4,5,0):
        islands,count=ndimage.label((labels==p)&(alpha>24))
        populations=np.bincount(islands.ravel());populations[0]=0
        for idx in range(1,count+1):
            if populations[idx]>max(65,.08*populations.max()):continue
            chip=islands==idx
            rim=ndimage.binary_dilation(chip,iterations=2)
            adjacent=labels[rim & (alpha>16) & (labels!=p)]
            if adjacent.size:
                labels[rim & (labels==p)]=np.bincount(adjacent).argmax()
    pieces=[]
    for p in range(6):
        own=(labels==p)&(alpha>4)
        if own.sum()<30:
            pieces.append(Image.new("RGBA",plate.size));continue
        distance,indices=ndimage.distance_transform_edt(~own,return_indices=True)
        # Backpaint concealed paint at the seams. Each layer retains the source
        # silhouette. The adjoining skin extends behind it when a joint flexes.
        near=(distance<(.027*w if p in (1,2,3) else .012*w)) & (alpha>4)
        out=arr[tuple(indices)].copy();out[:,:,3]=np.where(near,alpha,0)
        out[own]=arr[own]
        pieces.append(Image.fromarray(out,"RGBA"))
    return pieces


def register(parts,foot,joints):
    alpha=Image.new("L",parts[0].size)
    for p in parts:alpha=Image.fromarray(np.maximum(np.asarray(alpha),np.asarray(p.getchannel("A"))))
    b=alpha.point(lambda x:255 if x>36 else 0).getbbox()
    ratio=88/(b[3]-b[1])
    rect=(64-foot[0]*2*ratio,116-b[3]*ratio)
    scale=CELL/128
    def point(v):return [(rect[k]+v[k]*2*ratio) for k in range(2)]
    tiles=[]
    for part in parts:
        affine=(1/(ratio*scale),0,-rect[0]/ratio,0,1/(ratio*scale),-rect[1]/ratio)
        tiles.append(part.transform((CELL,CELL),Image.Transform.AFFINE,affine,Image.Resampling.BICUBIC))
    return tiles,[point(j) for j in joints]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source",type=Path)
    parser.add_argument("out",type=Path)
    args=parser.parse_args()
    base=Image.open(args.source).convert("RGBA"); views={}; pivots={}
    west=Image.open(args.source.with_name("sunburst-west.png")).convert("RGBA")
    for image,size,library in ((base,512,KEY),(west,724,WEST)):
        for name,((column,row),foot,masks,joints) in library.items():
            raw=image.crop((column*size,row*size,(column+1)*size,(row+1)*size))
            plate=raw.resize((512,512),Image.Resampling.LANCZOS) if size!=512 else raw
            views[name],pivots[name]=register(split(plate,masks,name),foot,joints)
    # In the west profile the shield covers the sword hand, skirt and shin.
    # Reconstruct those complete joints behind the shield so that its swing
    # cannot uncover disconnected paint from the original occluded drawing.
    def fit(im,source,target,sx,sy):
        x,y=source;u,v=target
        return im.transform((CELL,CELL),Image.Transform.AFFINE,
                            (1/sx,0,x-u/sx,0,1/sy,y-v/sy),Image.Resampling.BICUBIC)
    views["W"][0]=fit(views["S"][0],(122,151),(119,147),.56,.9)
    views["W"][1]=fit(views["S"][1],(91,203),(124,201),.76,.92)
    chest=fit(views["S"][3],(125,124),(117,115),.65,.93)
    # The face is painted in profile and stays entirely in the west view.
    chest.paste((0,0,0,0),(0,0,CELL,94))
    helmet=views["W"][3].crop((0,0,CELL,97))
    chest.alpha_composite(helmet,(0,0))
    views["W"][3]=chest
    # Opposite profile of the same obsidian blade with a complete gauntlet,
    # tapered in perspective; the far arm stays behind the pavise.
    views["W"][5]=fit(ImageOps.mirror(views["E"][5]),(120,150),(98,139),.72,.76)
    args.out.mkdir(exist_ok=True,parents=True)
    for name in views:
        for label,part in zip(PARTS,views[name],strict=True):part.save(args.out/f"{name}-{label}.png")
    (args.out/"pivots.json").write_text(json.dumps(pivots,indent=2))

if __name__=="__main__":main()
