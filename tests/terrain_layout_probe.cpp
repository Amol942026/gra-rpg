#include "../src/game.hpp"
#include <fstream>
#include <iostream>
using namespace ember;
int main(int argc,char** argv){
 if(argc!=3||std::string(argv[1])!="--no-audio")return 3;
 std::ofstream output(argv[2]);output<<"[";bool first=true;int failures=0;
 for(WorldLocation realm:{WorldLocation::Forest,WorldLocation::Desert,WorldLocation::Volcano}){
  float maximum=0,minimumJacobian=100;int raised=0,total=0;
  for(float y=MapWalkMin;y<MapWalkMax;y+=.2f)for(float x=MapWalkMin;x<MapWalkMax;x+=.2f){V p{x,y},g=terrain_height::gradient(realm,p);float h=terrain_height::height(realm,p);maximum=std::max(maximum,h);minimumJacobian=std::min(minimumJacobian,40-32*(g.x+g.y));raised+=h>.15f;total++;}
  bool pass=maximum*32<=terrain_height::MaxPixels&&minimumJacobian>8;failures+=!pass;
  if(!first)output<<',';first=false;output<<"{\"realm\":"<<int(realm)<<",\"hills\":"<<terrain_height::count(realm)<<",\"maximum\":"<<maximum<<",\"min_projection_jacobian\":"<<minimumJacobian<<",\"raised_fraction\":"<<double(raised)/total<<",\"pass\":"<<(pass?"true":"false")<<'}';
 }
 output<<']';return failures?2:0;
}
