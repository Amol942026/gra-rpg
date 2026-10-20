#include "sword_stow_clearance.hpp"
#include "../src/animation.hpp"
#include <iostream>
#include <fstream>
int main(int argc,char** argv){
 if(argc<2||std::string(argv[1])!="--no-audio")return 3;
 using namespace ember::hero_motion;float head=100,torso=100,gripError=0;int count=0,collisions=0;
 float worstPhase=0,worstGait=0,worstRun=0,worstMotion=0,torsoPhase=0,torsoGait=0,torsoRun=0;int headHits=0,torsoHits=0;
 for(float motion:{0.f,.5f,1.f})for(int blend=0;blend<=20;blend++)for(int gait=0;gait<32;gait++)for(int f=0;f<=240;f++){
  float run=blend/20.f;
  float phase=f/240.f;auto p=evaluate(gait/32.f,motion,run,0);auto k=ember::hero_sheath::apply(p,phase);auto c=stow_clearance::measure(p,k);
  count++;if(c.head<head){head=c.head;worstPhase=phase;worstGait=gait/32.f;worstRun=run;worstMotion=motion;}if(c.torso<torso){torso=c.torso;torsoPhase=phase;torsoGait=gait/32.f;torsoRun=run;}
  headHits+=c.head<1;torsoHits+=c.torso<1;
  collisions+=c.head<1||c.torso<1;
  if(k.reach>.99999f&&k.alignment>.99999f){auto m=ember::sword_stow::Mouth,v=ember::sword_pose::idleBlade(0);auto mouth=p.point(data::Pelvis,{m.r,m.f,m.z});auto tip=p.world[data::HandR].p+p.vector(data::Weapon,{v.r,v.f,v.z})*ember::sword_stow::visibleFraction(k);gripError=std::max(gripError,length(tip-mouth));}
 }
 std::cout<<"Poses: "<<count<<"; intersections: "<<collisions<<"; head minimum: "<<head<<"; torso minimum: "<<torso<<"; grip error: "<<gripError<<'\n';
 std::cout<<"Worst head phase="<<worstPhase<<" gait="<<worstGait<<" run="<<worstRun<<" motion="<<worstMotion<<'\n';
 std::cout<<"Worst torso phase="<<torsoPhase<<" gait="<<torsoGait<<" run="<<torsoRun<<"; head hits="<<headHits<<" torso hits="<<torsoHits<<'\n';
 int realHits=0,realFrames=0;float realHead=100,realTorso=100,realWorstP=0,realWorstRun=0;int worstMode=0;
 for(int fps:{30,60,120})for(int start=0;start<32;start++)for(int mode=0;mode<5;mode++){
  ember::AnimationState a;a.phase=start/32.f;
  for(int f=0;f<fps*6;f++){
   float t=f/float(fps);bool running=mode==0?(t>=.5f&&t<2.5f):mode==1?(int(t/.12f)%2==1):mode==2?(int(t/.3f)%2==1):mode==3?(t>=.5f&&t<1.1f):(int(t/.08f)%2==1);
   bool moving=mode!=3||t<1.1f;a.advance(moving?(running?6.45f:4.3f)/fps:0,0,1.f/fps,running);
   auto p=evaluate(a.phase,a.moveBlend,a.runBlend,a.idleTime);auto key=ember::hero_sheath::apply(p,a.swordStow);auto c=stow_clearance::measure(p,key);
   realHead=std::min(realHead,c.head);if(c.torso<realTorso){realTorso=c.torso;realWorstP=a.swordStow;realWorstRun=a.runBlend;worstMode=mode;}
   realFrames++;realHits+=c.head<1||c.torso<1;
  }
 }
 std::cout<<"Actual reversible input timelines: collisions="<<realHits<<" head="<<realHead<<" torso="<<realTorso<<" phase="<<realWorstP<<" run="<<realWorstRun<<" mode="<<worstMode<<'\n';
 bool failed=collisions||realHits||gripError>.65f;
 if(argc>2){std::ofstream out(argv[2]);out<<"{\"status\":\""<<(failed?"FAIL":"PASS")<<"\",\"poses\":"<<count<<",\"intersections\":"<<collisions<<",\"head_minimum\":"<<head<<",\"torso_minimum\":"<<torso<<",\"grip_error\":"<<gripError<<",\"input_frames\":"<<realFrames<<",\"input_intersections\":"<<realHits<<",\"input_head_minimum\":"<<realHead<<",\"input_torso_minimum\":"<<realTorso<<",\"audio\":false}\n";}
 return failed?2:0;
}
