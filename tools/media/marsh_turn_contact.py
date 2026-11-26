"""Compile the local CPU contact baker and verify the silhouettes used in game."""
import argparse
import hashlib
import json
import os
import shutil
import subprocess
from pathlib import Path


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def bake(root,stage):
    root,stage=root.resolve(),stage.resolve()
    out=root/"assets/media/mokradla-ruch"
    stage.mkdir(parents=True,exist_ok=True)
    src=root/"tools/media/marsh_turn_contact.cpp"
    ray=root/"external/raylib-5.5_win64_mingw-w64"
    cxx=root/"tools/w64devkit/bin/g++.exe"
    header=root/"src/generated/marsh_turn_frames.hpp"
    dependencies=(src,header,root/"src/entity_info.hpp",root/"src/generated/marsh_actor_frames.hpp",ray/"lib/libraylib.a")
    stamp=stage/"contact-build-sha.json"
    inputs={file.relative_to(root).as_posix():sha(file) for file in dependencies}
    exe=stage/"turn-contact.exe"
    env=dict(os.environ,PATH=str(cxx.parent)+os.pathsep+os.environ["PATH"])
    options=dict(env=env,check=True,creationflags=subprocess.CREATE_NO_WINDOW,cwd=root)
    if not (exe.is_file() and stamp.is_file() and json.loads(stamp.read_text())==inputs):
        subprocess.run([str(cxx),str(src),"-std=c++17","-O3","-I"+str(ray/"include"),
                        "-L"+str(ray/"lib"),"-lraylib","-lopengl32","-lgdi32","-lwinmm",
                        "-static","-o",str(exe)],**options)
        stamp.write_text(json.dumps(inputs,indent=2),encoding="utf-8")
    warp_manifest=json.loads((out/"turn-warp-receipt.json").read_text(encoding="utf-8"))
    for item in warp_manifest["files"]+warp_manifest["source_atlases"]:
        if sha(out/item["file"])!=item["sha256"]:
            raise ValueError("Unexpected painted atlas or warp: "+item["file"])
    temporary=stage/"turn-contact.baking"
    subprocess.run([str(exe),"--no-audio",str(out),str(temporary)],**options)
    pixels=temporary.read_bytes()
    if len(pixels)!=4+4*8*64*16*7 or pixels[:4]!=b"MWC1":
        raise ValueError("Incomplete ground contact bake")
    if any(pixels[4+i*7] for i in range(0,4*8*64*16,16)):
        raise ValueError("A cardinal endpoint was translated")
    destinations={}
    output=out/"turn-contact.bin"
    destinations[output]=pixels
    summary={"format":"MWC1", "angles_per_sector":16,"species":4,"sectors":8,"poses":64,
             "payload":"int8 dy, left x/y/radius, right x/y/radius, in local pixels after correction",
             "result":{"file":output.name,"bytes":len(pixels),"sha256":hashlib.sha256(pixels).hexdigest()},
             "atlases":warp_manifest["source_atlases"],"warps":warp_manifest["files"],
             "warp_manifest_sha256":sha(out/"turn-warp-receipt.json"),
             "build_sources":inputs,"driver_sha256":sha(Path(__file__))}
    destinations[out/"turn-contact-receipt.json"]=(json.dumps(summary,indent=2)+"\n").encode()
    for path,data in destinations.items():
        if path.is_file():
            if data==path.read_bytes():
                continue
            backup=stage/(path.name+"."+sha(path)[:12]+".before")
            if not backup.exists():
                shutil.copy2(path,backup)
        # Use the destination directory so the rename is on the same volume.
        pending=path.with_name(path.name+".pending")
        pending.write_bytes(data);pending.replace(path)
    print(summary["result"])


if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root",required=True,type=Path)
    parser.add_argument("--stage",required=True,type=Path)
    args=parser.parse_args()
    bake(args.root,args.stage)
