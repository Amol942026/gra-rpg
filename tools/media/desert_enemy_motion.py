"""Author and export the native Synfig curves for the two desert enemies."""

import argparse
from pathlib import Path
from xml.etree import ElementTree as ET

from scipy.interpolate import PchipInterpolator

from fire_imp_synfig_rig import canvas, save
from wolf_motion_synfig import control
import enemy_death_synfig
import synfig_cli


TRACKS = {
    "jackal_stride": ((0,1),(35,-1),(40,-.94),(46,-.28),(53,.74),(58,1.04),(60,1)),
    "jackal_lift": ((0,0),(35,0),(40,.33),(46,.98),(50,.91),(56,.30),(60,0)),
    "reaper_stride": ((0,1),(37,-1),(42,-.9),(48,-.24),(55,.79),(58,1.04),(60,1)),
    "reaper_lift": ((0,0),(37,0),(43,.61),(48,1),(55,.40),(60,0)),
    "jackal_crouch": ((0,0),(8,.12),(15,.56),(21,1),(24,.72),(27,.11),(34,.18),(39,.41),(49,.04),(60,0)),
    "jackal_pounce": ((0,0),(9,-.06),(17,-.19),(21,-.31),(24,.27),(27,1),(31,.82),(37,.54),(47,.14),(60,0)),
    "jackal_neck": ((0,0),(10,.19),(20,.62),(24,.19),(28,-1),(32,-.53),(38,.41),(43,.21),(51,-.07),(60,0)),
    "reaper_crouch": ((0,0),(12,.23),(20,.78),(23,1),(28,.42),(35,.57),(43,.25),(60,0)),
    "reaper_sickle": ((0,0),(8,-.14),(15,-.56),(22,-1),(24,-.92),(27,.48),(30,.94),(34,1),(42,.55),(53,.08),(60,0)),
    "reaper_elbow": ((0,0),(9,.16),(19,.74),(23,1),(26,.48),(29,-.25),(35,-.09),(43,.38),(52,.21),(60,0)),
    "reaper_turn": ((0,0),(9,-.19),(19,-.65),(23,-.79),(26,.22),(29,.91),(33,1),(41,.57),(53,.10),(60,0)),
    "jackal_fold": ((0,0),(7,.04),(14,.31),(23,.73),(36,1),(60,1)),
    "jackal_drop": ((0,0),(6,-.06),(15,.13),(28,.69),(39,.98),(46,.95),(55,1),(60,1)),
    "reaper_fold": ((0,0),(7,.04),(15,.40),(25,.77),(39,.94),(50,1),(60,1)),
    "reaper_drop": ((0,0),(7,-.04),(15,.11),(25,.32),(38,.83),(46,.98),(53,1),(60,1)),
    "jackal_roll": ((0,0),(8,-.08),(24,.56),(36,1),(44,.83),(54,.88),(60,.88)),
    "reaper_roll": ((0,0),(7,-.04),(18,.12),(30,.44),(43,.92),(49,1.04),(57,1),(60,1)),
}


def author(path):
    project = canvas("Pustynia", 400, 325, (135, 14), 60)
    project.find("name").text = "Emberfall / Pustynia / krzywe postaci"
    for index, (name, keys) in enumerate(TRACKS.items()):
        x,y = zip(*keys)
        evaluate = PchipInterpolator(x,y)
        support = x[1] if name.endswith("_stride") else 0
        values = [(100*(1-2*frame/support if frame<support else float(evaluate(frame))), -18*index)
                  for frame in range(61)]
        control(project, name, values, 4.2, (.54+.03*(index%3), .68, .48+.08*(index%5)))
    save(project,path)


def export(path, header):
    lines = ["#pragma once", "// Sampled from the native Synfig motion tracks.",
             "namespace ember::desert_enemy_motion {"]
    for layer in ET.parse(path).findall("layer"):
        keys = layer.findall("param[@name='origin']/animated/waypoint/vector/x")
        if len(keys)!=61:
            raise ValueError("Expected 61 samples in each Synfig trajectory")
        data = [f"{float(key.text)/100:.5f}f" for key in keys]
        rows = [" "+",".join(data[i:i+10])+"," for i in range(0,61,10)]
        lines.extend([f"inline constexpr float {layer.get('desc')}[61]={{",*rows,"};"])
    header.write_text("\n".join([*lines,"}",""]),encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root",type=Path,required=True)
    parser.add_argument("--render",action="store_true")
    parser.add_argument("--no-audio",action="store_true",required=True)
    args = parser.parse_args()
    art = Path("art/desert/new-enemies")
    motion = art/"ruch-pustynnych-bestii.sif"
    author(motion)
    export(motion,Path("src/generated/desert_enemy_motion.hpp"))
    folder = args.root
    folder.mkdir(parents=True,exist_ok=True)
    master = folder/"upadki-przeciwnikow.sif"
    enemy_death_synfig.author(master)
    enemy_death_synfig.export(master,Path("src/generated/enemy_death_data.hpp"))
    if args.render:
        import json
        receipt = synfig_cli.render(motion, output_root=folder/"ruch-synfig", threads=2,
                                     expected_frames=61, require_motion=True)
        (folder/"synfig-receipt.json").write_text(json.dumps(receipt,indent=2),encoding="utf-8")


if __name__ == "__main__":
    main()
