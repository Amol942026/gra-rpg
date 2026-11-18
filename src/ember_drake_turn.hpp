#pragma once
// Drake-specific access for the animated muzzle and its renderer probes.
namespace drake_turn {
namespace sheet=::ember_drake_turn_atlas;
using turn_art::filtered;
using turn_art::mix;
inline bool ready(){return turn_art::drake.ready();}
inline turn_art::View view(const AnimationState& state){return turn_art::view(state,turn_art::drake);}
}
