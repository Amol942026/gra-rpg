// Included inside Game; native third-field state and local combat.
FieldState& storedField(WorldLocation realm){return realm==WorldLocation::Volcano?volcanoState:realm==WorldLocation::Desert?desertState:forestState;}
std::vector<Prop> makeVolcanoScenery(const std::vector<Enemy>& inhabitants,bool protectPlayer=false)const{
 // Pick the largest remaining gaps from a deterministic, irregular candidate
 // set. Regional materials remain distinct without tightly packed clusters.
 // Generation happens only on realm creation / scenery migration, using no RNG.
 struct Candidate{Prop prop;float gap=10000;};std::vector<Candidate> candidates;candidates.reserve(8192);
 std::vector<Prop> result;result.reserve(209);
 for(int n=0;n<8192;n++){
   unsigned h=hash(n+941,271);V p{3.5f+float(h&65535)/65535.f*(MapSize-7),3.5f+float(h>>16)/65535.f*(MapSize-7)};
   int roll=hash(n,734)%100,type;
   if(p.x<24)type=roll<53?15:roll<87?14:16;
   else if(p.x>51)type=roll<53?17:roll<83?14:16;
   else type=roll<38?14:roll<70?16:17;
   float radius=propRadius(type);
   if(p.x<3||p.y<3||p.x>MapSize-3||p.y>MapSize-3||volcanoLava(p,radius+.8f)||nearVolcanoFountain(p,3)||nearTreasure(p,WorldLocation::Volcano)||distance(p,VolcanoArrival)<6||distance(p,VolcanoPortal)<4||distance(p,VolcanoBossHome)<8)continue;
   bool clear=true;
   for(const auto& road:VolcanoRoads)for(int i=1;i<6;i++)if(desertSegmentDistance(p,road[i-1],road[i])<1.7f+radius)clear=false;
   for(const auto& e:inhabitants)if(distance(p,e.home)<e.radius()+2.5f||distance(p,e.pos)<e.radius()+1.2f)clear=false;
   if(protectPlayer&&distance(p,player)<radius+1.2f)clear=false;
   if(clear)candidates.push_back({{p,type,.74f+float(hash(n,519)%25)*.01f}});
 }
 for(int placed=0;placed<208;placed++){
  int best=-1;float gap=4.4f;
  for(int i=0;i<int(candidates.size());i++)if(candidates[i].gap>gap){gap=candidates[i].gap;best=i;}
  if(best<0)break;
  Prop chosen=candidates[best].prop;result.push_back(chosen);
  for(auto& candidate:candidates)candidate.gap=std::min(candidate.gap,distance(candidate.prop.pos,chosen.pos));
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
 say(volcanoBossDead?"Kaldera Popiołu. Brama Eldrina prowadzi do wieży.":"Kaldera Popiołu. Varkhor strzeże trzeciej części Serca Świtu na północnym wschodzie.");sound(Cue::TeleportArrival,player);return true;
}
bool useVolcanoPortal(){
 if(!inVolcano()||dead()||allocationRequired||storyBlocked()||rolling()||knockedBack())return false;
 if(distance(player,VolcanoPortal)>2.8f){say("Podejdź do Bramy Eldrina (E / PPM).");return false;}
 enterMageTower();sound(Cue::TeleportArrival,player);dawnQuest=DawnQuest::AtTower;say("Eldrin może ponownie otworzyć drogę do Kaldery Popiołu.");return true;
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
  e.eruptionCd=6.f;e.eruptionWindup=.95f;e.attack=1.5f;e.eruptions++;sound(enemyAttackCue(e.type),e.pos);
  V side=normalized(V{player.y-e.pos.y,e.pos.x-player.x});
  for(int n=-1;n<=1;n++){V target=player+side*(n*2.6f);if(!walkable(target))target=player;fissures.push_back({target,0,.95f+std::abs(n)*.23f,48,e.id,false});}
  return true;
 }return false;
}
void launchCinder(const Enemy& e){if(cinders.size()<24){cinders.push_back({e.pos,normalized(player-e.pos)*7.5f,7,float(e.damage()),e.id});sound(Cue::MagicAttack,e.pos);}}
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
