"""Emberfall local media pipeline. Rendering/editing only: never plays audio.

Entry point: python tools/media/media_pipeline.py --no-audio demo
All jobs have unique names, logs and hash manifests. No game rebuild is run.
"""
from __future__ import annotations
import argparse, copy, datetime, json, os, re, shutil, subprocess, sys, uuid
from pathlib import Path
from asset_tools import sha,write_json,bake_lighting,make_synfig_cutout,pack_atlas
from game_import import REPO,cue_names,import_pack,bind_audio,restore_bindings

BASE=Path(r"C:\AI2D")
PYTHON=Path(sys.executable)
POWERSHELL=Path(shutil.which("pwsh") or
    r"C:\Users\Amadeusz\.cache\codex-runtimes\codex-primary-runtime\dependencies\native\powershell\pwsh.exe")
APPS={"krita":r"C:\Program Files\Krita (x64)\bin\krita.exe",
      "synfig":r"C:\Program Files\Synfig\bin\synfig.exe",
      "laigter":r"C:\AI2D\Tools\Laigter\laigter.exe",
      "lmms":r"C:\Program Files\LMMS\lmms.exe",
      "csound":r"C:\Program Files\Csound6_x64\bin\csound.exe",
      "audacity":r"C:\Program Files\Audacity\Audacity.exe"}


def file_version(path):
    import ctypes as C
    from ctypes import wintypes as W
    library=C.WinDLL("version",use_last_error=True)
    size=library.GetFileVersionInfoSizeW
    size.argtypes=[W.LPCWSTR,C.POINTER(W.DWORD)];size.restype=W.DWORD
    read=library.GetFileVersionInfoW
    read.argtypes=[W.LPCWSTR,W.DWORD,W.DWORD,C.c_void_p];read.restype=W.BOOL
    query=library.VerQueryValueW
    query.argtypes=[C.c_void_p,W.LPCWSTR,C.POINTER(C.c_void_p),C.POINTER(W.UINT)];query.restype=W.BOOL
    ignored=W.DWORD();length=size(str(path),C.byref(ignored))
    if not length:raise C.WinError(C.get_last_error())
    buffer=C.create_string_buffer(length)
    if not read(str(path),0,length,buffer):raise C.WinError(C.get_last_error())
    pointer=C.c_void_p();count=W.UINT()
    if not query(buffer,"\\",C.byref(pointer),C.byref(count)):raise C.WinError(C.get_last_error())
    words=C.cast(pointer,C.POINTER(W.DWORD))
    if count.value<16 or words[0]!=0xFEEF04BD:raise ValueError("Invalid version resource.")
    major,minor=words[2],words[3]
    return f"{major>>16}.{major&65535}.{minor>>16}.{minor&65535}"


def run(arguments,directory,label,*,allowed=(0,),timeout=240,cwd=None,extra_path=None):
    directory=Path(directory);directory.mkdir(parents=True,exist_ok=True)
    env=os.environ.copy();env.update(QT_QPA_PLATFORM="windows",QT_OPENGL="software",SDL_AUDIODRIVER="dummy")
    if extra_path:env["PATH"]=str(extra_path)+os.pathsep+env.get("PATH","")
    proc=subprocess.run(list(map(str,arguments)),cwd=cwd or REPO,env=env,capture_output=True,
                        timeout=timeout,creationflags=subprocess.CREATE_NO_WINDOW)
    (directory/(label+"-stdout.log")).write_bytes(proc.stdout)
    (directory/(label+"-stderr.log")).write_bytes(proc.stderr)
    if proc.returncode not in allowed:
        raise RuntimeError(f"{label}: exit {proc.returncode}; see {directory}")
    return proc.stdout.decode("utf-8-sig","replace")


def legacy(script,arguments,directory,label):
    return json.loads(run([PYTHON,script,*arguments],directory,label))


def doctor(directory):
    from krita_mcp_client import KritaMCP,start_krita_if_needed
    report={"tools":{},"audio_played":False}
    for name,path in APPS.items():
        if not Path(path).is_file():
            report["tools"][name]={"status":"MISSING","path":path};continue
        item={"path":path,"sha256":sha(path)}
        try:
            if name=="audacity":
                sys.path.insert(0,str(BASE/"audio/scripts"))
                from audacity_control import AudacityClient
                with AudacityClient(auto_start=True,log_path=Path(directory)/"audacity-doctor.jsonl") as client:
                    client.help("Help");client.info("Tracks")
                    item.update(version=file_version(path),method="mod-script-pipe",pid=client.server_pid)
            elif name=="krita":
                bridge=start_krita_if_needed()
                with KritaMCP(Path(directory)/"krita-mcp") as client:
                    result=client.call("krita_document",action="compatibility")
                    item.update(version=result["runtime_compatibility"]["krita_version"],
                                method="krita-codex-mcp==0.2.0 via STDIO",bridge=bridge)
            else:
                version=run([path,"--version"],directory,name+"-version",allowed=(0,3) if name=="synfig" else (0,),cwd=Path(path).parent)
                help_flag="--help-all" if name=="synfig" else "--help"
                run([path,help_flag],directory,name+"-help",allowed=(0,1) if name=="csound" else (0,),cwd=Path(path).parent)
                # Csound prints version on stderr.
                if not version.strip():version=Path(directory,name+"-version-stderr.log").read_text(encoding="utf-8",errors="replace")
                if name=="csound":
                    combined=version+Path(directory,name+"-version-stderr.log").read_text(encoding="utf-8",errors="replace")
                    match=re.search(r"Csound version\s+([\d.]+)",combined)
                    if not match:raise ValueError("Csound returned no recognizable version.")
                    version="Csound "+match.group(1)
                item.update(version=version.strip().splitlines()[0],method="CLI and native project files")
            item["status"]="PASS"
        except Exception as error:
            item.update(status="FAIL",error=str(error))
        report["tools"][name]=item
    report["status"]="PASS" if all(x["status"]=="PASS" for x in report["tools"].values()) else "FAIL"
    report["game"]={"engine":"C++17 / raylib 5.5", "graphics":"procedural 2D and cached RGBA render textures",
                    "audio_cues":cue_names(),"music":"assets/audio/echoes-of-the-ruins.ogg",
                    "material_map_runtime_shader":False,"source":"src/audio.hpp; src/main.cpp; src/native_world.hpp"}
    write_json(Path(directory)/"environment.json",report)
    return report


def generate_maps(sprite,directory):
    run([POWERSHELL,"-NoProfile","-ExecutionPolicy","Bypass","-File",BASE/"Tools/Generate-LaigterMaps.ps1",
         "-ImagePath",sprite],directory,"laigter")
    manifests=sorted((BASE/"maps").glob(Path(sprite).stem+"_*/maps-manifest.json"),key=lambda p:p.stat().st_mtime)
    if not manifests:raise RuntimeError("Laigter returned no manifest.")
    result=json.loads(manifests[-1].read_text(encoding="utf-8-sig"))
    if result["Status"]!="PASS" or result["SourceSha256"]!=sha(sprite):raise RuntimeError("Invalid Laigter result.")
    return manifests[-1]


def animate(sprite,project,directory):
    setup=make_synfig_cutout(sprite,project)
    render_root=Path(directory)/"renders"
    run([POWERSHELL,"-NoProfile","-ExecutionPolicy","Bypass","-File",BASE/"animations/Render-Synfig.ps1",
         "-ProjectPath",project,"-OutputRoot",render_root,"-ExpectedFrames","60","-Threads","2","-RequireMotion"],directory,"synfig")
    manifests=list(render_root.glob("*/render-manifest.json"))
    if len(manifests)!=1:raise RuntimeError("Ambiguous Synfig output.")
    result=json.loads(manifests[0].read_text(encoding="utf-8-sig"))
    folder=Path(result["OutputDirectory"])
    frames=[folder/x["Name"] for x in result["Frames"]]
    if any(sha(p)!=x["Sha256"] for p,x in zip(frames,result["Frames"])):raise ValueError("Frame hash mismatch.")
    atlas=pack_atlas(frames,Path(directory)/"crystal_atlas.png")
    return {"setup":setup,"render_manifest":str(manifests[0]),"atlas":atlas}


def compose_music(name,directory):
    """Compose a fresh four-bar phrase, render 3 cycles for settled release tails."""
    score=copy.deepcopy(json.loads((BASE/"audio/music/fantasy_score.json").read_text(encoding="utf-8-sig")))
    score.update(title="Emberfall — Crystal Sanctum",key="D minor",bpm=96,bars=12)
    chords=[(50,[0,3,7]),(46,[0,4,7]),(43,[0,3,7]),(45,[0,4,7])]
    melody=[[74,77,81,77],[77,74,70,74],[79,77,74,70],[76,73,69,73]]
    def pitch(n):return ["C","C#","D","D#","E","F","F#","G","G#","A","A#","B"][n%12]+str(n//12-1)
    for track in score["tracks"]:
        phrase=[];role=track["role"]
        for bar,(root,intervals) in enumerate(chords):
            def note(beat,duration,n,velocity):
                phrase.append(dict(beat=bar*4+beat,duration=duration,pitch=pitch(n),velocity=velocity))
            if role=="harp":
                for j in range(8):note(j*.5,.38,root+12+intervals[[0,1,2,1,0,2,1,2][j]],65 if j%2==0 else 52)
            elif role=="flute":
                for j,n in enumerate(melody[bar]):note(j,.72,n,63 if j%2==0 else 57)
            elif role in ("pad","strings"):
                for interval in intervals:note(0,3.85,root+12+interval,42)
            elif role=="bass":
                note(0,1.65,root-12,54);note(2,1.65,root-12,49)
            elif role=="drum":
                note(0,.18,36,43);note(2,.15,36,35)
            else:
                raise ValueError("Unknown template instrument role: "+role)
        track["notes"]=[dict(n,beat=n["beat"]+repeat*16) for repeat in range(3) for n in phrase]
    score_path=BASE/"audio/music"/(name+"_score.json")
    write_json(score_path,score)
    result=legacy(BASE/"audio/music/lmms_music.py",["create","--name",name,"--score",score_path],directory,"lmms-render")
    wav=BASE/"audio/music"/(name+".wav")
    if not wav.exists():raise RuntimeError("LMMS produced no WAV.")
    return {"project":str(BASE/"audio/music"/(name+".mmp")),"wav":str(wav),"phrase_seconds":10,
            "rendered_cycles":3,"score":str(score_path),"result":result}


def generate_sfx(name,directory):
    outputs={}
    for index,preset in enumerate(("electric","weapon_hit","loot")):
        result=legacy(BASE/"audio/sfx/csound_generate.py",["generate","--preset",preset,"--name",name+"_"+preset,
                      "--seed",str(7530+index),"--gain","0.7"],directory,"csound-"+preset)
        if result["status"]!="PASS":raise RuntimeError("Csound render failed.")
        outputs[preset]=result
    return outputs


def process_audio(recipe,directory,label):
    recipe_path=Path(directory)/(label+"-recipe.json")
    write_json(recipe_path,recipe)
    sys.path.insert(0,str(BASE/"audio/scripts"))
    from audacity_workflow import retained_projects
    if len(retained_projects())>=8:
        from offline_audio import process_offline
        return process_offline(recipe,directory)
    return legacy(BASE/"audio/scripts/audacity_process.py",["--recipe",recipe_path],directory,label,)


def probe_game(files,directory):
    directory=Path(directory);directory.mkdir(parents=True,exist_ok=True)
    compiler=REPO/"tools/w64devkit/bin/g++.exe"
    raylib=REPO/"external/raylib-5.5_win64_mingw-w64"
    exe=directory/"asset_probe.exe"
    run([compiler,REPO/"tools/media/asset_probe.cpp","-std=c++17","-O2","-static","-I"+str(raylib/"include"),
         "-L"+str(raylib/"lib"),"-lraylib","-lopengl32","-lgdi32","-lwinmm","-o",exe],directory,"probe-compile",extra_path=compiler.parent)
    return json.loads(run([exe,"--no-audio",*files],directory,"raylib-probe"))


def demo(directory,name):
    from create_krita_sprite import create_sprite
    from audio_tools import deliver_music
    directory=Path(directory);directory.mkdir(parents=True,exist_ok=False)
    steps={}
    # Snapshot existing runtime files; the test must not alter the game build.
    protected=[REPO/"dist/Emberfall.exe",*sorted((REPO/"assets/audio").glob("*"))]
    protected={str(p):sha(p) for p in protected if p.is_file()}
    write_json(directory/"protected-files.json",protected)
    try:
        print("Checking all six tools...",flush=True)
        steps["environment"]=doctor(directory/"doctor")
        if steps["environment"]["status"]!="PASS":raise RuntimeError("A tool failed its live environment check.")
        print("Drawing layered sprite in Krita MCP...",flush=True)
        steps["sprite"]=create_sprite(name,directory/"krita")
        print("Generating Laigter maps and baking light...",flush=True)
        maps=generate_maps(steps["sprite"]["png"],directory/"maps")
        steps["maps"]=json.loads(maps.read_text(encoding="utf-8-sig"))
        steps["lighting"]=bake_lighting(steps["sprite"]["png"],maps,BASE/"sprites"/(name+"_lit.png"))
        print("Rendering Synfig clip and packing atlas...",flush=True)
        steps["animation"]=animate(steps["lighting"]["path"],BASE/"animations"/(name+".sif"),directory/"animation")
        print("Composing music in LMMS...",flush=True)
        steps["music_source"]=compose_music(name+"_music",directory/"music")
        print("Synthesizing three Csound effects...",flush=True)
        steps["sfx_sources"]=generate_sfx(name,directory/"sfx")
        print("Editing, mixing and normalizing with Audacity...",flush=True)
        music_edit=directory/"music/processed.wav"
        steps["music_edit"]=process_audio({"inputs":[steps["music_source"]["wav"]],"output":str(music_edit),
             "trim":{"start":10,"end":20},"normalize_db":-3,"fade_in":0,"fade_out":0},directory/"audio","audacity-music")
        steps["music_delivery"]=deliver_music(music_edit,directory/"music/crystal_sanctum_loop.wav")
        steps["sfx_edits"]={}
        for preset,result in steps["sfx_sources"].items():
            output=directory/"sfx"/(preset+"_processed.wav")
            recipe={"inputs":[result["wav"]],"output":str(output),"normalize_db":-2,"fade_in":.005,"fade_out":.04}
            if preset=="electric":recipe["effects"]=[{"command":"Echo","params":{"Delay":.035,"Decay":.14}}]
            steps["sfx_edits"][preset]=process_audio(recipe,directory/"audio","audacity-"+preset)
        files=[]
        def add(source,path,role):files.append({"source":str(source),"path":path,"role":role,"sha256":sha(source)})
        add(steps["sprite"]["png"],"sprites/crystal.png","diffuse")
        add(steps["lighting"]["path"],"sprites/crystal_lit.png","baked_material")
        add(directory/"animation/crystal_atlas.png","sprites/crystal_atlas.png","animation_atlas")
        add(directory/"animation/crystal_atlas.json","sprites/crystal_atlas.json","animation_metadata")
        add(steps["music_delivery"]["wav"],"audio/crystal_sanctum_loop.wav","music_master")
        add(steps["music_delivery"]["ogg"],"audio/crystal_sanctum_loop.ogg","music_stream")
        bindings={"music":steps["music_delivery"]["ogg"]}
        for preset,cue in (("electric","spell"),("weapon_hit","swing"),("loot","pickup")):
            output=steps["sfx_edits"][preset]["output"]
            add(output,"audio/"+cue+".wav","sfx_"+cue);bindings[cue]=output
        event_map={"known_cues":cue_names(),"bindings":bindings,"deployment":"Use bind-audio after approving these sounds for production."}
        write_json(directory/"events.json",event_map);add(directory/"events.json","audio/events.json","event_mapping")
        manifest={"schema":1,"id":name,"files":files,"provenance":{"kind":"project-authored test assets",
            "external_samples":[],"paid_services":False,"graphics":"native Krita MCP","music":"LMMS built-in synthesis; user template szablon.mmp",
            "sfx":"Csound original procedural patches"}}
        write_json(directory/"pack.json",manifest)
        print("Importing pack and checking exact raylib decoders...",flush=True)
        steps["import"]=import_pack(directory/"pack.json")
        return finish_integration(steps,directory,name,bindings,protected)
    except BaseException as error:
        write_json(directory/"failure.json",{"status":"FAIL","error":str(error),"completed_steps":steps,"audio_played":False})
        raise


def finish_integration(steps,directory,name,bindings,protected):
    for item in steps["import"]["files"]:
        if sha(item["path"])!=item["sha256"]:raise ValueError("Imported asset changed since production.")
    imported=[x["path"] for x in steps["import"]["files"] if Path(x["path"]).suffix in (".png",".wav",".ogg")]
    steps["raylib"]=probe_game(imported,directory/("raylib_"+uuid.uuid4().hex[:6]))
    fixture=REPO/"art/pipeline/qa"/(name+"_"+uuid.uuid4().hex[:6])/"assets"
    (fixture/"audio").mkdir(parents=True,exist_ok=False)
    fixture_before={}
    for cue in bindings:
        filename="echoes-of-the-ruins.ogg" if cue=="music" else cue+".wav"
        source=REPO/"assets/audio"/filename
        if source.exists():
            shutil.copy2(source,fixture/"audio"/filename);fixture_before[filename]=sha(source)
    steps["event_binding"]=bind_audio(bindings,asset_root=fixture)
    steps["event_restore"]=restore_bindings(steps["event_binding"]["receipt"])
    assert all(sha(fixture/"audio"/filename)==digest for filename,digest in fixture_before.items())
    assert all(sha(path)==digest for path,digest in protected.items())
    steps["protected_files_unchanged"]={"status":"PASS","files":protected}
    report={"status":"PASS","run_directory":str(directory),"name":name,"steps":steps,
            "audio_played":False,"game_rebuilt":False,
            "limits":["The existing game has no runtime material-map shader: maps are baked offline.",
                      "The atlas adapter is ready; this test does not replace procedural actors or rebuild the game.",
                      "Event deployment was tested in an isolated fixture; production sounds stay unchanged.",
                      "Audacity saved projects are retained to avoid a native project-close crash; excess work uses FFmpeg.",
                      "Technical audio checks were silent; no artistic listening assessment was performed."]}
    write_json(directory/"report.json",report)
    print(json.dumps({"status":"PASS","report":str(directory/"report.json"),"pack":steps["import"]["destination"]}),flush=True)
    return report


def resume_integration(directory):
    directory=Path(directory).resolve()
    if (directory/"report.json").exists():raise FileExistsError("A successful report already exists.")
    failed=json.loads((directory/"failure.json").read_text(encoding="utf-8"))
    steps=failed["completed_steps"]
    for key in ("import","music_delivery","sfx_edits","animation","sprite"):
        if key not in steps:raise ValueError("This resume action requires a completed production/import stage.")
    bindings=json.loads((directory/"events.json").read_text(encoding="utf-8"))["bindings"]
    snapshot=directory/"protected-files.json"
    if snapshot.exists():protected=json.loads(snapshot.read_text(encoding="utf-8"))
    else:
        paths=[REPO/"dist/Emberfall.exe",*sorted((REPO/"assets/audio").glob("*"))]
        protected={str(p):sha(p) for p in paths if p.is_file()}
        write_json(directory/"protected-files-at-integration-resume.json",protected)
    return finish_integration(steps,directory,directory.name,bindings,protected)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--no-audio",action="store_true",required=True,help="Required; all jobs remain silent.")
    sub=parser.add_subparsers(dest="action",required=True)
    for name in ("doctor","demo"):
        p=sub.add_parser(name);p.add_argument("--directory",type=Path)
    p=sub.add_parser("maps");p.add_argument("sprite",type=Path)
    p=sub.add_parser("animate");p.add_argument("sprite",type=Path);p.add_argument("project",type=Path)
    p=sub.add_parser("music");p.add_argument("name")
    p=sub.add_parser("sfx");p.add_argument("name")
    p=sub.add_parser("process-audio");p.add_argument("recipe",type=Path)
    p=sub.add_parser("import-pack");p.add_argument("manifest",type=Path)
    p=sub.add_parser("bind-audio");p.add_argument("mapping",type=Path)
    p=sub.add_parser("restore-audio");p.add_argument("receipt",type=Path)
    p=sub.add_parser("loop-music");p.add_argument("input",type=Path);p.add_argument("output",type=Path)
    p=sub.add_parser("resume-integration");p.add_argument("run_directory",type=Path)
    args=parser.parse_args()
    identifier="media_"+datetime.datetime.now().strftime("%Y%m%d_%H%M%S_")+uuid.uuid4().hex[:6]
    directory=getattr(args,"directory",None) or BASE/"pipeline/runs"/identifier
    if args.action=="demo":demo(directory,identifier);return
    if args.action=="resume-integration":resume_integration(args.run_directory);return
    Path(directory).mkdir(parents=True,exist_ok=False)
    if args.action=="doctor":result=doctor(directory)
    elif args.action=="maps":result={"manifest":str(generate_maps(args.sprite,directory))}
    elif args.action=="animate":result=animate(args.sprite,args.project,directory)
    elif args.action=="music":result=compose_music(args.name,directory)
    elif args.action=="sfx":result=generate_sfx(args.name,directory)
    elif args.action=="process-audio":result=process_audio(json.loads(args.recipe.read_text(encoding="utf-8-sig")),directory,"audacity")
    elif args.action=="import-pack":result=import_pack(args.manifest)
    elif args.action=="bind-audio":
        data=json.loads(args.mapping.read_text(encoding="utf-8-sig"));result=bind_audio(data.get("bindings",data))
    elif args.action=="restore-audio":result=restore_bindings(args.receipt)
    elif args.action=="loop-music":
        from audio_tools import deliver_music
        result=deliver_music(args.input,args.output)
    write_json(Path(directory)/"result.json",result)
    print(json.dumps(result,indent=2))
    if result.get("status")=="FAIL":raise SystemExit(1)


if __name__=="__main__":main()
