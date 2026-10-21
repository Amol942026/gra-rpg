#include "../src/hero_motion.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
using namespace ember;
inline void check(bool ok,const char* label){if(!ok)throw std::runtime_error(label);}
#include "walk_tests.hpp"
int main(int argc,char** argv){
 if(argc!=3||std::string(argv[1])!="--no-audio")return 3;
 walkTests();
 using namespace ember::hero_motion;std::filesystem::path root=argv[2];std::filesystem::create_directories(root);
 std::ofstream csv(root/"runtime-controls.csv");csv<<"frame,hip_r,hip_z,l_foot_r,l_foot_f,l_foot_z,r_foot_r,r_foot_f,r_foot_z,l_hand_r,l_hand_f,l_hand_z,r_hand_r,r_hand_f,r_hand_z\n";
 for(int f=0;f<=60;f++){
  auto p=evaluate(f/60.f,1,0,0);csv<<f<<','<<p.world[data::Pelvis].p.x<<','<<p.world[data::Pelvis].p.z;
  for(int b:{data::FootL,data::FootR,data::HandL,data::HandR}){auto v=p.world[b].p;csv<<','<<v.x<<','<<v.y<<','<<v.z;}csv<<'\n';
 }
 float low[2]={100,100},highSupport[2]={-100,-100},kneeWidth[2]={0,0},chestYaw[2]={0,0};
 for(int mode=0;mode<2;mode++)for(int f=0;f<600;f++){
  float t=f/600.f;auto p=mode?evaluate(t,1,0,0):forward(sample(data::WalkClip,t));
  if(!mode)secondary(p,ember::hero_secondary::evaluate(t,1,0,0));
  float l=walk_checks::soleHeight(p,data::FootL),r=walk_checks::soleHeight(p,data::FootR);
  low[mode]=std::min({low[mode],l,r});highSupport[mode]=std::max(highSupport[mode],std::min(l,r));
  kneeWidth[mode]=std::max(kneeWidth[mode],std::abs(p.world[data::ShinL].p.x-p.world[data::ShinR].p.x));
  auto front=p.vector(data::Spine,{0,1,0});chestYaw[mode]=std::max(chestYaw[mode],std::abs(std::atan2(front.x,front.y)*180/3.14159265f));
 }
 std::ofstream out(root/"metrics.json");out<<"{\"before\":{\"lowest_boot\":"<<low[0]<<",\"highest_support\":"<<highSupport[0]<<",\"knee_width\":"<<kneeWidth[0]<<",\"chest_yaw\":"<<chestYaw[0]<<"},\"after\":{\"lowest_boot\":"<<low[1]<<",\"highest_support\":"<<highSupport[1]<<",\"knee_width\":"<<kneeWidth[1]<<",\"chest_yaw\":"<<chestYaw[1]<<"},\"audio\":false}\n";
 std::cout<<"Boot floor before/after: "<<low[0]<<"/"<<low[1]<<"; support height before/after: "<<highSupport[0]<<"/"<<highSupport[1]<<"; chest yaw before/after: "<<chestYaw[0]<<"/"<<chestYaw[1]<<'\n';
 return low[1]<-.04f||highSupport[1]>.04f?2:0;
}
