"""Deliver verified material-only changes, preserving the tested executables.

Backs up active runtime assets and both ZIPs, replaces only seven PNGs, the
provenance receipt and LICENSES, then compares every archived entry bytewise.
"""
from pathlib import Path
import argparse,copy,json,shutil,uuid,zipfile
from caldera_comfy import NAMES,REPO,sha,write

def run(root):
 assert json.loads((root/'acceptance.json').read_text())['status']=='PASS'
 assert json.loads((REPO/'dist/qa-volcano-comfy-20261104/checks/summary.json').read_text())['status']=='PASS'
 assert not (root/'delivery.json').exists(),'Inspect completed delivery before retrying'
 backup=root/'delivery-backup';backup.mkdir(exist_ok=True);rows=[]
 changes={f'assets/volcano/{n}.png':REPO/f'assets/volcano/{n}.png' for n in NAMES}
 changes['assets/volcano/redesign-receipt.json']=REPO/'assets/volcano/redesign-receipt.json';changes['LICENSES.md']=REPO/'LICENSES.md'
 for destination in ['dist','dist/player']:
  exe=REPO/destination/'Emberfall.exe';before=sha(exe)
  for relative,source in changes.items():
   target=REPO/destination/relative;saved=backup/destination/relative;saved.parent.mkdir(parents=True,exist_ok=True)
   if target.exists():shutil.copy2(target,saved)
   target.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(source,target);assert sha(source)==sha(target)
  assert sha(exe)==before;rows.append({'runtime':destination,'exe_unchanged':True,'exe_sha256':before,'changed_files':len(changes)})
 for name in ['Emberfall-Windows-x64.zip','Emberfall-DEV-Windows-x64.zip']:
  archive=REPO/name;original=backup/name;shutil.copy2(archive,original);temporary=archive.with_name(archive.stem+'-'+uuid.uuid4().hex+'.zip');replaced=set()
  with zipfile.ZipFile(original) as old,zipfile.ZipFile(temporary,'w') as new:
   new.comment=old.comment
   for entry in old.infolist():
    relative=entry.filename.split('/',1)[-1];source=changes.get(relative)
    payload=source.read_bytes() if source else old.read(entry);new.writestr(copy.copy(entry),payload)
    if source:replaced.add(relative)
  assert replaced==set(changes),(name,'missing entries',set(changes)-replaced)
  with zipfile.ZipFile(original) as old,zipfile.ZipFile(temporary) as new:
   assert new.testzip() is None;assert old.namelist()==new.namelist()
   for entry in old.infolist():
    source=changes.get(entry.filename.split('/',1)[-1]);expected=source.read_bytes() if source else old.read(entry)
    assert new.read(entry.filename)==expected,'ZIP mismatch: '+entry.filename
  temporary.replace(archive);rows.append({'archive':name,'sha256':sha(archive),'bytes':archive.stat().st_size,'replaced_entries':len(replaced),'all_other_entries_byte_identical':True,'native_masters_included':False})
 write(root/'delivery.json',{'status':'PASS','backups':str(backup),'checks':rows});print(json.dumps(rows))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();run(a.root)
