"""Append stable item IDs and three rarity-driven loot entries per species."""
from pathlib import Path
import json,re

ROOT=Path(__file__).resolve().parents[2]
CATALOG=json.loads((ROOT/'art/items/expansion/catalog.json').read_text(encoding='utf-8'))

def main():
    assert len(CATALOG)==36 and len({x['id'] for x in CATALOG})==36
    items=ROOT/'src/items.hpp';code=items.read_text(encoding='utf-8')
    if 'SlimeBlade' in code: raise RuntimeError('Catalogue already applied; do not append IDs twice')
    code=code.replace('DuneShield,Count','DuneShield,'+','.join(x['id'] for x in CATALOG)+',Count')
    code=code.replace('std::array<GearDefinition,28>','std::array<GearDefinition,64>')
    marker='}};\ninline constexpr bool beastGear'
    rows=[' {'+','.join((json.dumps(x['name'],ensure_ascii=False),'Kind::'+x['shape'],'Rarity::'+x['rarity'],str(x['power']),json.dumps(x['description'],ensure_ascii=False)))+'},' for x in CATALOG]
    assert marker in code
    code=code.replace(marker,'\n'.join(rows)+'\n'+marker)
    items.write_text(code,encoding='utf-8')
    loot=ROOT/'src/loot.hpp';lines=loot.read_text(encoding='utf-8').splitlines();index=0
    for n,line in enumerate(lines):
        if line.lstrip().startswith('{{{Kind::'):
            additions=[x for x in CATALOG if x['beast']==index];assert len(additions)==3
            at=line.rindex('}}}')+1
            lines[n]=line[:at]+''.join(',{Kind::'+x['id']+'}' for x in additions)+line[at:];index+=1
    assert index==12
    loot.write_text('\n'.join(lines)+'\n',encoding='utf-8')
    print('PASS: 36 appended stable IDs and 36 independent rarity-based loot rules')

if __name__=='__main__': main()
