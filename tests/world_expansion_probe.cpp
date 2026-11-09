#include "../src/game.hpp"
#include <iostream>
using namespace ember;
int main(int argc,char** argv){
 if(argc!=2||std::string(argv[1])!="--no-audio")return 3;
 int failed=0;
 for(auto realm:{WorldLocation::Forest,WorldLocation::Desert,WorldLocation::Volcano}){
  Game g;g.activateField(realm);int creatures=0,objects=0;
  for(const auto& e:g.enemies)creatures+=e.home.x>=72||e.home.y>=72;
  for(const auto& p:g.props)objects+=p.pos.x>=72||p.pos.y>=72;
  std::cout<<"realm "<<int(realm)<<": "<<g.enemies.size()<<" enemies / "<<g.props.size()<<" props; expansion "<<creatures<<" / "<<objects<<'\n';
  if(realm==WorldLocation::Desert){int route=0;for(const auto& road:desertRoads()){
   int blocked=0;for(V p:road.points)if(!desertWater(p)&&distance(p,DesertPortal)>1&&!g.walkable(p)){
    if(blocked++==0){std::cout<<"route "<<route<<" blocked at "<<p.x<<", "<<p.y;for(const auto& prop:g.props)if(distance(prop.pos,p)<propRadius(prop.type))std::cout<<" by type "<<prop.type<<" at "<<prop.pos.x<<", "<<prop.pos.y;std::cout<<'\n';}failed++;
   }route++;
  }}
 }
 return failed?2:0;
}
