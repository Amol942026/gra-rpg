#pragma once
#include "enemy_death_motion.hpp"

namespace ember::enemy_death {
inline float landingFit(int type,float progress){
 const float contact=groundContact(type);
 // Start folding the skeleton's long, thin limbs earlier in its short fall.
 const float lead=type==2?.32f:.24f;
 return window(progress,contact-lead,contact);
}
inline float restingBlend(int type,float progress){
 const float contact=groundContact(type);
 // Complete the dissolve while the rig is falling. The impact frame uses
 // the same sprite, transform and silhouette as the persistent corpse.
 const float lead=type==2?.30f:type==CinderhornType?.28f:.20f;
 return window(progress,contact-lead,contact);
}
}
