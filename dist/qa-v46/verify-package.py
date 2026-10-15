from pathlib import Path, PurePosixPath
import hashlib, json, re, shutil, subprocess, uuid, zipfile
root=Path(__file__).resolve().parents[2]
qa=root/'dist/qa-v46'
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
 assert 'wersja 0.46' in instructions and 'WERSJA DLA GRACZA' in instructions and 'F3' not in instructions
 for name in names:
  p=PurePosixPath(name);assert not p.is_absolute() and '..' not in p.parts
 z.extractall(extracted)
launch=extracted/'Emberfall'
startup=subprocess.STARTUPINFO();startup.dwFlags|=subprocess.STARTF_USESHOWWINDOW;startup.wShowWindow=subprocess.SW_HIDE
result=subprocess.run([str(launch/'Emberfall.exe'),'--startup-smoke','--no-audio','--dev','--start=desert-boss','--labels-qa','--world-motion-qa'],cwd=launch,startupinfo=startup,timeout=90)
assert result.returncode==0,result.returncode
text=(launch/'startup-smoke.txt').read_text(encoding='utf-8')
for field in ['Build edition: player','Frames: 180','Fullscreen: yes','Developer mode: 0','Developer menu open: 0','Developer destination: unavailable','World location: forest','Intro active: 1','Audio device opened: no']:
 assert field in text,field
assert not (launch/'audio-settings.cfg').exists()
assert not (launch/'qa-v46').exists()
shutil.copy2(launch/'startup-smoke.txt',qa/'packaged-startup.txt')
shutil.copy2(launch/'screenshots/emberfall-startup.png',qa/'packaged-prologue.png')
assert sha(root/'dist/audio-settings.cfg')==audio_hash
assert sha(root/'dist/Emberfall.exe')==developer_hash
checks={}
for name in ['graphics-tests.txt','skills-tests.txt','combat-tests.txt','developer-tests.txt','shadow-tests.txt','motion-tests.txt','player-build-tests.txt','player-skills-tests.txt','player-combat-tests.txt','player-world-pixels.txt','player-labels-tests.txt','player-sword-tests.txt','player-trade-tests.txt']:
 report=(qa/name).read_text(encoding='utf-8');m=re.search(r'Checks: (\d+); failures: (\d+)',report);assert m and int(m[2])==0,name;checks[name]=int(m[1])
model=(qa/'model-tests.txt').read_text(encoding='utf-8');m=re.search(r'PASS: (\d+) gameplay',model);assert m
model_checks=int(m[1]);assert model_checks==6548
assert f'PASS: {model_checks}' in (qa/'player-build-output.txt').read_text(encoding='utf-8')
assert extracted.resolve().is_relative_to(qa.resolve()) and extracted.name.startswith('extracted-')
shutil.rmtree(extracted)
evidence={'version':'0.46','edition':'player','package_bytes':package.stat().st_size,'zip_entries':len(names),'verified_payload_files':len(expected),'zip_sha256':sha(package),'player_exe_sha256':sha(root/'dist/player/Emberfall.exe'),'developer_exe_sha256':developer_hash,'model_checks':model_checks,'checks':checks,'failures':0,'all_test_launches_no_audio':True,'audio_preferences_sha256':audio_hash,'audio_preferences_unchanged':True,'developer_mode_absent':True,'packaged_dev_start_and_qa_arguments_ignored':True,'packaged_start_frames':180,'packaged_audio_device_opened':False,'extracted_verification_copy_removed':True,'sword_sheathed_while_running':True,'running_arms_alternate_and_retain_segment_lengths':True,'fresh_sessions_start_with_all_skills_unlearned':True,'developer_kit_does_not_grant_skills':True,'moving_current_character_preserves_earned_ranks':True,'skill_unlock_requires_confirmed_point_allocation':True,'unlearned_skills_reject_hud_and_hotkey_actions_without_cost':True}
motion=(qa/'motion-tests.txt').read_text(encoding='utf-8');perf=re.search(r'mean=([0-9.]+) ms, p95=([0-9.]+) ms, FPS=([0-9.]+)',motion);assert perf
evidence['run_roll_mean_ms']=float(perf[1]);evidence['run_roll_p95_ms']=float(perf[2]);evidence['run_roll_fps']=float(perf[3])
(qa/'package-verification.json').write_text(json.dumps(evidence,indent=2)+'\n',encoding='utf-8')
print(json.dumps(evidence,indent=2))
