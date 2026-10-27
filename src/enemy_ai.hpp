// Included inside Game. Patrol choices have a separate deterministic sequence,
// so wandering never changes the random loot awarded for a kill.
bool clearEnemySegment(V from,V to,float padding)const{
 V delta=to-from;
 if(inDesert()&&segmentCircle(from,delta,DesertOasis,2.45f+padding)<=1)return false;
 if(inDesert())for(V f:DesertFountains)if(segmentCircle(from,delta,f,FountainRadius+padding)<=1)return false;
 for(const auto& p:props)if(segmentCircle(from,delta,p.pos,propRadius(p.type)+padding)<=1)return false;
 return true;
}
bool canSeePlayer(const Enemy& e)const{
 return e.alive&&!dead()&&distance(e.pos,player)<EnemySightRange&&clearEnemySegment(e.pos,player,0);
}
bool enemyPositionClear(V p,float radius)const{
 if(inDesert()&&(desertWater(p,radius)||nearDesertFountain(p,radius)))return false;
 if(p.x<1.6f+radius||p.y<1.6f+radius||p.x>MapWalkMax-radius||p.y>MapWalkMax-radius)return false;
 for(const auto& prop:props)if(distance(p,prop.pos)<radius+propRadius(prop.type)+.003f)return false;
 return true;
}
// Direct return is cheap. A blocked return uses a small map-grid search, only
// when selecting the next waypoint, with clearance for the creature's body.
bool homeWaypoint(const Enemy& e,V& goal)const{
 const float radius=e.radius()+.001f;
 if(enemyPositionClear(e.home,e.radius())&&clearEnemySegment(e.pos,e.home,radius)){goal=e.home;return true;}
 constexpr int Cells=MapSize*MapSize;
 std::array<int,Cells> parents,queue;parents.fill(-2);
 std::array<signed char,Cells> usable;usable.fill(-1);
 auto position=[](int cell){return V{float(cell%MapSize),float(cell/MapSize)};};
 auto clear=[&](int cell){if(usable[cell]<0)usable[cell]=enemyPositionClear(position(cell),e.radius());return usable[cell]!=0;};
 int start=-1;float nearest=1e9f;int cx=int(std::round(e.pos.x)),cy=int(std::round(e.pos.y));
 for(int y=std::max(1,cy-2);y<=std::min(MapSize-2,cy+2);y++)for(int x=std::max(1,cx-2);x<=std::min(MapSize-2,cx+2);x++){
  int cell=y*MapSize+x;float d=distance(e.pos,position(cell));
  if(d<nearest&&clear(cell)&&clearEnemySegment(e.pos,position(cell),radius)){nearest=d;start=cell;}
 }
 if(start<0)return false;
 int head=0,tail=0,best=start;float bestDistance=distance(position(start),e.home);
 queue[tail++]=start;parents[start]=-1;
 while(head<tail){
  int cell=queue[head++];V p=position(cell);float d=distance(p,e.home);
  if(d<bestDistance){bestDistance=d;best=cell;}
  if(d<.8f&&clearEnemySegment(p,e.home,radius)){best=cell;break;}
  for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++){
   if(dx==0&&dy==0)continue;int x=int(p.x)+dx,y=int(p.y)+dy;
   if(x<1||y<1||x>=MapSize-1||y>=MapSize-1)continue;
   int next=y*MapSize+x;
   if(parents[next]!=-2||!clear(next)||!clearEnemySegment(p,position(next),radius))continue;
   parents[next]=cell;queue[tail++]=next;
  }
 }
 int step=best;while(parents[step]>=0&&parents[step]!=start)step=parents[step];
 goal=distance(e.pos,position(start))>.18f?position(start):position(step);
 return distance(e.pos,goal)>.08f;
}
float patrolPause(Enemy& e){return .65f+(hash(e.id,int(e.patrolSequence++))%1401)*.001f;}
bool choosePatrolGoal(Enemy& e){
 for(int attempt=0;attempt<24;attempt++){
  unsigned h=hash(e.id,int(e.patrolSequence++));float angle=(h%6283)*.001f;
  float radius=.65f+((h>>13)%1001)*.001f*(PatrolRadius-.65f);
  V goal=e.home+V{std::cos(angle),std::sin(angle)}*radius;
  if(distance(goal,e.pos)<.5f||!enemyPositionClear(goal,e.radius())||!clearEnemySegment(e.pos,goal,e.radius()+.001f))continue;
  e.patrolGoal=goal;e.patrolGoalActive=true;e.behavior=EnemyBehavior::Patrol;return true;
 }
 e.patrolWait=.5f;e.behavior=EnemyBehavior::Idle;return false;
}
V patrolVelocity(Enemy& e,float dt){
 if(dt<=0)return {};
 if(e.behavior==EnemyBehavior::Chase){e.behavior=EnemyBehavior::Return;e.patrolGoalActive=false;e.patrolWait=0;}
 if(distance(e.pos,e.home)>PatrolRadius+.1f&&e.behavior!=EnemyBehavior::Return){e.behavior=EnemyBehavior::Return;e.patrolGoalActive=false;e.patrolWait=0;}
 const bool returning=e.behavior==EnemyBehavior::Return;
 if(returning&&distance(e.pos,e.home)<.3f){e.behavior=EnemyBehavior::Idle;e.patrolGoalActive=false;e.patrolWait=patrolPause(e);return {};}
 if(e.patrolGoalActive&&distance(e.pos,e.patrolGoal)<.1f){
  e.patrolGoalActive=false;
  if(!returning){e.behavior=EnemyBehavior::Idle;e.patrolWait=patrolPause(e);return {};}
 }
 if(!e.patrolGoalActive){
  if(e.patrolWait>0){e.patrolWait=std::max(0.f,e.patrolWait-dt);return {};}
  if(returning){if(!homeWaypoint(e,e.patrolGoal)){e.patrolWait=.5f;return {};}e.patrolGoalActive=true;}
  else if(!choosePatrolGoal(e))return {};
 }
 V delta=e.patrolGoal-e.pos;float speed=enemySpeed(e.type)*(returning?1.f:PatrolSpeedFactor);
 return normalized(delta)*std::min(speed,length(delta)/dt);
}

