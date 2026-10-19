"""Regression guard: native Close is blocked and saved projects are retained.

Runs only editing operations, never playback. Requires --no-audio.
"""
import argparse,json,sys,uuid
from pathlib import Path
sys.path.insert(0,r"C:\AI2D\audio\scripts")
from audacity_control import AudacityClient,project_windows,AudacityError
from audacity_workflow import AudacityProject,file_sha


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--no-audio",action="store_true",required=True)
    a=p.parse_args()
    folder=Path(r"C:\AI2D\pipeline")/("close_recovery_"+uuid.uuid4().hex[:10])
    folder.mkdir(parents=True)
    source=Path(r"C:\AI2D\audio\sfx\zaklecie_elektryczne.wav")
    digest=file_sha(source);checks=[]
    with AudacityClient(auto_start=True,log_path=folder/"commands.jsonl") as c:
        c.help("Help");c.info("Tracks")
        baseline={w["hwnd"] for w in project_windows(c.server_pid)}
        first=AudacityProject.create(c,folder,name="Close_test_first")
        first.import_audio(source)
        first.trim(.1,2.1);first.select_all();first.normalize(-3)
        first.export_wav(folder/"first.wav");first.save_project(folder/"first.aup3")
        second=AudacityProject.create(c,folder,name="Close_test_second")
        second.import_audio(source)
        second.trim(.1,2.1);second.select_all();second.normalize(-3)
        second.export_wav(folder/"second.wav");second.save_project(folder/"second.aup3")
        try:first.close_project()
        except AudacityError:pass
        else:raise AssertionError("Unsafe native Close was enabled")
        first.release_saved_project()
        assert not first._owned and len(second.tracks())==1
        checks.append({"name":"Release preserves the other working project","status":"PASS"})
        second.release_saved_project()
        assert not second._owned
        checks.append({"name":"Both saved projects release ownership without native Close","status":"PASS"})
        c.connect()
        assert baseline.issubset({w["hwnd"] for w in project_windows(c.server_pid)})
        assert {first._hwnd,second._hwnd}.issubset({w["hwnd"] for w in project_windows(c.server_pid)})
        checks.append({"name":"Pre-existing project windows remain intact","status":"PASS"})
    rows=[json.loads(line) for line in (folder/"commands.jsonl").read_text(encoding="utf-8").splitlines()]
    assert not any(row.get("command","").startswith("Close:") for row in rows)
    assert sum(row.get("event")=="saved_project_released_without_close" for row in rows)==2
    checks.append({"name":"No unstable pipe Close command was sent","status":"PASS"})
    assert file_sha(source)==digest
    report={"status":"PASS","checks":checks,"original_unchanged":True,"audio_played":False,"directory":str(folder)}
    (folder/"report.json").write_text(json.dumps(report,indent=2),encoding="utf-8")
    print(json.dumps(report,indent=2))


if __name__=="__main__":main()
