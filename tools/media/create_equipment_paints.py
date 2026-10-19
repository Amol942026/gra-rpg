"""Author equipment palettes and inlays in Krita, then bake compact drawing data."""
from pathlib import Path
import json, re, shutil, hashlib, argparse
from krita_mcp_client import KritaMCP, start_krita_if_needed

# Primary material, secondary material, trim, jewel/inlay; original 2D art.
DESIGNS = [
 ("Sword", "c7d8d0", "65472f", "c4a66a", "e8e8be", "blade"),
 ("Axe", "92a6ac", "6b432c", "c3a062", "c8d6d7", "axe"),
 ("Helmet", "8eaaa9", "536a70", "c4a66a", "d9e0ce", "crest"),
 ("Armor", "8ea5a5", "2b675d", "c4a66a", "acc7b5", "sun"),
 ("Shield", "976c3b", "5f412d", "c4a66a", "b2bab0", "cross"),
 ("Boots", "78543a", "3f352c", "c4a66a", "d0b885", "lacing"),
 ("Ring", "c4a66a", "795e41", "e0c68a", "e98643", "sun"),
 ("Dagger", "bdc6ba", "5f4d36", "bca376", "cbe3d1", "blade"),
 ("Cloak", "766a53", "484d43", "c4aa79", "d4c8a8", "fang"),
 ("Amulet", "ceaa65", "715337", "edd398", "dca64d", "gem"),
 ("Bow", "a77740", "62422e", "c4a66a", "d9c597", "arc"),
 ("WolfKnife", "e0d8b6", "55676c", "a8b7b6", "d4ece5", "fang"),
 ("WolfCloak", "7d9693", "43545a", "ccd0b9", "b3d4cd", "fang"),
 ("MoonBow", "e0dfbd", "657d89", "b7cdd8", "9fe1ed", "moon"),
 ("BoarArmor", "815438", "533d31", "b7a17d", "e1d0a2", "tusk"),
 ("TuskAxe", "dbd0ac", "70492e", "b8a46e", "e9d8ad", "tusk"),
 ("SilkBoots", "a6b1bd", "536478", "cfced3", "a4d7de", "web"),
 ("VenomRing", "b7c9c2", "3f6859", "d5d9b1", "76d26b", "web"),
 ("WebCowl", "666c91", "42465f", "c0c6d2", "b6d5e9", "web"),
 ("BoneHelmet", "d6c8a4", "57666a", "a8ab94", "efe2bd", "bone"),
 ("CryptSword", "557987", "323d50", "a9bac5", "7de4e9", "rune"),
 ("BanditBow", "664a38", "362d31", "9c8a6b", "a8b49d", "arc"),
 ("BanditBoots", "4c4941", "2d3234", "9b9472", "8aa191", "lacing"),
 ("RootShield", "60754b", "403f2d", "a18c58", "b6c779", "root"),
 ("GroveAmulet", "a38e50", "495838", "caac64", "e0ad4b", "root"),
 ("GelAmulet", "5fb99b", "335e5e", "b0c9b1", "a4ecc4", "drop"),
 ("GuardianSword", "9eaaa1", "465e5d", "c4ac69", "90d8b8", "rune"),
 ("GuardianArmor", "697f77", "3c5450", "bda96a", "9ec8a2", "rune"),
 ("GuardianHelmet", "7e9380", "4e6055", "b3a76f", "c1d8ac", "root"),
 ("EchoRing", "9d8dba", "554b6c", "c7b5ce", "db9af0", "rune"),
 ("StingDagger", "dba659", "6b4c34", "cbb175", "f3d18a", "fang"),
 ("ScarabHelmet", "b5824b", "376d6b", "e0b969", "7ec7b4", "scarab"),
 ("ScarabArmor", "b3854b", "356969", "dfb972", "86c8b2", "scarab"),
 ("CobraBoots", "b29a56", "4e6950", "dfbb78", "c5d194", "scales"),
 ("MirageBow", "d3af68", "596c67", "f1d99a", "91d6cb", "rune"),
 ("SunAmulet", "d8ac58", "9a6237", "f8d994", "f1bd50", "sun"),
 ("DuneBlade", "d5ad61", "7e6a48", "f3d494", "b7e5d0", "wave"),
 ("SandRobe", "c8ac73", "8e7250", "edce8b", "a3d4c6", "wave"),
 ("DuneShield", "8c8070", "615c51", "d6b675", "efce8d", "wave"),
]
PATTERNS = {
 "blade":[[(8,2),(8,14)],[(4,10),(12,10)]],
 "axe":[[(6,14),(10,2)],[(6,3),(12,3),(14,6),(9,8)]],
 "crest":[[(4,12),(4,6),(8,2),(12,6),(12,12)],[(8,3),(8,14)]],
 "sun":[[(8,3),(13,8),(8,13),(3,8),(8,3)],[(8,0),(8,2)],[(8,14),(8,16)],[(0,8),(2,8)],[(14,8),(16,8)]],
 "cross":[[(3,8),(13,8)],[(8,3),(8,13)]],
 "lacing":[[(4,3),(12,6),(4,9),(12,12)],[(12,3),(4,6),(12,9),(4,12)]],
 "fang":[[(3,2),(6,5),(8,14),(10,5),(13,2)]],
 "gem":[[(8,2),(13,7),(8,14),(3,7),(8,2)],[(3,7),(13,7)]],
 "arc":[[(4,2),(10,4),(12,8),(10,12),(4,14)],[(4,2),(7,8),(4,14)]],
 "moon":[[(10,2),(5,4),(3,8),(5,12),(10,14),(7,10),(7,6),(10,2)]],
 "tusk":[[(2,3),(3,10),(6,14),(7,10)],[(14,3),(13,10),(10,14),(9,10)]],
 "web":[[(1,1),(15,15)],[(15,1),(1,15)],[(8,0),(8,16)],[(0,8),(16,8)],[(8,3),(13,8),(8,13),(3,8),(8,3)]],
 "bone":[[(4,4),(12,12)],[(12,4),(4,12)],[(2,4),(4,2)],[(12,14),(14,12)]],
 "rune":[[(8,1),(3,5),(8,8),(13,5),(8,1)],[(8,8),(8,15)],[(3,11),(8,8),(13,11)]],
 "root":[[(8,15),(8,3)],[(8,7),(3,3)],[(8,11),(13,5)],[(8,12),(3,10)]],
 "drop":[[(8,1),(3,8),(4,12),(8,15),(12,12),(13,8),(8,1)]],
 "scarab":[[(8,2),(4,5),(4,11),(8,14),(12,11),(12,5),(8,2)],[(8,3),(8,13)],[(1,4),(4,6)],[(1,12),(4,10)],[(15,4),(12,6)],[(15,12),(12,10)]],
 "scales":[[(1,3),(5,6),(9,3),(13,6),(16,3)],[(1,9),(5,12),(9,9),(13,12),(16,9)]],
 "wave":[[(1,5),(5,2),(10,7),(15,4)],[(1,11),(5,8),(10,13),(15,10)]],
}

def main(args):
 root=Path(__file__).resolve().parents[2]
 source=Path(r"C:\AI2D\sprites")/args.name
 kra,png=source.with_suffix('.kra'),source.with_suffix('.png')
 if kra.exists() or png.exists():raise FileExistsError(source)
 start_krita_if_needed()
 with KritaMCP(args.log_directory) as c:
  doc=c.call('krita_document',action='create',width=512,height=320,name=args.name)['affected_document_id']
  state=c.call('krita_state',document_id=doc)
  c.call('krita_edit',action='select_all',document_id=doc)
  c.call('krita_edit',action='clear',document_id=doc,layer_id=state['active_layer_id'])
  c.call('krita_edit',action='deselect',document_id=doc)
  for index,(kind,*values) in enumerate(DESIGNS):
   c.call('krita_layers',action='create',document_id=doc,name=kind+' materials and inlay')
   layer=c.call('krita_state',document_id=doc)['active_layer_id']
   x,y=(index%8)*64,(index//8)*64
   for n,color in enumerate(values[:4]):
    c.call('krita_draw',shape='rectangle',document_id=doc,layer_id=layer,x=x+n*16,y=y,width=16,height=8,foreground='#'+color,fill_style='ForegroundColor',stroke_style='None')
   for points in PATTERNS[values[4]]:
    c.call('krita_draw',shape='path',document_id=doc,layer_id=layer,points=[{'x':x+16+a*2,'y':y+24+b*2} for a,b in points],foreground='#'+values[3],brush_size=3,fill_style='None',stroke_style='ForegroundColor')
  c.call('krita_save_export',document_id=doc,kra_path=str(kra),png_path=str(png),overwrite=False)
  c.call('krita_preview',document_id=doc,max_width=768,max_height=480)
  c.call('krita_document',action='close',document_id=doc,discard_changes=False)
 bake(root,kra,png)

def bake(root,kra,png):
 from PIL import Image
 image=Image.open(png).convert('RGBA')
 enum=re.search(r'enum class Kind \{(.*?)\};',(root/'src/items.hpp').read_text(encoding='utf-8'),re.S).group(1)
 kinds=[v.strip() for v in enum.split(',') if v.strip()!='Count']
 palette=[['0x8ea5a5','0x2b675d','0xc4a66a','0xc7d8d0'] for _ in kinds]
 rects=[];spans=[(0,0) for _ in kinds]
 for index,(kind,*_) in enumerate(DESIGNS):
  x,y=(index%8)*64,(index//8)*64;key=kinds.index(kind)
  palette[key]=['0x%02x%02x%02x'%image.getpixel((x+8+n*16,y+4))[:3] for n in range(4)]
  start=len(rects)
  for row in range(16):
   col=0
   while col<16:
    if image.getpixel((x+17+col*2,y+25+row*2))[3]<100:col+=1;continue
    left=col;col+=1
    while col<16 and image.getpixel((x+17+col*2,y+25+row*2))[3]>=100:col+=1
    rects.append((left,row,col-left))
  spans[key]=(start,len(rects)-start)
 output=root/'src/generated/equipment_paints.hpp'
 code=['#pragma once','// Generated from native Krita artwork by create_equipment_paints.py.','namespace equipment_paint_data {','struct Paint { unsigned primary,secondary,trim,accent; };','struct Stroke { unsigned char x,y,width; };','struct Span { unsigned short start,count; };',f'inline constexpr Paint paints[{len(kinds)}]={{']
 code += [' {'+','.join(p)+'}, // '+kind for p,kind in zip(palette,kinds)]
 code += ['};',f'inline constexpr Stroke strokes[{len(rects)}]={{']+[' {'+','.join(map(str,r))+'},' for r in rects]+['};',f'inline constexpr Span spans[{len(kinds)}]={{']+[' {'+str(s)+','+str(n)+'},' for s,n in spans]+['};','}']
 output.write_text('\n'.join(code)+'\n',encoding='utf-8')
 destination=root/'art/equipment/equipment-paints.png';destination.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(png,destination)
 metadata={'master':str(kra),'png':str(png),'sha256':hashlib.sha256(png.read_bytes()).hexdigest(),'authoring':'Original native Krita 2D palettes and inlays; no external assets','items':[d[0] for d in DESIGNS],'generated':str(output),'strokes':len(rects)}
 destination.with_suffix('.json').write_text(json.dumps(metadata,indent=2),encoding='utf-8')
 print(json.dumps({'status':'PASS','items':len(DESIGNS),'strokes':len(rects),'master':str(kra)}))

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--name',required=True);p.add_argument('--log-directory',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);p.add_argument('--bake-only',action='store_true');args=p.parse_args()
 if args.bake_only:
  source=Path(r'C:\AI2D\sprites')/args.name;bake(Path(__file__).resolve().parents[2],source.with_suffix('.kra'),source.with_suffix('.png'))
 else:main(args)
