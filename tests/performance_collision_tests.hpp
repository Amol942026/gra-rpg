#pragma once
void performanceCollisionTests(){
 auto legacyConstrain=[](const Game& g,Enemy& e){
  float radius=e.radius();V before=e.pos;e.pos.x=std::clamp(e.pos.x,1.6f+radius,MapWalkMax-radius);e.pos.y=std::clamp(e.pos.y,1.6f+radius,MapWalkMax-radius);float correction=distance(before,e.pos);
  if(g.inDesert()&&desertWater(e.pos,radius)){V d=e.pos-DesertOasis;float n=length(d),gap=2.45f+radius+.002f;correction=std::max(correction,gap-n);e.pos=DesertOasis+normalized(n>.0001f?d:V{1,0})*gap;}
  if(g.inDesert())for(V f:DesertFountains){V d=e.pos-f;float n=length(d),gap=FountainRadius+radius+.002f;if(n<gap){correction=std::max(correction,gap-n);e.pos=f+normalized(n>.00001f?d:V{1,0})*gap;}}
  for(const auto& p:g.props){V delta=e.pos-p.pos;float d=length(delta),clearance=radius+propRadius(p.type)+.002f;if(d<clearance){correction=std::max(correction,clearance-d);V normal=d>.00001f?delta*(1/d):V{1,0};e.pos=p.pos+normal*clearance;}}
  return correction;
 };
 for(int location=0;location<2;location++){
  Game g;if(location)g.activateField(WorldLocation::Desert);
  for(int i=0;i<8000;i++){
   unsigned h=Game::hash(i,137);V p{float(h%7601)*.01f-2,float((h>>12)%7601)*.01f-2};Enemy a{};a.type=i%EnemyTypeCount;
   if(i%2==0){const auto& prop=g.props[i%g.props.size()];float angle=(h%6283)*.001f,radius=a.radius()+propRadius(prop.type)+.002f+(int(h%5)-2)*.00005f;p=prop.pos+V{std::cos(angle),std::sin(angle)}*radius;}
   bool walk=true;if(g.inDesert()&&(desertWater(p)||nearDesertFountain(p)))walk=false;if(p.x<1.6f||p.y<1.6f||p.x>MapWalkMax||p.y>MapWalkMax)walk=false;
   for(const auto& chest:g.chests)if(chest.location==g.location&&distance(p,chest.pos)<ChestRadius)walk=false;for(const auto& prop:g.props)if(distance(p,prop.pos)<propRadius(prop.type))walk=false;
   check(g.walkable(p)==walk,"broad phase preserves exact player collision, including tangency");
   bool clear=true;float radius=a.radius();if(g.inDesert()&&(desertWater(p,radius)||nearDesertFountain(p,radius)))clear=false;if(p.x<1.6f+radius||p.y<1.6f+radius||p.x>MapWalkMax-radius||p.y>MapWalkMax-radius)clear=false;
   for(const auto& prop:g.props)if(distance(p,prop.pos)<radius+propRadius(prop.type)+.003f)clear=false;
   check(g.enemyPositionClear(p,radius)==clear,"broad phase preserves AI clearance for every enemy radius");
   a.pos=p;Enemy b=a;float before=legacyConstrain(g,a),after=g.constrainEnemy(b);
   check(a.pos.x==b.pos.x&&a.pos.y==b.pos.y&&before==after,"broad phase keeps enemy collision correction bit-identical");
  }
 }
}
