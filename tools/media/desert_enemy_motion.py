"""Author and export the native Synfig curves for the two desert enemies."""

import argparse
from pathlib import Path
import shutil
from xml.etree import ElementTree as ET

from scipy.interpolate import CubicSpline, PchipInterpolator

from fire_imp_synfig_rig import canvas, save
from wolf_motion_synfig import control
import enemy_death_synfig
import synfig_cli


TRACKS = {
    "jackal_stride": ((0,1),(38,-1),(41,-1.04),(47,-.34),(52,.50),(57,1.03),(60,1)),
    "jackal_lift": ((0,0),(38,0),(41,.12),(45,.64),(49,1),(52,.89),(56,.34),(59,.026),(60,0)),
    "reaper_stride": ((0,1),(38,-1),(41,-1.07),(49,-.03),(55,.86),(57,1.08),(60,1)),
    "reaper_lift": ((0,0),(38,0),(41,.27),(46,.94),(50,1),(55,.47),(58,.09),(60,0)),
    "jackal_crouch": ((0,0),(8,.10),(15,.47),(20,.88),(22,1),(25,.60),(28,.11),(32,.09),(36,.54),(40,.61),(46,.26),(54,.025),(60,0)),
    "jackal_pounce": ((0,0),(9,-.04),(16,-.17),(21,-.29),(23,-.24),(25,.16),(28,.91),(30,1),(35,.74),(41,.35),(49,.08),(60,0)),
    "jackal_neck": ((0,0),(8,.06),(16,.26),(22,.50),(24,.31),(27,-.71),(29,-1),(33,-.67),(37,-.05),(41,.26),(47,.12),(56,-.02),(60,0)),
    "jackal_land": ((0,0),(21,0),(24,.13),(27,.72),(30,1),(33,.77),(37,.30),(42,0),(60,0)),
    "reaper_crouch": ((0,0),(8,.10),(16,.45),(22,1),(24,.92),(27,.52),(34,.83),(40,.74),(51,.14),(60,0)),
    "reaper_sickle": ((0,0),(7,-.12),(14,-.51),(21,-.96),(23,-1),(25,-.53),(27,.42),(30,.99),(35,1),(42,.72),(50,.28),(60,0)),
    "reaper_elbow": ((0,0),(9,.16),(17,.72),(23,1),(26,.44),(29,-.40),(33,-.28),(40,.25),(50,.29),(60,0)),
    "reaper_turn": ((0,0),(8,-.09),(15,-.47),(21,-.86),(23,-.72),(26,.27),(29,1),(34,.91),(41,.58),(52,.08),(60,0)),
    "jackal_fold": ((0,0),(6,.04),(13,.23),(22,.53),(34,.77),(46,.94),(57,1),(60,1)),
    "jackal_drop": ((0,0),(8,.06),(16,.21),(24,.42),(35,.76),(47,.96),(57,1),(60,1)),
    "jackal_roll": ((0,0),(6,-.035),(17,.10),(27,.38),(40,.87),(51,.98),(58,1),(60,1)),
    "jackal_buckle": ((0,0),(6,.07),(12,.56),(20,1),(30,.77),(42,.25),(54,0),(60,0)),
    "reaper_fold": ((0,0),(7,.02),(15,.17),(25,.48),(38,.76),(49,.96),(57,1),(60,1)),
    "reaper_drop": ((0,0),(8,.03),(16,.17),(26,.44),(33,.57),(44,.88),(52,.98),(58,1),(60,1)),
    "reaper_roll": ((0,0),(8,-.035),(17,-.02),(25,.13),(34,.42),(47,.94),(53,1.01),(60,1)),
    "reaper_buckle": ((0,0),(7,.06),(15,.32),(24,.95),(32,1),(42,.65),(51,.19),(58,0),(60,0)),
    "jackal_weight": ((0,.32),(7,.62),(14,.82),(23,.60),(30,.32),(37,.62),(44,.82),(53,.60),(60,.32)),
    "jackal_balance": ((0,0),(7,.36),(15,.10),(23,-.36),(30,0),(37,.36),(45,.10),(53,-.36),(60,0)),
    "jackal_bite": ((0,0),(18,0),(22,.26),(25,.86),(26,1),(28,.38),(29,.05),(33,0),(60,0)),
    "jackal_head_walk": ((0,.10),(6,-.27),(12,-.56),(18,-.18),(24,.43),(30,.10),
                        (36,-.27),(42,-.56),(48,-.18),(54,.43),(60,.10)),
    "jackal_tail_walk": ((0,-.46),(7,-.88),(14,-.48),(22,.44),(30,.9),(38,.58),
                        (46,-.08),(53,-.61),(60,-.46)),
    "jackal_fore_flex": ((0,0),(38,0),(40,.14),(44,.83),(47,1),(51,.60),(55,.21),(60,0)),
    "jackal_hind_flex": ((0,0),(38,0),(41,.30),(45,.90),(49,1),(53,.66),(57,.18),(60,0)),
}


def jackal_stride(phase):
    support = 38 / 60
    tangent = -2 * (1-support) / support
    ramp = .24
    cruise = (2-ramp*tangent) / (1-ramp)

    def swing_start(t):
        u = t/ramp
        return -1 + tangent*t + (cruise-tangent)*ramp*u**3*(1-.5*u)

    if phase <= support:
        return 1-2*phase/support
    swing = (phase-support)/(1-support)
    if swing < ramp:
        return swing_start(swing)
    if swing > 1-ramp:
        return -swing_start(1-swing)
    return swing_start(ramp)+cruise*(swing-ramp)


def author(path):
    project = canvas("Pustynia", 400, 570, (135, 14), 60)
    project.find("name").text = "Emberfall / Pustynia / krzywe postaci"
    for index, (name, keys) in enumerate(TRACKS.items()):
        x,y = zip(*keys)
        cyclic = name in ("jackal_weight","jackal_balance","jackal_head_walk","jackal_tail_walk")
        evaluate = CubicSpline(x,y,bc_type="periodic") if cyclic else PchipInterpolator(x,y)
        support = x[1] if name.endswith("_stride") else 0
        if name == "jackal_stride":
            values = [(100*jackal_stride(frame/60), -18*index) for frame in range(61)]
        else:
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
    parser.add_argument("--only-regional",action="store_true")
    parser.add_argument("--no-audio",action="store_true",required=True)
    args = parser.parse_args()
    art = Path("art/desert/new-enemies")
    folder = args.root
    folder.mkdir(parents=True,exist_ok=True)
    motion = folder/"ruch-pustynnych-bestii.sif"
    author(motion)
    shutil.copyfile(motion, art/motion.name)
    export(motion,Path("src/generated/desert_enemy_motion.hpp"))
    if not args.only_regional:
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
