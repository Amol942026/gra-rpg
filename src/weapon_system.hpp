#pragma once
// Included inside Game. The wheel stores shortcuts, never additional items.
Slot weaponSource(int index)const{
 if(index<0||index>=WeaponWheelSize)return {};
 const auto& ref=weaponWheel[index];if(ref.kind==Kind::None)return {};
 auto matches=[&](Item value){return !value.empty()&&gearSlot(value.kind)==0&&value.kind==ref.kind&&value.tier==ref.tier;};
 if(matches(equipment[0]))return {Place::Equip,0};
 for(int i=0;i<BagSize;i++)if(matches(bag[i]))return {Place::Bag,i};return {};
}
bool bindWeapon(Slot source,int index){
 if(dead()||allocationRequired||index<0||index>=WeaponWheelSize||(source.place!=Place::Bag&&!(source.place==Place::Equip&&source.index==0)))return false;
 Item* value=get(source);if(!value||value->empty()||gearSlot(value->kind)!=0){say("Przypisz broń z plecaka lub miejsca BROŃ.");return false;}
 for(int i=0;i<WeaponWheelSize;i++)if(i!=index&&weaponWheel[i].kind==value->kind&&weaponWheel[i].tier==value->tier)weaponWheel[i]={};
 weaponWheel[index]={value->kind,value->tier};sound(Cue::Ui,player);say("Broń przypisana do koła.");return true;
}
bool clearWeaponBinding(int index){if(dead()||allocationRequired||index<0||index>=WeaponWheelSize)return false;weaponWheel[index]={};sound(Cue::Ui,player);return true;}
bool selectWeapon(int index){
 if(dead()||allocationRequired)return false;Slot source=weaponSource(index);
 if(source.place==Place::None){say("Tej broni nie masz już w plecaku.");return false;}
 cancelBow();if(source.place==Place::Equip)return true;
 return transfer(source,{Place::Equip,0});
}
int ammoCount()const{int count=0;for(const auto& value:bag)if(value.kind==Kind::Arrow&&!value.empty())count+=value.count;return count;}
float bowCharge()const{return std::clamp(bowDrawTime/BowDrawDuration,0.f,1.f);}
static float bowRange(float charge){return BowMinRange+(BowMaxRange-BowMinRange)*std::clamp(charge,0.f,1.f);}
float arrowDamage(float charge)const{return float(damage())+12*std::clamp(charge,0.f,1.f);}
void cancelBow(){bowDrawing=false;bowDrawTime=0;bowReleaseTime=0;piercingArrowArmed=false;}
void aimBow(V aim){if(!bowDrawing||length(aim)<.001f)return;bowAim=normalized(aim);facing=bowAim;animation.face(bowAim.x,bowAim.y);}
bool beginBow(V aim){
 if(sandTrapped()||dead()||allocationRequired||storyBlocked()||inTower()||rolling()||attackCd>0||bowDrawing||archetype(equipment[0].kind)!=Kind::Bow)return false;
 if(piercingArrowArmed&&mana<PiercingArrowCost){say("Przeszywająca strzała wymaga 20 many.");return false;}
 if(ammoCount()<=0){say("Brak strzał w plecaku.");sound(Cue::Reject,player);return false;}
 bowDrawing=true;bowDrawTime=bowReleaseTime=0;bowAim=normalized(length(aim)>.001f?aim:facing);if(length(bowAim)<.001f)bowAim={0,-1};aimBow(bowAim);return true;
}
bool releaseBow(){
 if(!bowDrawing)return false;
 float charge=bowCharge();V direction=bowAim;bool piercing=piercingArrowArmed;cancelBow();
 if(sandTrapped()||dead()||allocationRequired||storyBlocked()||inTower()||rolling()||attackCd>0||archetype(equipment[0].kind)!=Kind::Bow||ammoCount()<=0)return false;
 if(piercing&&(skillRank(Skill::PiercingArrow)<=0||piercingArrowCd>0||mana<PiercingArrowCost))return false;
 for(auto& value:bag)if(value.kind==Kind::Arrow&&!value.empty()){if(--value.count<=0)value={};break;}
 arrows.push_back({player,direction*(12+8*charge),bowRange(charge),arrowDamage(charge),piercing,piercing?piercingTargets():1,{},rollCritical()});
 if(piercing){mana-=PiercingArrowCost;piercingArrowCd=PiercingArrowCooldown;}
 lastBowPiercing=piercing;
 attackCd=BowCooldown;lastBowCharge=charge;bowReleaseTime=.18f;sound(Cue::BowShot,player);return true;
}
void updateArrows(float dt){
 for(size_t i=0;i<arrows.size();){
  // Work with an owned copy while hit() may award levels or change world state.
  Arrow shot=std::move(arrows[i]);arrows.erase(arrows.begin()+i);
  float initialRemaining=shot.remaining;float travel=std::min(shot.remaining,length(shot.velocity)*std::max(0.f,dt));
  V delta=normalized(shot.velocity)*travel;float block=2;
  if(!inTower())for(const auto& prop:props)block=std::min(block,segmentCircle(shot.pos,delta,prop.pos,propRadius(prop.type)+.06f));
  V end=shot.pos+delta;
  for(int axis=0;axis<2;axis++){
   float start=axis==0?shot.pos.x:shot.pos.y,finish=axis==0?end.x:end.y;
   if(finish<1||finish>MapSize-2)block=std::min(block,std::clamp(((finish<1?1.f:float(MapSize-2))-start)/(finish-start),0.f,1.f));
  }
  std::vector<std::pair<float,int>> contacts;
  if(!inTower())for(const auto& enemy:enemies)if(enemy.alive&&std::find(shot.hitIds.begin(),shot.hitIds.end(),enemy.id)==shot.hitIds.end()){
   float t=segmentCircle(shot.pos,delta,enemy.pos,enemy.radius()+.09f);
   if(t<=1&&t<block)contacts.push_back({t,enemy.id});
  }
  std::sort(contacts.begin(),contacts.end());bool stopped=block<=1;float stop=stopped?block:1.f;
  for(const auto& contact:contacts){
   Enemy* enemy=findEnemy(contact.second);if(!enemy)continue;
   shot.hitIds.push_back(enemy->id);V impact=shot.pos+delta*contact.first;
   if(shot.piercing)pierceFlashes.push_back({impact});
   hit(*enemy,shot.damage,DamageType::Physical,shot.critical);
   if(int(shot.hitIds.size())>=shot.maxTargets){stop=contact.first;stopped=true;break;}
   if(inTower()||storyBlocked()){stopped=true;stop=contact.first;break;}
  }
  shot.pos=shot.pos+delta*stop;shot.remaining-=travel*stop;
  bool expired=travel>=initialRemaining-.00001f;
  if(!stopped&&!expired&&!inTower()&&!storyBlocked()){arrows.insert(arrows.begin()+i,std::move(shot));i++;}
  if(allocationRequired||storyBlocked()||inTower())return;
 }
}
