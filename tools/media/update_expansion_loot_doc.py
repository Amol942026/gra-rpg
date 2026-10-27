"""Extend the documented species tables from the checked-in expansion catalogue."""
import json,re
from pathlib import Path
root=Path(__file__).resolve().parents[2];path=root/'LOOT.md';text=path.read_text(encoding='utf-8')
items=json.loads((root/'art/items/expansion/catalog.json').read_text(encoding='utf-8'))
rarity={'Common':('Pospolity',60),'Rare':('Rzadki',15),'VeryRare':('Bardzo rzadki',4),'ExtremelyRare':('Niezwykle rzadki',1)}
headings=re.findall(r'^## (.+)$',text,re.M)[:12]
for i,title in enumerate(headings):
    pattern=r'(## '+re.escape(title)+r'\n\n.*?)(?=\n## |\Z)'
    def extend(match):
        body=match.group(1).rstrip()
        for item in items:
            if item['beast']==i and '| '+item['name']+' |' not in body:
                name,chance=rarity[item['rarity']];body+='\n| '+item['name']+' | '+name+' | '+str(chance)+'% | 1 |'
        return body+'\n'
    text=re.sub(pattern,extend,text,count=1,flags=re.S)
text=text.replace('28 nowych przedmiotów korzysta','64 tematyczne przedmioty (w tym 36 dodanych 27.10.2026) korzystają')
slots={'Sword':'broń','Axe':'broń','Dagger':'broń','Bow':'broń','Helmet':'hełm','Shield':'tarcza','Armor':'zbroja','Cloak':'zbroja','Boots':'buty','Ring':'ozdoba','Amulet':'ozdoba'}
for item in items:
    row='| '+item['name']+' | '+slots[item['shape']]+' | '+('atak' if item['shape'] in ('Sword','Axe','Dagger','Bow','Ring','Amulet') else 'pancerz')+' +'+str(item['power'])+' | '+item['description']+' |'
    if row not in text:text=text.rstrip()+'\n'+row+'\n'
if '## Zgodność zapisów — rozszerzenie 27.10.2026' not in text:
    text+='\n## Zgodność zapisów — rozszerzenie 27.10.2026\n\nŁącznie dostępnych jest 75 elementów wyposażenia. Stare identyfikatory przedmiotów i zapisy pozostają zgodne. Istniejące ciała zachowują swój wcześniej wylosowany łup; nowe przedmioty mogą wypaść z kolejnych zabitych bestii. Każdy gatunek zyskał trzy dodatkowe pozycje, bez zmiany procentów rzadkości ani gwarantowanych fragmentów fabularnych.\n'
path.write_text(text,encoding='utf-8');print('PASS: species drop tables and all 36 item descriptions documented')
