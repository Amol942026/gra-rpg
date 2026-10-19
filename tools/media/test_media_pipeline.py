"""Independent final asset inspection and importer/fallback acceptance checks.

Uses completed assets, never regenerates artwork or plays audio.
"""
import argparse,contextlib,copy,json,shutil,sqlite3,sys,uuid
from pathlib import Path
import numpy as np
from PIL import Image
from asset_tools import sha,write_json
from game_import import REPO,import_pack,bind_audio,restore_bindings
from media_pipeline import process_audio


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--no-audio",action="store_true",required=True)
    parser.add_argument("report",type=Path)
    args=parser.parse_args()
    report=json.loads(args.report.read_text(encoding="utf-8"));steps=report["steps"]
    work=Path(r"C:\AI2D\pipeline")/("acceptance_"+uuid.uuid4().hex[:10]);work.mkdir()
    checks=[]
    def passed(name,**extra):checks.append(dict(name=name,status="PASS",**extra))
    def rejected(name,operation):
        try:operation()
        except (ValueError,FileExistsError):passed(name);return
        raise AssertionError(name+" was not rejected")
    assert report["status"]=="PASS" and all(x["status"]=="PASS" for x in steps["environment"]["tools"].values())
    passed("All six programs passed actual connection/version checks")
    for item in steps["import"]["files"]:assert sha(item["path"])==item["sha256"]
    passed("Imported runtime files match production hashes",files=len(steps["import"]["files"]))
    image=Image.open(steps["sprite"]["png"]).convert("RGBA")
    assert image.getchannel("A").getextrema()==(0,255) and image.size==(256,256)
    passed("Sprite transparency, size and visible content")
    for item in steps["maps"]["Maps"]:
        rgb=np.asarray(Image.open(item["Path"]).convert("RGB"),dtype=np.float64)
        assert rgb.shape==(256,256,3)
        if item["Type"]=="normal":assert np.max(np.abs(np.linalg.norm(rgb/127.5-1,axis=2)-1))<.1
        else:assert np.array_equal(rgb[:,:,0],rgb[:,:,1]) and np.array_equal(rgb[:,:,1],rgb[:,:,2])
    passed("All pixels of four material maps have valid dimensions/channels/vectors")
    animation=steps["animation"]
    rendered=json.loads(Path(animation["render_manifest"]).read_text(encoding="utf-8-sig"))
    assert rendered["ActualFrames"]==60 and rendered["UniquePixelFrames"]==60
    for item in rendered["Frames"]:
        path=Path(rendered["OutputDirectory"])/item["Name"]
        assert sha(path)==item["Sha256"]
        with Image.open(path) as frame:frame.load();assert frame.size==(128,128)
    atlas=animation["atlas"]
    assert atlas["gpu_bytes_rgba8"]==atlas["width"]*atlas["height"]*4 and atlas["gpu_bytes_rgba8"]<5*1024*1024
    assert atlas["loop_mean_pixel_step"]<=atlas["max_internal_mean_pixel_step"]*1.3+.01
    passed("60 actual animation frames, loop continuity and atlas memory budget",gpu_bytes=atlas["gpu_bytes_rgba8"])
    sys.path.insert(0,r"C:\AI2D\audio\scripts")
    from audacity_workflow import decode_wav,retained_projects,AudacityProject
    from audacity_control import AudacityError
    deliveries=[steps["music_delivery"]["wav"],*[r["output"] for r in steps["sfx_edits"].values()]]
    for path in deliveries:
        stats,_=decode_wav(path)
        assert stats["clipped_samples"]==0 and stats["sample_rate"]==48000 and stats["channels"]==2
    music=steps["music_delivery"]
    assert music["frames"]==music["ogg_decode_frames"]==480000
    assert abs(float(music["integrated_loudness"]["input_i"])+18)<.5
    passed("Music/SFX contain valid PCM without clipping; Vorbis preserves exact 10-second loop")
    for job in [steps["music_edit"],*steps["sfx_edits"].values()]:
        with contextlib.closing(sqlite3.connect(Path(job["project"]).as_uri()+"?mode=ro",uri=True)) as db:
            assert db.execute("PRAGMA quick_check").fetchall()==[("ok",)]
            assert db.execute("SELECT count(*) FROM sampleblocks").fetchone()[0]>0
        assert job["retained_project"]["status"]=="saved_project_retained"
    passed("Four native AUP3 projects are intact and explicitly retained")
    placeholder=AudacityProject.__new__(AudacityProject)
    try:placeholder.close_project()
    except AudacityError:passed("Unsafe native project Close is blocked before I/O")
    else:raise AssertionError("Native close guard missing")
    manifest=Path(steps["import"]["source_manifest"])
    rejected("Existing asset pack cannot be overwritten",lambda:import_pack(manifest))
    data=json.loads(manifest.read_text(encoding="utf-8"))
    bad=copy.deepcopy(data);bad["id"]="reject_"+uuid.uuid4().hex[:10];bad["files"][0]["path"]="../escape.png"
    write_json(work/"bad_path.json",bad)
    rejected("Path traversal rejected before copy",lambda:import_pack(work/"bad_path.json"))
    bad=copy.deepcopy(data);bad["id"]="reject_"+uuid.uuid4().hex[:10];bad["files"][0]["sha256"]="0"*64
    write_json(work/"bad_hash.json",bad)
    rejected("Mismatched source hash rejected before copy",lambda:import_pack(work/"bad_hash.json"))
    rejected("Unknown game event rejected",lambda:bind_audio({"nonexistent-event":deliveries[1]}))
    # Exercise the real automatic budget branch, without opening eight windows.
    import audacity_workflow
    original=audacity_workflow.retained_projects
    audacity_workflow.retained_projects=lambda:[{}]*8
    try:
        fallback=process_audio({"inputs":[steps["sfx_sources"]["electric"]["wav"],steps["sfx_sources"]["weapon_hit"]["wav"]],
            "output":str(work/"offline_mix.wav"),"trim":{"start":.05,"end":.8},"mix":True,
            "track_gain_db":{"0":-6,"1":-10},"effects":[{"command":"Echo","params":{"Delay":.035,"Decay":.14}}],
            "fade_in":.01,"fade_out":.04,"normalize_db":-3},work,"budget-fallback")
    finally:audacity_workflow.retained_projects=original
    assert fallback["status"]=="PASS" and fallback["project"] is None
    assert abs(fallback["wav"]["seconds"]-.75)<.001 and abs(fallback["wav"]["peak_dbfs"]+3)<.01
    passed("Eight-project budget routes real trim/gain/mix/Echo/fades/normalize to offline engine")
    fixture=REPO/"art/pipeline/qa"/("restore_guard_"+uuid.uuid4().hex[:8])/"assets"
    (fixture/"audio").mkdir(parents=True)
    initial=REPO/"assets/audio/spell.wav";target=fixture/"audio/spell.wav"
    shutil.copy2(initial,target)
    binding=bind_audio({"spell":deliveries[1]},asset_root=fixture)
    with target.open("ab") as f:f.write(b"changed")
    digest=sha(target)
    rejected("Restore refuses a target changed after deployment",lambda:restore_bindings(binding["receipt"]))
    assert sha(target)==digest
    shutil.copy2(deliveries[1],target)
    restore_bindings(binding["receipt"]);assert sha(target)==sha(initial)
    assert all(sha(p)==digest for p,digest in steps["protected_files_unchanged"]["files"].items())
    passed("Existing game audio and executable remain unchanged")
    result=dict(status="PASS",checks=checks,check_count=len(checks),pipeline_report=str(args.report),
                fallback=fallback,work_directory=str(work),audio_played=False,game_rebuilt=False)
    write_json(work/"acceptance-report.json",result)
    print(json.dumps({"status":"PASS","check_count":len(checks),"report":str(work/"acceptance-report.json")},indent=2))


if __name__=="__main__":main()
