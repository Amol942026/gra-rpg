"""Sew a single permanent 2D painting for each piece of the Ash Wraith's robe."""
import re
from pathlib import Path

import cv2
import numpy as np
from PIL import Image

SIZE = 192
WIDTH = 768
HEIGHT = 224
HEM = 16
# Centers of the cardinal images, in clockwise screen coordinates.
CARDINAL = (0, 2, 4, 6)
REFERENCE = Path("art/volcano/ash-wraith/redesign/references/sunburst-turnaround.png")


def remap(image, x, y, interpolation=cv2.INTER_CUBIC):
    return cv2.remap(image, x.astype(np.float32), y.astype(np.float32), interpolation,
                     borderMode=cv2.BORDER_REPLICATE)

def fill_edges(rgb,mask):
    _,labels=cv2.distanceTransformWithLabels((~mask).astype(np.uint8),cv2.DIST_L2,5,labelType=cv2.DIST_LABEL_PIXEL)
    bank=np.zeros((labels.max()+1,3),np.uint8)
    bank[labels[mask]]=rgb[mask]
    return bank[labels]

class ClothChart:
    def __init__(self,tile,part,oy):
        ink=tile[...,3]>42
        self.rgb=fill_edges(tile[...,:3],ink)
        rows=np.flatnonzero(ink.any(1))
        yy=np.arange(len(tile))
        def smooth(v):
            a=np.interp(yy,rows,v).astype(np.float32)
            return cv2.GaussianBlur(a[:,None],(1,0),1.15).ravel()
        self.left=smooth(np.argmax(ink[rows],axis=1))-.5
        self.right=smooth(tile.shape[1]-1-np.argmax(ink[rows,::-1],axis=1))+.5
        self.ys=np.array([rows[0]-.5,rows[-1]+.5])
        self.vs=np.array([0.,1.])
        if part==3:
            self.ys=np.array([self.ys[0],56.-oy,73.-oy,self.ys[-1]])
            self.vs=np.array([0,.40,.64,1])
    def local_xy(self,delta,v):
        y=np.interp(v,self.vs,self.ys)
        l=np.interp(y,np.arange(len(self.left)),self.left)
        r=np.interp(y,np.arange(len(self.right)),self.right)
        return (r+l)*.5+(r-l)*.5*np.sin(delta),y
    def coords(self):
        y,x=np.indices(self.rgb.shape[:2])
        width=self.right-self.left
        sin=np.clip((2*x-self.right[:,None]-self.left[:,None])/width[:,None],-1,1)
        longitude=.5+np.arcsin(sin)/np.pi
        v=np.interp(y,self.ys,self.vs)
        return np.uint8(np.rint(255*np.stack((longitude,v),axis=-1)).clip(0,255))

class Pigment:
    """Address original brush strokes in the large master with the KRA cutout coordinates."""
    def __init__(self,master,key):
        col,row={0:(0,0),2:(2,0),4:(0,0),6:(1,1)}[key]
        h,w=master.shape[0]//2,master.shape[1]//3
        cell=master[row*h:(row+1)*h,col*w:(col+1)*w].copy()
        alpha=cell[...,3]
        _,labels,stats,_=cv2.connectedComponentsWithStats(np.uint8(alpha>42),connectivity=4)
        index=1+np.argmax(stats[1:,cv2.CC_STAT_AREA])
        margin=cv2.dilate(np.uint8(labels==index),cv2.getStructuringElement(cv2.MORPH_CROSS,(3,3)),iterations=3)
        ys,xs=np.where((margin>0)&(alpha>16))
        self.rgba=cell[ys.min():ys.max()+1,xs.min():xs.max()+1]
        self.width=round(self.rgba.shape[1]*152/self.rgba.shape[0])
        self.x0=(SIZE-self.width)//2
        self.flip=key==4

    def read(self,chart,ox,oy,x,y):
        painted=remap(chart.rgb,x,y)
        gx=x+ox
        if self.flip:
            gx=(SIZE-1)-gx
        mx=(gx-self.x0+.5)*(self.rgba.shape[1]/self.width)-.5
        my=(y+oy-18+.5)*(self.rgba.shape[0]/152)-.5
        original=remap(self.rgba,mx,my)
        # The drawn lining extended under a cuff in Krita can cover a completely
        # different object in the original image. Retain that painted repair.
        delta=np.abs(painted.astype(float)-original[...,:3]).mean(2)
        keep=(original[...,3]>242)&(cv2.GaussianBlur(delta,(0,0),2)<12)
        return np.where(keep[...,None],original[...,:3],painted)

def seam(a,b,xmid,band):
    # A single seam runs along the darkest matching fold near the edge of a
    # painted panel. It stays in the same place on the cloth for every frame.
    x=np.arange(xmid-band,xmid+band+1)%WIDTH
    aa=a[:,x].astype(float)
    bb=b[:,x].astype(float)
    mismatch=abs(aa-bb).mean(2)+.10*(aa+bb).mean(2)+9*(np.linspace(-1,1,len(x))**2)
    cost=mismatch[0].copy()
    pred=np.zeros(mismatch.shape,np.int32)
    ids=np.arange(len(x))
    for row in range(1,len(mismatch)):
        paths=np.stack((np.r_[1e6,cost[:-1]]+1.7,cost,np.r_[cost[1:],1e6]+1.7))
        move=paths.argmin(0)
        pred[row]=ids+move-1
        cost=paths[move,ids]+mismatch[row]
    result=np.zeros(HEIGHT,np.int32)
    mark=cost.argmin()
    for row in range(HEIGHT-1,-1,-1):
        result[row]=mark+xmid-band
        mark=pred[row,mark]
    return result

def panoramic(charts,part):
    angles=2*np.pi*(np.arange(WIDTH)+.5)/WIDTH
    theta,v=np.meshgrid(angles,(np.arange(HEIGHT)+.5)/HEIGHT)
    keys=[]
    maps=[]
    for d,(chart,pigment,ox,oy) in zip(CARDINAL,charts):
        delta=(theta-d*np.pi/4+np.pi)%(2*np.pi)-np.pi
        x,y=chart.local_xy(np.clip(delta,-np.pi/2,np.pi/2),v)
        maps.append(np.stack((x,y),axis=-1).astype(np.float32))
        keys.append(pigment.read(chart,ox,oy,x,y))
    # Give the hollow face and its brass clasp one continuous frontal panel.
    # Place the lining seam on the actual side of the hood, outside the aperture.
    centers=(22,158,225,315) if part==3 else (45,135,225,315)
    mids=[round(WIDTH*d/360) for d in centers]
    band=WIDTH//24
    yy,xx=np.indices((HEIGHT,WIDTH),dtype=np.float32)
    offsets=np.zeros((4,HEIGHT,WIDTH,2),np.float32)
    dis=cv2.DISOpticalFlow_create(cv2.DISOPTICAL_FLOW_PRESET_MEDIUM)
    dis.setFinestScale(0)
    dis.setUseSpatialPropagation(True)
    for i,mid in enumerate(mids):
        j=(i+1)%4
        lo=max(0,mid-band*2)
        hi=min(WIDTH,mid+2*band+1)
        l=cv2.cvtColor(keys[i][:,lo:hi],cv2.COLOR_RGB2GRAY)
        r=cv2.cvtColor(keys[j][:,lo:hi],cv2.COLOR_RGB2GRAY)
        to_right=dis.calc(l,r,None)
        to_left=dis.calc(r,l,None)
        # Meet in garment coordinates at the stitched selvage. Keep the
        # actual design and straight rows in the center of both fabric panels.
        weight=np.maximum(0,1-np.abs(np.arange(lo,hi)-mid)/(2*band))
        weight=weight*weight*(3-2*weight)
        for piece,field in ((i,to_right),(j,to_left)):
            offsets[piece,:,lo:hi]-=.5*weight[None,:,None]*np.clip(field,[-12,-15],[12,15])
    for i,(chart,pigment,ox,oy) in enumerate(charts):
        origin=remap(maps[i],xx+offsets[i,:,:,0],yy+offsets[i,:,:,1],cv2.INTER_LINEAR)
        keys[i]=pigment.read(chart,ox,oy,origin[:,:,0],origin[:,:,1])
    cuts=[seam(keys[i],keys[(i+1)%4],mid,band//2) for i,mid in enumerate(mids)]
    x=np.arange(WIDTH)[None,:]
    out=np.empty_like(keys[0])
    for i in range(4):
        a=cuts[i-1][:,None]
        b=cuts[i][:,None]
        use=((x>=a)&(x<b)) if i else ((x>=a)|(x<b))
        out[use]=keys[i][use]
    return out

def build(folder,header,output):
    """Keep the existing red signed distance and store the cloth chart in G/B."""
    rows=re.findall(r"\{0,(\d+),(\d+),(\d+),(\d+),(\d+),(\d+)\}",header.read_text())
    rect=np.asarray(rows,int).reshape(32,6,6)
    art=np.asarray(Image.open(folder/'parts-0.png').convert('RGBA'))
    edge=np.asarray(Image.open(folder/'edges.png').convert('RGB'))
    coords=edge.copy()
    panels=[]
    master=np.asarray(Image.open(REFERENCE).convert("RGBA"))
    pigment={k:Pigment(master,k) for k in CARDINAL}
    cv2.setNumThreads(2)
    for p in range(6):
        views=[]
        for k,(x,y,w,h,ox,oy) in enumerate(rect[:,p]):
            chart=ClothChart(art[y:y+h,x:x+w],p,oy)
            uv=chart.coords()
            coords[y-2:y+h+2,x-2:x+w+2,1:]=np.pad(uv,((2,2),(2,2),(0,0)),mode='edge')
            if k//4 in CARDINAL and k%4==0:
                views.append((chart,pigment[k//4],ox,oy))
        panels.append(np.pad(panoramic(views,p),((HEM,HEM),(0,0),(0,0)),mode="edge"))
    output.mkdir(parents=True,exist_ok=True)
    Image.fromarray(coords).save(output/'edges.png',optimize=True)
    Image.fromarray(np.concatenate(panels)).save(output/'weave.png',optimize=True)


