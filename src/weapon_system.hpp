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
void cancelBow(){bowDrawing=false;bowDrawTime=0;bowReleaseTime=0;}
void aimBow(V aim){if(!bowDrawing||length(aim)<.001f)return;bowAim=normalized(aim);facing=bowAim;animation.face(bowAim.x,bowAim.y);}
bool beginBow(V aim){
 if(dead()||allocationRequired||rolling()||attackCd>0||bowDrawing||equipment[0].kind!=Kind::Bow)return false;
 if(ammoCount()<=0){say("Brak strzał w plecaku.");sound(Cue::Reject,player);return false;}
 bowDrawing=true;bowDrawTime=bowReleaseTime=0;bowAim=normalized(length(aim)>.001f?aim:facing);if(length(bowAim)<.001f)bowAim={0,-1};aimBow(bowAim);return true;
}
bool releaseBow(){
 if(!bowDrawing)return false;
 float charge=bowCharge();V direction=bowAim;cancelBow();
 if(dead()||allocationRequired||rolling()||attackCd>0||equipment[0].kind!=Kind::Bow||ammoCount()<=0)return false;
 for(auto& value:bag)if(value.kind==Kind::Arrow&&!value.empty()){if(--value.count<=0)value={};break;}
 arrows.push_back({player,direction*(12+8*charge),bowRange(charge),arrowDamage(charge)});
 attackCd=BowCooldown;lastBowCharge=charge;bowReleaseTime=.18f;sound(Cue::BowShot,player);return true;
}
void updateArrows(float dt){
 for(size_t i=0;i<arrows.size();){
  auto& arrow=arrows[i];float travel=std::min(arrow.remaining,length(arrow.velocity)*std::max(0.f,dt));
  V delta=normalized(arrow.velocity)*travel;float first=2;int victim=-1;
  for(const auto& enemy:enemies)if(enemy.alive){float t=segmentCircle(arrow.pos,delta,enemy.pos,enemy.radius()+.09f);if(t<first){first=t;victim=enemy.id;}}
  for(const auto& prop:props){float t=segmentCircle(arrow.pos,delta,prop.pos,propRadius(prop.type)+.06f);if(t<=first){first=t;victim=-1;}}
  V end=arrow.pos+delta;
  for(int axis=0;axis<2;axis++){
   float start=axis==0?arrow.pos.x:arrow.pos.y,finish=axis==0?end.x:end.y;
   if(finish<1||finish>MapSize-2){float t=std::clamp(((finish<1?1.f:float(MapSize-2))-start)/(finish-start),0.f,1.f);if(t<=first){first=t;victim=-1;}}
  }
  bool impact=first<=1||travel>=arrow.remaining-.00001f;
  arrow.pos=arrow.pos+delta*(first<=1?first:1);arrow.remaining-=travel;
  if(impact){float amount=arrow.damage;arrows.erase(arrows.begin()+i);if(victim>=0&&first<=1)if(Enemy* enemy=findEnemy(victim))hit(*enemy,amount,DamageType::Physical);if(allocationRequired)return;}
  else i++;
 }
}
