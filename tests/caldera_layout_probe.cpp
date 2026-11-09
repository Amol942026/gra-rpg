#include "../src/save_game.hpp"
#include <iostream>
using namespace ember;
int main(int argc,char** argv){
 if(argc<3||std::string(argv[1])!="--no-audio")return 3;
 Game g;g.activateField(WorldLocation::Volcano);
 std::vector<float> gaps;float nearest=1000,average=0;int count=0;
 for(const auto& p:g.props)if(p.type>=14){
  float d=1000;for(const auto& q:g.props)if(&q!=&p&&q.type>=14)d=std::min(d,distance(p.pos,q.pos));
  nearest=std::min(nearest,d);average+=d;count++;
 }
 for(float y=4;y<=68;y++)for(float x=4;x<=68;x++){
  V p{x,y};bool clear=!volcanoLava(p,1.62f)&&!nearVolcanoFountain(p,3)&&!nearTreasure(p,WorldLocation::Volcano)&&distance(p,VolcanoArrival)>=6&&distance(p,VolcanoPortal)>=4&&distance(p,VolcanoBossHome)>=8;
  for(const auto& road:VolcanoRoads)for(int i=1;i<6;i++)if(desertSegmentDistance(p,road[i-1],road[i])<2.52f)clear=false;
  for(const auto& e:g.enemies)if(distance(p,e.home)<e.radius()+2.5f)clear=false;
  if(!clear)continue;float d=1000;for(const auto& q:g.props)if(q.type>=14)d=std::min(d,distance(p,q.pos));gaps.push_back(d);
 }
 std::sort(gaps.begin(),gaps.end());
 std::ofstream out(argv[2]);out<<"{\"revision\":"<<VolcanoSceneryRevision<<",\"decorations\":"<<count<<",\"min_spacing\":"<<nearest<<",\"mean_spacing\":"<<average/count<<",\"largest_gap\":"<<gaps.back()<<",\"gap_p95\":"<<gaps[(gaps.size()-1)*95/100]<<",\"props\":[";
 bool first=true;for(const auto& p:g.props){if(!first)out<<',';first=false;out<<'['<<p.pos.x<<','<<p.pos.y<<','<<p.type<<','<<p.size<<']';}out<<"]}\n";
 if(argc==4){auto old=saves::read("art/volcano/river-detail/fixtures/legacy-v10.efs");saves::atomicWrite(argv[3],saves::encode(old));}
 std::cout<<count<<" decorations; minimum separation "<<nearest<<"; largest empty radius "<<gaps.back()<<'\n';
}
