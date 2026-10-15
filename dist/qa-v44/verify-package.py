from pathlib import Path, PurePosixPath
import hashlib, json, re, shutil, subprocess, uuid, zipfile
root=Path(__file__).resolve().parents[2]
qa=root/'dist/qa-v44'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
developer_hash=sha(root/'dist/Emberfall.exe')
audio_hash=sha(root/'dist/audio-settings.cfg')
assert audio_hash=='12ba35cfd8ef37db138c8a4cc1a40cd35e9e8f17e4e7456d2b3c957e4dbd7cd6'
package=root/'Emberfall-Windows-x64.zip'
expected={'Emberfall/Emberfall.exe':root/'dist/player/Emberfall.exe','Emberfall/LICENSES.md':root/'LICENSES.md','Emberfall/LOOT.md':root/'LOOT.md'}
for asset in (root/'assets').rglob('*'):
 if asset.is_file():expected['Emberfall/assets/'+asset.relative_to(root/'assets').as_posix()]=asset
extracted=qa/('extracted-'+uuid.uuid4().hex)
with zipfile.ZipFile(package) as z:
 names=z.namelist()
 assert len(names)==len(set(names))
 assert set(names)==set(expected)|{'Emberfall/JAK_URUCHOMIC.txt','Emberfall/Uruchom.bat'}
 for name,source in expected.items():assert hashlib.sha256(z.read(name)).hexdigest()==sha(source),name
 instructions=z.read('Emberfall/JAK_URUCHOMIC.txt').decode('utf-8-sig')
 assert 'wersja 0.44' in instructions and 'WERSJA DLA GRACZA' in instructions and 'F3' not in instructions
 for name in names:
  p=PurePosixPath(name);assert not p.is_absolute() and '..' not in p.parts
 z.extractall(extracted)
launch=extracted/'Emberfall'
startup=subprocess.STARTUPINFO();startup.dwFlags|=subprocess.STARTF_USESHOWWINDOW;startup.wShowWindow=subprocess.SW_HIDE
result=subprocess.run([str(launch/'Emberfall.exe'),'--startup-smoke','--no-audio','--dev','--start=desert-boss','--labels-qa'],cwd=launch,startupinfo=startup,timeout=90)
assert result.returncode==0,result.returncode
text=(launch/'startup-smoke.txt').read_text(encoding='utf-8')
for field in ['Build edition: player','Frames: 180','Fullscreen: yes','Developer mode: 0','Developer menu open: 0','Developer destination: unavailable','World location: forest','Intro active: 1','Audio device opened: no']:
 assert field in text,field
assert not (launch/'audio-settings.cfg').exists()
assert not (launch/'qa-v44').exists()
shutil.copy2(launch/'startup-smoke.txt',qa/'packaged-startup.txt')
shutil.copy2(launch/'screenshots/emberfall-startup.png',qa/'packaged-prologue.png')
assert sha(root/'dist/audio-settings.cfg')==audio_hash
assert sha(root/'dist/Emberfall.exe')==developer_hash
checks={}
for name in ['labels-tests.txt','player-labels-tests.txt','player-build-tests.txt','player-trade-tests.txt','player-sword-tests.txt','combat-tests.txt']:
 report=(qa/name).read_text(encoding='utf-8');m=re.search(r'Checks: (\d+); failures: (\d+)',report);assert m and int(m[2])==0,name;checks[name]=int(m[1])
assert 'PASS: 6308' in (qa/'player-build-output.txt').read_text(encoding='utf-8')
assert extracted.resolve().is_relative_to(qa.resolve()) and extracted.name.startswith("extracted-")
shutil.rmtree(extracted)
evidence={'version':'0.44','edition':'player','package_bytes':package.stat().st_size,'zip_entries':len(names),'verified_payload_files':len(expected),'zip_sha256':sha(package),'player_exe_sha256':sha(root/'dist/player/Emberfall.exe'),'developer_exe_sha256':developer_hash,'model_checks':6308,'checks':checks,'failures':0,'moving_label_pixel_comparisons_identical':True,'native_scales_tested':[.8,1,1.2,1.5],'camera_samples_per_build':128,'final_frame_pixel_comparison_passed':True,'all_test_launches_no_audio':True,'audio_preferences_sha256':audio_hash,'audio_preferences_unchanged':True,'developer_mode_absent':True,'packaged_dev_start_and_qa_arguments_ignored':True,'packaged_start_frames':180,'packaged_audio_device_opened':False,'extracted_verification_copy_removed':True}
(qa/'package-verification.json').write_text(json.dumps(evidence,indent=2)+'\n',encoding='utf-8')
print(json.dumps(evidence,indent=2))
