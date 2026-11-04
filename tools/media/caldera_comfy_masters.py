"""Retain editable layered OpenRaster sources for the ComfyUI material pass.

Krita is absent on this PC, despite older environment documentation. Do not
forge a KRA save: preserve the actual workflow/PNG masters and an ORA that
can be opened in Krita when installed. No unrelated app/config is modified.
"""
from pathlib import Path
import argparse,io,json,zipfile,xml.etree.ElementTree as ET
from PIL import Image
from caldera_comfy import NAMES,sha,write

def png(im):
 b=io.BytesIO();im.save(b,format='PNG');return b.getvalue()
def run(root):
 folder=root/'masters';folder.mkdir(exist_ok=True)
 for name in NAMES:
   receipt=folder/f'{name}-receipt.json'
   if receipt.exists():
    old=json.loads(receipt.read_text());assert sha(folder/f'{name}.ora')==old['ora_sha256'];continue
   im=Image.open(root/'runtime'/f'{name}.png').convert('RGBA');cell=im.width//8
   xml=ET.Element('image',{'w':str(im.width),'h':str(im.height),'name':'Emberfall / ComfyUI Caldera / '+name,'version':'0.0.3'});stack=ET.SubElement(xml,'stack')
   ora=folder/f'{name}.ora'
   with zipfile.ZipFile(ora,'w',zipfile.ZIP_DEFLATED) as z:
    z.writestr('mimetype','image/openraster',compress_type=zipfile.ZIP_STORED)
    for p,label in enumerate(['back','leg-left','leg-right','body','arm-left','arm-right']):
     layer=Image.new('RGBA',im.size);layer.paste(im.crop((0,p*cell,im.width,(p+1)*cell)),(0,p*cell));src=f'data/layer-{p}.png'
     ET.SubElement(stack,'layer',{'name':'ComfyUI material / '+label,'src':src,'opacity':'1.0','visibility':'visible','composite-op':'svg:src-over','x':'0','y':'0'});z.writestr(src,png(layer))
    ET.SubElement(stack,'layer',{'name':'Original before ComfyUI (backup)','src':'data/original.png','opacity':'1.0','visibility':'hidden','composite-op':'svg:src-over','x':'0','y':'0'})
    z.writestr('data/original.png',(root/'backup'/f'{name}.png').read_bytes());z.writestr('mergedimage.png',png(im));z.writestr('stack.xml',ET.tostring(xml,encoding='utf-8',xml_declaration=True))
   with zipfile.ZipFile(ora) as z:
    assert z.testzip() is None
    assert Image.open(z.open('mergedimage.png')).convert('RGBA').tobytes()==im.tobytes()
   write(receipt,{'format':'OpenRaster','layers':[x.attrib['name'] for x in stack],'ora_sha256':sha(ora),'pixels_match_runtime':True,'native_kra':False,'reason':'Krita and its bridge are not installed on this PC','generation':'local ComfyUI; original rig and alpha retained'})
   print('Saved editable OpenRaster master: '+name,flush=True)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();run(a.root)
