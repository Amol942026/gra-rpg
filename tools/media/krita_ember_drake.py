"""Finish the traced directional paintings with DreamShaper materials in Krita."""
from pathlib import Path

from PyQt5.QtGui import QColor, QImage
from krita_abyss_chests import Master


CELL = 192
PARTS = (
    ("far-wing", "01 Skrzydło dalsze"),
    ("diagonal-a", "02 Łapy po przekątnej A"),
    ("diagonal-b", "03 Łapy po przekątnej B"),
    ("armour-tail", "04 Korpus, zbroja i ogon"),
    ("near-wing", "05 Skrzydło bliższe"),
    ("head", "06 Głowa, rogi i szyja"),
)
# Boxes lie inside DreamShaper's painted surface, never reaching the backdrop.
SWATCHES = {
    "membrane": (408,150,75,35), "patina": (340,209,43,19),
    "basalt": (356,279,54,28), "scute": (442,317,30,27),
}


def swatch_table(paint, bounds):
    x0,y0,w,h=bounds
    values=[[paint.pixelColor(x0+x,y0+y) for x in range(w)] for y in range(h)]
    mean=sum(.24*v.red()+.60*v.green()+.16*v.blue() for row in values for v in row)/(w*h)
    return [[(.24*v.red()+.60*v.green()+.16*v.blue()-mean, v.red(),v.green(),v.blue())
             for v in row] for row in values]


def reflected(x,width):
    v=int(x)%(2*width-2)
    return v if v<width else 2*width-2-v


def finish_row(image,row,tables):
    # Identify copper, horn and shadow from the original brushwork. Preserve
    # fine specular rims. Reflected sampling keeps the glaze continuous when
    # the camera switches between the hand-traced pose and its mirrored partner.
    row_y=row*CELL
    for direction in range(8):
        start=direction*CELL
        for y in range(CELL):
            for x in range(CELL):
                colour=image.pixelColor(start+x,row_y+y)
                if not colour.alpha():continue
                r,g,b=colour.red(),colour.green(),colour.blue()
                warm=max(0,min(1,(r-b-14)/62))
                light=(.25*r+.60*g+.15*b)/255
                rim=max(0,min(1,(light-.55)/.35))
                wing=row in (0,4)
                blend=warm if wing else warm*.65
                dark_skin=not wing and b>g*.85 and r<168
                material=("patina" if row==0 else "membrane") if wing and warm>.30 else (
                    "basalt" if row in (3,5) else "scute")
                table=tables[material]
                u=CELL-1-x if direction in (3,4,5) else x
                # Anisotropic glaze follows the long membrane ribs. Slate
                # scutes pick up finer, broken striations than the membranes.
                uu=reflected(u*(1.50 if wing else 1.18)+.36*y,len(table[0]))
                vv=reflected(y*(.73 if wing else 1.24)-.19*u,len(table))
                detail,mr,mg,mb=table[vv][uu]
                pigment=(.11 if wing else .085)*(1-rim)
                # Keep the eyelids, split lip and horn ridges legible at game
                # scale. Scaled skin accepts more grain than the smooth horn.
                grain=.36 if wing else .27 if row!=5 else .13
                if not wing and light>.56:
                    grain*=.48
                texture=detail*grain*(1-.76*rim)
                cool=(1-warm)*(1-rim)*(1.2 if dark_skin else .72)
                rgb=(r*(1-pigment)+mr*pigment+texture-5*cool+1.5*blend,
                     g*(1-pigment)+mg*pigment+texture+3.5*cool,
                     b*(1-pigment)+mb*pigment+texture+13*cool-1.4*blend)
                image.setPixelColor(start+x,row_y+y,QColor(*[max(0,min(255,round(v))) for v in rgb],colour.alpha()))


def produce(output, retained):
    output=Path(output)
    output.mkdir(parents=True,exist_ok=False)
    source=output.parent
    local=QImage(str(source/"comfy/drake-paint.png"))
    assert (local.width(),local.height())==(640,512)
    tables={name:swatch_table(local,box) for name,box in SWATCHES.items()}
    doc=Master(output,"zarowy-drak",CELL*8,CELL*6,retained)
    for row,(key,label) in enumerate(PARTS):
        painting=QImage(str(source/"cutouts"/(key+".png"))).convertToFormat(QImage.Format_ARGB32)
        if painting.size().width()!=CELL*8 or painting.size().height()!=CELL*6:
            raise ValueError("Invalid cutout for "+key)
        finish_row(painting,row,tables)
        doc.layer(label,lambda p,art=painting:p.drawImage(0,0,art))
    return doc.save()


def run(args):
    if len(args)>1 and args[1]=="jaw":
        output=Path(args[0])
        output.mkdir(parents=True,exist_ok=False)
        master=Master(output,"ember-drake-mouth",480,768,[])
        for name,label in (("throat-teeth","Gardziel i kły"),("upper-head","Górny pysk"),("mandible","Żuchwa")):
            image=QImage(str(output.parent/"jaw-layers"/(name+".png")))
            assert (image.width(),image.height())==(480,768)
            master.layer(label,lambda p,layer=image:p.drawImage(0,0,layer))
        master.save()
    else:
        produce(Path(args[0]),[])
