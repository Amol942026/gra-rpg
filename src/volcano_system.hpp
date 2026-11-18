// Included inside Game; native third-field state and local combat.
FieldState& storedField(WorldLocation realm){return realm==WorldLocation::Volcano?volcanoState:realm==WorldLocation::Desert?desertState:forestState;}
static bool retiredVolcanoBridgeProp(const Prop& p){
 // These two silhouettes covered the deck of the first crossing.
 return (p.type==17&&distance(p.pos,{30.58011f,50.88041f})<.02f)||
        (p.type==14&&distance(p.pos,{21.01995f,46.48085f})<.02f);
}
static bool volcanoPropCoversBridge(const Prop& prop){
 if(prop.type<14||prop.type>17)return false;
 // Bounds of visible pixels across the eight painted variants, in world
 // sprite pixels relative to the ground anchor. The vent includes its steam.
 struct Bounds {float halfWidth,top,bottom;};
 static constexpr Bounds bounds[]={{60,-153,17},{57,-165,14},{57,-147,18},{57,-171,13}};
 const auto shape=bounds[prop.type-14];
 const float rise=terrain_height::pixels(WorldLocation::Volcano,prop.pos);
 const float span=VolcanoBridgeHalfSpan,depth=VolcanoBridgeHalfDepth;
 for(size_t i=0;i<VolcanoBridges.size();i++){
  float y=VolcanoBridges[i].y;
  V delta=prop.pos-V{lavaCenter(VolcanoBridgeRivers[i],y),y};
  float u=(delta.x-delta.y)*40,v=(delta.x+delta.y)*20-rise;
  float left=u-shape.halfWidth*prop.size,right=u+shape.halfWidth*prop.size;
  float top=v+shape.top*prop.size,bottom=v+shape.bottom*prop.size;
  if(left>(span+depth)*40||right<-(span+depth)*40||top>(span+depth)*20||bottom<-(span+depth)*20)continue;
  if(left/80+top/40>span||right/80+bottom/40<-span||-right/80+top/40>depth||-left/80+bottom/40<-depth)continue;
  return true;
 }
 return false;
}
void appendVolcanoScenery(std::vector<Prop>& result,const std::vector<Enemy>& inhabitants,bool extension,bool protectPlayer=false)const{
 // Deterministic gap placement. The legacy pass keeps its original candidate
 // space and road layout; the new pass continues the same spacing beyond it.
 struct Candidate{Prop prop;float gap=10000;};std::vector<Candidate> candidates;candidates.reserve(8192);
 const int extent=extension?MapSize:PreviousMapSize;
 const size_t roadCount=extension?VolcanoRoads.size():9;
 for(int n=0;n<8192;n++){
   unsigned h=hash(n+941+(extension?14219:0),271);
   V p{3.5f+float(h&65535)/65535.f*(extent-7),3.5f+float(h>>16)/65535.f*(extent-7)};
   if(extension&&p.x<PreviousMapSize-.5f&&p.y<PreviousMapSize-.5f)continue;
   int roll=hash(n,734)%100,type;
   if(p.x<24)type=roll<53?15:roll<87?14:16;
   else if(p.x>51)type=roll<53?17:roll<83?14:16;
   else type=roll<38?14:roll<70?16:17;
   const float radius=propRadius(type);
   if(volcanoLava(p,radius+.8f)||nearVolcanoFountain(p,3)||nearTreasure(p,WorldLocation::Volcano)||distance(p,VolcanoArrival)<6||distance(p,VolcanoPortal)<4||distance(p,VolcanoBossHome)<8||nearFieldSite(WorldLocation::Volcano,p,2.4f))continue;
   const Prop prop{p,type,.74f+float(hash(n,519)%25)*.01f};
   if(extension&&volcanoPropCoversBridge(prop))continue;
   bool clear=true;
   for(size_t road=0;road<roadCount&&clear;road++)for(int i=1;i<6;i++)
    if(desertSegmentDistance(p,VolcanoRoads[road][i-1],VolcanoRoads[road][i])<1.7f+radius)clear=false;
   if(!clear)continue;
   for(const auto& e:inhabitants)if(distance(p,e.home)<e.radius()+2.5f||distance(p,e.pos)<e.radius()+1.2f){clear=false;break;}
   if(protectPlayer&&distance(p,player)<radius+1.2f)clear=false;
   for(const auto& drop:ground)if(drop.location==WorldLocation::Volcano&&distance(p,drop.pos)<radius+1.2f)clear=false;
   if(!clear)continue;
   float gap=10000;
   for(const auto& old:result)gap=std::min(gap,distance(old.pos,p));
   candidates.push_back({prop,gap});
 }
 for(int placed=0;placed<208;placed++){
  int best=-1;float gap=4.4f;
  for(int i=0;i<int(candidates.size());i++)if(candidates[i].gap>gap){gap=candidates[i].gap;best=i;}
  if(best<0)break;
  Prop chosen=candidates[best].prop;result.push_back(chosen);
  for(auto& candidate:candidates)candidate.gap=std::min(candidate.gap,distance(candidate.prop.pos,chosen.pos));
 }
}
static void clearVolcanoBridgeProps(std::vector<Prop>& scenery){
 scenery.erase(std::remove_if(scenery.begin(),scenery.end(),[](const Prop& prop){
  return retiredVolcanoBridgeProp(prop)||volcanoPropCoversBridge(prop);
 }),scenery.end());
}
void upgradeVolcanoScenery(std::vector<Prop>& scenery,const std::vector<Enemy>& inhabitants,bool retain,bool protectPlayer){
 if(!retain){
  scenery.clear();
  appendVolcanoScenery(scenery,inhabitants,false,protectPlayer);
  scenery.push_back({VolcanoPortal,13,1});
 }
 clearVolcanoBridgeProps(scenery);
 appendVolcanoScenery(scenery,inhabitants,true,protectPlayer);
}
void addVolcanoLandmarks(std::vector<Prop>& scenery,const std::vector<Enemy>& inhabitants,bool protectPlayer)const{
 if(std::any_of(scenery.begin(),scenery.end(),[](const Prop& prop){return volcanoLandmark(prop.type);}))return;
 // Places of interest are spread across the four corners and old mining routes.
 struct Placement{V around;int design;};
 static constexpr Placement placements[]={
  {{17,29},0},{{55,69},0},{{91,89},0},{{130,121},0},
  {{14,49},1},{{34,94},1},{{85,114},1},{{132,28},1},
  {{14,102},2},{{40,19},2},{{81,132},2},{{132,100},2},
  {{14,63},3},{{40,76},3},{{93,54},3},{{115,67},3}
 };
 const auto clear=[&](V p,float radius){
  if(p.x<4||p.y<4||p.x>MapWalkMax-3||p.y>MapWalkMax-3||
     distance(p,VolcanoArrival)<4||distance(p,VolcanoPortal)<3.4f||distance(p,VolcanoBossHome)<9||
     nearFieldSite(WorldLocation::Volcano,p,2.75f)||nearTreasure(p,WorldLocation::Volcano,3.6f)||
     volcanoLava(p,radius+1.4f)||nearVolcanoFountain(p,3.4f)||volcano_roads::distanceTo(p)<2.55f+radius||
     (protectPlayer&&distance(p,player)<radius+1.4f))return false;
  for(size_t i=0;i<VolcanoBridges.size();i++){
   const float y=VolcanoBridges[i].y;
   if(distance(p,{lavaCenter(VolcanoBridgeRivers[i],y),y})<8.6f)return false;
  }
  for(const auto& prop:scenery){
   const float gap=volcanoLandmark(prop.type)?5.5f:radius+propRadius(prop.type)+1.05f;
   if(distance(p,prop.pos)<gap)return false;
  }
  for(const auto& enemy:inhabitants){
   const float gap=radius+enemy.radius()+1.f;
   if(distance(p,enemy.home)<gap||distance(p,enemy.pos)<gap)return false;
  }
  for(const auto& drop:ground)if(drop.location==WorldLocation::Volcano&&distance(p,drop.pos)<radius+1.4f)return false;
  return true;
 };
 for(size_t i=0;i<std::size(placements);i++){
  const auto& plan=placements[i];
  const int type=VolcanoLandmarkStart+plan.design;
  const float phase=float(hash(int(i),812)%360)*AnimationPi/180;
  for(int attempt=0;attempt<=96;attempt++){
   const float reach=1.25f*((attempt+11)/12),angle=phase+attempt*(2*AnimationPi/12);
   const V p=plan.around+V{std::cos(angle),std::sin(angle)}*reach;
   if(!clear(p,propRadius(type)))continue;
   scenery.push_back({p,type,.91f+float(hash(int(i),512)%15)*.01f});
   break;
  }
 }
}
void refreshVolcanoScenery(){
 if(!volcanoCreated||volcanoSceneryVersion>=VolcanoSceneryRevision)return;
 const bool active=fieldLocation==WorldLocation::Volcano;
 auto& scenery=active?props:volcanoState.props;
 if(volcanoSceneryVersion<6)upgradeVolcanoScenery(scenery,active?enemies:volcanoState.enemies,volcanoSceneryVersion>=5,inVolcano());
 else clearVolcanoBridgeProps(scenery);
 addVolcanoLandmarks(scenery,active?enemies:volcanoState.enemies,inVolcano());
 volcanoSceneryVersion=VolcanoSceneryRevision;
}
void buildVolcano(){
 enemies.clear();props.clear();forage.clear();
 for(size_t i=0;i<VolcanoSpawns.size();i++){
  auto spawn=VolcanoSpawns[i];float radius=enemyInfo(spawn.type).radius;
  while(volcanoLava(spawn.pos,radius+.15f))spawn.pos.x-=.25f;
  float health=enemyInfo(spawn.type).health;enemies.push_back({2000+int(i)+(i>=99),spawn.type,spawn.pos,spawn.pos,health,health});enemies.back().patrolWait=.6f+(hash(int(i),301)%1200)*.001f;
 }
 float hp=enemyInfo(VolcanoBossType).health;enemies.push_back({VolcanoBossId,VolcanoBossType,VolcanoBossHome,VolcanoBossHome,hp,hp});
 upgradeVolcanoScenery(props,enemies,false,false);
 const size_t guardianIndex=enemies.size()-1;
 addRegionalEnemies(WorldLocation::Volcano,enemies,props,forage);
 std::rotate(enemies.begin()+guardianIndex,enemies.begin()+guardianIndex+1,enemies.end());
 volcanoCreated=true;volcanoSceneryVersion=VolcanoSceneryRevision;
 addRealmDecorations(WorldLocation::Volcano);
 addVolcanoLandmarks(props,enemies,false);
}
bool canTravelVolcano()const{return mageReachable()&&desertBossDead;}
void resetFieldArrival(V arrival){
 player=arrival;facing={1,0};animation={};animation.face(1,0);cancelBow();closeCorpse();openedChest=-1;
 rollRemaining=rollCd=swing=attackCd=spellCd=hurt=stepDistance=0;
 arrows.clear();fireballs.clear();explosions.clear();effects.clear();combatImpacts.clear();pierceFlashes.clear();healingPulse=0;cinders.clear();fissures.clear();
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
static void advanceVolcanoAnimation(Enemy& e,V movement,float dt,bool faceMovement){
 const float phase=e.animation.phase;
 const float previousAngle=e.animation.facingAngle;
 const bool plantedTurn=e.type==SlagCrawlerType&&e.animation.facingInitialized;
 const float maxTurnRate=smoothVolcanoTurn(e.type)?7.f:0.f;
 e.animation.advance(movement.x,movement.y,dt,false,faceMovement,true,maxTurnRate);
 if(plantedTurn){
  const float arc=std::abs(std::remainder(e.animation.facingAngle-previousAngle,2*AnimationPi));
  e.animation.pivotPhase=std::fmod(e.animation.pivotPhase+arc*(1.9f/AnimationPi),1.f);
 }
 e.animation.phase=std::fmod(phase+std::hypot((movement.x-movement.y)*40,(movement.x+movement.y)*20)/VolcanoStrides[e.type-VolcanoFirstType],1.f);
}
void cancelVolcanoAttacks(){
 cinders.clear();fissures.clear();for(auto& e:enemies){if(e.eruptionWindup>0)e.eruptionCd=std::max(e.eruptionCd,2.8f);e.eruptionWindup=0;}
}
bool prepareEruption(Enemy& e,float dt,bool visible){
 if(e.type!=VolcanoBossType||(!inVolcano()&&!testSummon(e.id)))return false;
 if(visible||e.eruptionWindup>0)e.eruptionCd=std::max(0.f,e.eruptionCd-dt);
 if(e.eruptionWindup>0){e.animation.face(player.x-e.pos.x,player.y-e.pos.y);e.eruptionWindup=std::max(0.f,e.eruptionWindup-dt);return true;}
 if(dt>0&&visible&&e.eruptionCd<=.00001f&&e.windup<=0&&e.attack<=0&&fissures.size()<=9){
  e.eruptionCd=6.f;e.eruptionWindup=.95f;e.attack=1.5f;e.eruptions++;sound(enemyAttackCue(e.type),e.pos);
  V side=normalized(V{player.y-e.pos.y,e.pos.x-player.x});
  for(int n=-1;n<=1;n++){V target=player+side*(n*2.6f);if(!walkable(target))target=player;fissures.push_back({target,0,.95f+std::abs(n)*.23f,48,e.id,false});}
  return true;
 }return false;
}
void setCinderLaunch(CinderBolt& b,const Enemy& owner,V aim){
 if(owner.type!=FireImpType&&owner.type!=AshWraithType)return;
 const float angle=int(owner.animation.direction)*AnimationPi/4;
 if(owner.type==FireImpType)b.aimRange=std::clamp(distance(aim,owner.pos),.01f,7.f);
 b.castFacing=owner.animation.facingInitialized?owner.animation.facingAngle:std::atan2(.5f*std::sin(angle),std::cos(angle));
}
void launchCinder(const Enemy& e){
 if(cinders.size()>=24)return;
 CinderBolt bolt{e.pos,normalized(player-e.pos)*7.5f,7,float(e.damage()),e.id};
 setCinderLaunch(bolt,e,player);
 cinders.push_back(bolt);sound(enemyAttackCue(e.type),e.pos);
}
void updateVolcanoAttacks(float dt,V heroBefore,bool wasRolling,float rollAge){
 if((cinders.empty()&&fissures.empty())||dt<=0)return;
 for(size_t i=0;i<cinders.size();){
  auto& b=cinders[i];
  // Use the same time interval for the missile and the moving hero. A long
  // simulation step must stop at the spell's range, even if the hero moved.
  const float speed=length(b.velocity);
  const float step=speed>0?std::min(dt,b.remaining/speed):0;
  const V delta=b.velocity*step,heroTravel=(player-heroBefore)*(step/dt);
  float obstacle=2;
  for(const auto& p:props)obstacle=std::min(obstacle,segmentCircle(b.pos,delta,p.pos,propRadius(p.type)+.12f));
  for(const auto& c:chests)if(c.location==location)obstacle=std::min(obstacle,segmentCircle(b.pos,delta,c.pos,ChestRadius+.12f));
  if(hasFountains())for(V p:fountains())obstacle=std::min(obstacle,segmentCircle(b.pos,delta,p,FountainRadius+.12f));
  const float contact=dead()?2:segmentCircle(b.pos-heroBefore,delta-heroTravel,{},.42f);
  const bool hit=contact<=1&&contact<=obstacle;
  const float fraction=std::min({1.f,contact,obstacle});
  b.pos=b.pos+delta*fraction;b.remaining-=speed*step*fraction;
  const bool dodged=hit&&wasRolling&&rollAge+step*contact<RollProtection;
  if(hit){
   const V at=heroBefore+heroTravel*contact;
   if(dodged){sound(Cue::Dodge,at);effects.push_back({at,"UNIK",.65f,3});}
   else takeMonsterDamage(incomingDamage(b.damage,DamageType::Magic),at,.22f,b.velocity,DamageType::Magic);
  }
  if((hit||obstacle<=1)&&!dodged){
   const auto* owner=findEnemy(b.owner);
   if(owner&&owner->type==FireImpType){
    if(b.aimRange<=0)setCinderLaunch(b,*owner,player);
    explosions.push_back({b.pos,spellDefinition(Spell::Fireball).impactLife,Spell::Fireball,b.handWeight(),b.castFacing});
    sound(Cue::FireballImpact,b.pos);
   }
  }
  if(hit||obstacle<=1||b.remaining<=.00001f||b.pos.x<MapWalkMin||b.pos.y<MapWalkMin||b.pos.x>MapWalkMax||b.pos.y>MapWalkMax)
   cinders.erase(cinders.begin()+i);
  else ++i;
 }
 for(auto& f:fissures){
  float before=f.age;f.age+=dt;
  if(!f.struck&&f.age>=f.delay){
   f.struck=true;float t=std::clamp((f.delay-before)/dt,0.f,1.f);V at=heroBefore+(player-heroBefore)*t;
   if(distance(at,f.pos)<1.35f&&clearEnemySegment(f.pos,at,0)&&!dead()){
    if(wasRolling&&rollAge+dt*t<RollProtection){sound(Cue::Dodge,at);effects.push_back({at,"UNIK",.65f,3});}
    else{
     const V away=at-f.pos;
     takeMonsterDamage(incomingDamage(f.damage,DamageType::Magic),at,.3f,length(away)>.001f?away:facing*-1,DamageType::Magic);
    }
   }sound(Cue::StoneHit,f.pos);
  }
 }
 fissures.erase(std::remove_if(fissures.begin(),fissures.end(),[](const auto& f){return f.age>f.delay+.65f;}),fissures.end());
}
