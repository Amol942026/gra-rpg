#include "../../tests/sword_stow_clearance.hpp"
#include <iostream>
int main(int,char**){
 using namespace ember::hero_motion;
 for(int f=0;f<=36;f++){
  float phase=f/36.f;auto p=evaluate(.1f,1,phase,0);auto k=ember::hero_sheath::apply(p,phase);auto c=stow_clearance::measure(p,k);
  auto h=p.world[data::Weapon].p;auto v=ember::sword_pose::idleBlade(0);auto b=p.vector(data::Weapon,{v.r,v.f,v.z});
  std::cout<<f<<','<<c.head<<','<<c.torso<<','<<h.x<<','<<h.y<<','<<h.z<<','<<b.x<<','<<b.y<<','<<b.z<<'\n';
 }
}
