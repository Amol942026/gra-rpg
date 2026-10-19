"""Validated, reversible file integration with Emberfall's existing asset paths."""
from pathlib import Path
import datetime, json, os, re, shutil, subprocess, uuid, wave
from asset_tools import sha,write_json

REPO=Path(__file__).resolve().parents[2]


def validate_audio(path):
    """Read local PCM or decode the complete Vorbis file before deployment."""
    path=Path(path).resolve()
    if not (path.is_relative_to(Path(r"C:\AI2D")) or path.is_relative_to(REPO)):
        raise ValueError("Audio source outside production roots.")
    if path.suffix.lower()==".wav":
        with wave.open(str(path),"rb") as stream:
            if stream.getnchannels()!=2 or stream.getframerate()!=48000 or stream.getsampwidth()!=2 or stream.getnframes()<=0:
                raise ValueError("Game delivery must be nonempty stereo PCM16 / 48 kHz.")
            expected=stream.getnframes()*4
            if len(stream.readframes(stream.getnframes()))!=expected:raise ValueError("Truncated PCM data.")
    elif path.suffix.lower()==".ogg":
        ffmpeg=Path(r"C:\Program Files\Krita (x64)\bin\ffmpeg.exe")
        ffprobe=ffmpeg.with_name("ffprobe.exe")
        probe=subprocess.run([str(ffprobe),"-v","error","-show_entries","stream=codec_name,sample_rate,channels",
                             "-of","json",str(path)],capture_output=True,timeout=30,creationflags=subprocess.CREATE_NO_WINDOW)
        if probe.returncode:raise ValueError("Cannot inspect OGG delivery.")
        tracks=json.loads(probe.stdout)["streams"]
        if len(tracks)!=1 or tracks[0].get("codec_name")!="vorbis" or tracks[0].get("sample_rate")!="48000" or tracks[0].get("channels")!=2:
            raise ValueError("Game music must be a stereo / 48 kHz Vorbis stream.")
        decode=subprocess.run([str(ffmpeg),"-nostdin","-v","error","-xerror","-i",str(path),"-map","0:a:0","-f","null","-"],
                              capture_output=True,timeout=120,creationflags=subprocess.CREATE_NO_WINDOW)
        if decode.returncode:raise ValueError("OGG contains undecodable audio packets.")
    else:raise ValueError("Unsupported game audio file.")
    return True


def cue_names():
    source=(REPO/"src/audio.hpp").read_text(encoding="utf-8")
    match=re.search(r"Names\[CueCount\]\s*=\s*\{(.*?)\};",source,re.S)
    if not match:raise ValueError("Cannot read actual game's cue mapping.")
    return re.findall(r'"([a-z-]+)"',match.group(1))


def import_pack(manifest_path, *, destination=None):
    manifest_path=Path(manifest_path).resolve()
    manifest=json.loads(manifest_path.read_text(encoding="utf-8-sig"))
    identifier=manifest["id"]
    if not re.fullmatch(r"[a-zA-Z0-9_-]{1,80}",identifier):raise ValueError("Invalid asset pack ID.")
    destination=Path(destination or REPO/"assets/media"/identifier).resolve()
    if not destination.is_relative_to(REPO):raise ValueError("Import must stay in the game repository.")
    if destination.exists():raise FileExistsError(destination)
    checked=[];seen=set()
    for item in manifest["files"]:
        rel=Path(item["path"])
        if rel.is_absolute() or ".." in rel.parts or rel.suffix.lower() not in (".png",".wav",".ogg",".json"):
            raise ValueError("Unsafe or unsupported runtime asset path.")
        target=(destination/rel).resolve()
        if not target.is_relative_to(destination) or str(target).lower() in seen:raise ValueError("Duplicate/escaping target.")
        seen.add(str(target).lower())
        source=Path(item["source"]).resolve()
        if not (source.is_relative_to(Path(r"C:\AI2D")) or source.is_relative_to(REPO)):
            raise ValueError("Source outside production roots.")
        if sha(source)!=item["sha256"]:raise ValueError("Asset hash does not match manifest.")
        checked.append((source,target,item))
    destination.mkdir(parents=True,exist_ok=False)
    copied=[]
    for source,target,item in checked:
        target.parent.mkdir(parents=True,exist_ok=True)
        with source.open("rb") as incoming,target.open("xb") as outgoing:shutil.copyfileobj(incoming,outgoing)
        if sha(target)!=item["sha256"]:raise IOError("Imported file hash mismatch.")
        copied.append({"path":str(target),"sha256":sha(target),"bytes":target.stat().st_size,"role":item["role"]})
    receipt={"status":"PASS","source_manifest":str(manifest_path),"destination":str(destination),"files":copied,
             "provenance":manifest.get("provenance",{}),
             "game_rebuilt":False,"runtime_audio_bindings_changed":False}
    write_json(destination/"import-receipt.json",receipt)
    return receipt


def bind_audio(mapping, *, asset_root=None):
    """Map Cue IDs to known filenames; back up replacements before atomic rename.

    By default writes the game's source assets. An isolated in-repo fixture can
    be supplied for acceptance testing. No new sound-event IDs are invented.
    """
    root=Path(asset_root or REPO/"assets").resolve()
    if not root.is_relative_to(REPO):raise ValueError("Destination must stay in this game repository.")
    available=set(cue_names())|{"music"}
    jobs=[]
    for cue,path in mapping.items():
        if cue not in available:raise ValueError("Unknown game audio event: "+cue)
        source=Path(path).resolve()
        extension=".ogg" if cue=="music" else ".wav"
        if source.suffix.lower()!=extension:raise ValueError("Incorrect format for event "+cue)
        target=(root/"audio"/("echoes-of-the-ruins.ogg" if cue=="music" else cue+".wav")).resolve()
        if not target.is_relative_to(root):raise ValueError("Audio destination escapes through a link.")
        if source==target:raise ValueError("Source cannot be a binding target.")
        validate_audio(source)
        jobs.append((cue,source,target,sha(source)))
    batch=datetime.datetime.now().strftime("%Y%m%d_%H%M%S_")+uuid.uuid4().hex[:8]
    backup=REPO/"art/pipeline/backups"/batch
    backup.mkdir(parents=True,exist_ok=False)
    records=[]
    try:
        for cue,source,target,digest in jobs:
            target.parent.mkdir(parents=True,exist_ok=True)
            previous=None;before=None
            if target.exists():
                previous=backup/target.name
                shutil.copy2(target,previous)
                before=sha(target)
                if sha(previous)!=before:raise IOError("Backup hash mismatch.")
            temporary=target.with_name(target.name+"."+batch+".tmp")
            with source.open("rb") as incoming,temporary.open("xb") as outgoing:shutil.copyfileobj(incoming,outgoing)
            if sha(temporary)!=digest:raise IOError("Binding hash mismatch.")
            os.replace(temporary,target)
            records.append({"event":cue,"target":str(target),"source":str(source),"sha256":digest,
                            "backup":str(previous) if previous else None,"before_sha256":before})
    finally:
        write_json(backup/"binding-receipt.json",{"records":records,"asset_root":str(root)})
    return {"status":"PASS","records":records,"receipt":str(backup/"binding-receipt.json"),"asset_root":str(root)}


def restore_bindings(receipt):
    receipt=Path(receipt).resolve()
    if not receipt.is_relative_to((REPO/"art/pipeline/backups").resolve()):raise ValueError("Invalid backup receipt location.")
    data=json.loads(receipt.read_text(encoding="utf-8"))
    planned=[]
    for item in data["records"]:
        target=Path(item["target"]).resolve()
        if not target.is_relative_to(REPO):raise ValueError("Invalid restore target.")
        if sha(target)!=item["sha256"]:raise ValueError("Target changed after binding; refusing restore.")
        if item["backup"] is None:
            # Preserve the new file rather than deleting it.
            moved=receipt.parent/("removed_binding_"+target.name)
            if moved.exists():raise FileExistsError(moved)
            planned.append((target,moved,None))
        else:
            backup=Path(item["backup"]).resolve()
            if backup.parent!=receipt.parent:raise ValueError("Backup must stay beside its receipt.")
            if sha(backup)!=item["before_sha256"]:raise ValueError("Damaged backup.")
            planned.append((target,backup,item["before_sha256"]))
    for target,source,digest in planned:
        if digest is None:target.rename(source)
        else:
            temporary=target.with_name(target.name+".restore_"+uuid.uuid4().hex+".tmp")
            shutil.copy2(source,temporary)
            if sha(temporary)!=digest:raise IOError("Restore hash mismatch.")
            os.replace(temporary,target)
    return {"status":"PASS","restored":len(data["records"])}
