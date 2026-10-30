// Included inside Game; native third-field state and local combat.
FieldState& storedField(WorldLocation realm){return realm==WorldLocation::Volcano?volcanoState:realm==WorldLocation::Desert?desertState:forestState;}
std::vector<Prop> makeVolcanoScenery(const std::vector<Enemy>& inhabitants,bool protectPlayer=false)const{
 struct Cluster{V centre;int type,count;float spread;};
 const Cluster groups[]={{{9,44},15,7,4.6f},{{12,29},15,7,4.4f},{{19,13},15,6,4.2f},{{8,54},15,5,3.7f},
  {{18,60},14,6,4.2f},{{6,19},14,6,3.8f},{{21,36},14,7,4.5f},{{34,13},14,6,3.5f},{{35,49},14,6,3.7f},
  {{58,63},14,6,3.8f},{{63,31},14,7,4.4f},{{67,18},14,5,3.5f},{{45,9},14,5,3.3f},
  {{43,55},17,7,4.4f},{{65,47},17,7,4.4f},{{57,9},17,6,3.8f},{{33,64},17,6,3.6f},
  {{37,16},16,4,3.2f},{{44,45},16,4,3.4f},{{56,36},16,4,3.6f},
  {{12,59},14,3,3.2f},{{13,33},15,3,3.2f},{{39,53},16,3,3.1f},{{61,20},17,3,3.5f}};
 std::vector<Prop> result;result.reserve(132);
 for(int group=0;group<int(std::size(groups));group++){
  const auto& c=groups[group];int made=0;
  for(int n=0;n<c.count*12&&made<c.count;n++){
   unsigned h=hash(group+941,n+271);float a=n*2.3999632f+group*.71f;
   float r=c.spread*std::sqrt(((h>>8)%1000+.5f)/1000.f);V p=c.centre+V{std::cos(a)*r,std::sin(a)*r};
   float radius=propRadius(c.type);
   if(p.x<3||p.y<3||p.x>MapSize-3||p.y>MapSize-3||volcanoLava(p,radius+.8f)||nearVolcanoFountain(p,3)||nearTreasure(p,WorldLocation::Volcano)||distance(p,VolcanoArrival)<6||distance(p,VolcanoPortal)<4||distance(p,VolcanoBossHome)<8)continue;
   bool clear=true;
   for(const auto& road:VolcanoRoads)for(int i=1;i<6;i++)if(desertSegmentDistance(p,road[i-1],road[i])<1.7f+radius)clear=false;
   for(const auto& e:inhabitants)if(distance(p,e.home)<e.radius()+2.5f||distance(p,e.pos)<e.radius()+1.2f)clear=false;
   for(const auto& q:result)if(distance(p,q.pos)<radius+propRadius(q.type)+.8f)clear=false;
   if(protectPlayer&&distance(p,player)<radius+1.2f)clear=false;
   if(clear){result.push_back({p,c.type,.82f+float((h>>19)%25)*.01f});made++;}
  }
 }
 result.push_back({VolcanoPortal,13,1});return result;
}
void refreshVolcanoScenery(){
 if(!volcanoCreated||volcanoSceneryVersion>=VolcanoSceneryRevision)return;
 if(fieldLocation==WorldLocation::Volcano)props=makeVolcanoScenery(enemies,inVolcano());
 else volcanoState.props=makeVolcanoScenery(volcanoState.enemies);
 volcanoSceneryVersion=VolcanoSceneryRevision;
}
void buildVolcano(){
 enemies.clear();props.clear();forage.clear();
 for(size_t i=0;i<VolcanoSpawns.size();i++){
  auto spawn=VolcanoSpawns[i];float radius=enemyInfo(spawn.type).radius;
  while(volcanoLava(spawn.pos,radius+.15f))spawn.pos.x-=.25f;
  float health=enemyInfo(spawn.type).health;enemies.push_back({2000+int(i),spawn.type,spawn.pos,spawn.pos,health,health});enemies.back().patrolWait=.6f+(hash(int(i),301)%1200)*.001f;
 }
 float hp=enemyInfo(VolcanoBossType).health;enemies.push_back({VolcanoBossId,VolcanoBossType,VolcanoBossHome,VolcanoBossHome,hp,hp});
 props=makeVolcanoScenery(enemies);volcanoCreated=true;volcanoSceneryVersion=VolcanoSceneryRevision;
}
bool canTravelVolcano()const{return mageReachable()&&desertBossDead;}
void resetFieldArrival(V arrival){
 player=arrival;facing={1,0};animation={};animation.face(1,0);cancelBow();closeCorpse();openedChest=-1;
 rollRemaining=rollCd=swing=attackCd=spellCd=hurt=stepDistance=0;
 arrows.clear();fireballs.clear();explosions.clear();effects.clear();pierceFlashes.clear();healingPulse=0;cinders.clear();fissures.clear();
}
bool travelToVolcano(){
 if(!canTravelVolcano())return false;activateField(WorldLocation::Volcano);resetFieldArrival(VolcanoArrival);
 say(volcanoBossDead?"Kaldera Popiołu. Brama Eldrina prowadzi do wieży.":"Kaldera Popiołu. Varkhor strzeże trzeciej części Serca Świtu na północnym wschodzie.");sound(Cue::Spell,player);return true;
}
bool useVolcanoPortal(){
 if(!inVolcano()||dead()||allocationRequired||storyBlocked()||rolling()||knockedBack())return false;
 if(distance(player,VolcanoPortal)>2.8f){say("Podejdź do Bramy Eldrina (E / PPM).");return false;}
 enterMageTower();dawnQuest=DawnQuest::AtTower;say("Eldrin może ponownie otworzyć drogę do Kaldery Popiołu.");return true;
}
static bool volcanoRanged(int type){return type==14||type==15;}
static void advanceVolcanoAnimation(Enemy& e,V movement,float dt){
 float phase=e.animation.phase;e.animation.advance(movement.x,movement.y,dt,false,e.eruptionWindup<=0&&e.windup<=0);
 e.animation.phase=std::fmod(phase+std::hypot((movement.x-movement.y)*40,(movement.x+movement.y)*20)/VolcanoStrides[e.type-VolcanoFirstType],1.f);
}
void cancelVolcanoAttacks(){
 cinders.clear();fissures.clear();for(auto& e:enemies){if(e.eruptionWindup>0)e.eruptionCd=std::max(e.eruptionCd,2.8f);e.eruptionWindup=0;}
}
bool prepareEruption(Enemy& e,float dt,bool visible){
 if(!inVolcano()||e.type!=VolcanoBossType)return false;
 if(visible||e.eruptionWindup>0)e.eruptionCd=std::max(0.f,e.eruptionCd-dt);
 if(e.eruptionWindup>0){e.animation.face(player.x-e.pos.x,player.y-e.pos.y);e.eruptionWindup=std::max(0.f,e.eruptionWindup-dt);return true;}
 if(dt>0&&visible&&e.eruptionCd<=.00001f&&e.windup<=0&&e.attack<=0&&fissures.size()<=9){
  e.eruptionCd=6.f;e.eruptionWindup=.95f;e.attack=1.5f;e.eruptions++;sound(Cue::EnemyAttack,e.pos);
  V side=normalized(V{player.y-e.pos.y,e.pos.x-player.x});
  for(int n=-1;n<=1;n++){V target=player+side*(n*2.6f);if(!walkable(target))target=player;fissures.push_back({target,0,.95f+std::abs(n)*.23f,48,e.id,false});}
  return true;
 }return false;
}
void launchCinder(const Enemy& e){if(cinders.size()<24){cinders.push_back({e.pos,normalized(player-e.pos)*7.5f,7,float(e.damage()),e.id});sound(Cue::Swing,e.pos);}}
void updateVolcanoAttacks(float dt,V heroBefore,bool wasRolling,float rollAge){
 if(!inVolcano()||dt<=0)return;
 for(size_t i=0;i<cinders.size();){
  auto& b=cinders[i];V delta=b.velocity*dt;float first=2;
  for(const auto& p:props)first=std::min(first,segmentCircle(b.pos,delta,p.pos,propRadius(p.type)+.12f));
  for(const auto& c:chests)if(c.location==location)first=std::min(first,segmentCircle(b.pos,delta,c.pos,ChestRadius+.12f));
  for(V p:fountains())first=std::min(first,segmentCircle(b.pos,delta,p,FountainRadius+.12f));
  float contact=segmentCircle(b.pos-heroBefore,delta-(player-heroBefore),{},.42f);
  bool hit=contact<=1&&contact<=first;
  if(hit&&!dead()){
   V at=heroBefore+(player-heroBefore)*contact;
   if(wasRolling&&rollAge+dt*contact<RollProtection){sound(Cue::Dodge,at);effects.push_back({at,"UNIK",.65f,3});}
   else{float damage=incomingDamage(b.damage,DamageType::Magic);hp=std::max(0.f,hp-damage);hurt=.22f;sound(dead()?Cue::Death:Cue::Hurt,player);effects.push_back({player,"-"+std::to_string(int(damage)),.8f,4});}
  }
  b.pos=b.pos+delta;b.remaining-=length(delta);
  if(hit||first<=1||b.remaining<=0||b.pos.x<MapWalkMin||b.pos.y<MapWalkMin||b.pos.x>MapWalkMax||b.pos.y>MapWalkMax)cinders.erase(cinders.begin()+i);else i++;
 }
 for(auto& f:fissures){
  float before=f.age;f.age+=dt;
  if(!f.struck&&f.age>=f.delay){
   f.struck=true;float t=std::clamp((f.delay-before)/dt,0.f,1.f);V at=heroBefore+(player-heroBefore)*t;
   if(distance(at,f.pos)<1.35f&&clearEnemySegment(f.pos,at,0)&&!dead()){
    if(wasRolling&&rollAge+dt*t<RollProtection){sound(Cue::Dodge,at);effects.push_back({at,"UNIK",.65f,3});}
    else{float damage=incomingDamage(f.damage,DamageType::Magic);hp=std::max(0.f,hp-damage);hurt=.3f;sound(dead()?Cue::Death:Cue::Hurt,at);effects.push_back({at,"-"+std::to_string(int(damage)),.8f,4});}
   }sound(Cue::StoneHit,f.pos);
  }
 }
 fissures.erase(std::remove_if(fissures.begin(),fissures.end(),[](const auto& f){return f.age>f.delay+.65f;}),fissures.end());
}
