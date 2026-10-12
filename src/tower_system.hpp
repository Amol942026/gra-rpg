#pragma once
// Included in Game. Changing floors preserves progress and isolates local loot.
bool groundVisible(const Ground& value)const{return value.location==location&&(!inTower()||value.floor==towerFloor);}
bool towerUseAllowed()const{return inTower()&&!dead()&&!storyBlocked()&&!allocationRequired&&!rolling();}
bool stairPresent(int direction)const{return direction==1?towerFloor<TowerFloorCount-1:direction==-1&&towerFloor>0;}
bool stairReachable(int direction)const{return towerUseAllowed()&&stairPresent(direction)&&distance(player,direction==1?TowerStairUp:TowerStairDown)<=TowerUseRange;}
bool climbTower(int direction){
 if(!stairReachable(direction))return false;
 towerFloor+=direction;player=direction==1?V{14.7f,14.5f}:V{9.6f,14.5f};
 cancelBow();closeCorpse();swing=rollRemaining=stepDistance=0;animation={};effects.clear();arrows.clear();fireballs.clear();explosions.clear();pierceFlashes.clear();
 facing=direction==1?V{-1,0}:V{1,0};animation.face(facing.x,facing.y);sound(Cue::Ui,player);
 say(std::string("Piętro ")+std::to_string(towerFloor+1)+" / 5 — "+TowerFloors[towerFloor].name);return true;
}
bool towerObjectReachable(int index)const{
 const auto& objects=towerFurnishings(towerFloor);
 return towerUseAllowed()&&index>=0&&index<int(objects.size())&&towerInteractive(objects[index].type)&&distance(player,objects[index].pos)<=TowerUseRange;
}
bool restAtTower(int index){
 if(!towerObjectReachable(index)||towerFurnishings(towerFloor)[index].type!=4)return false;
 hp=float(maxHp());mana=float(maxMana());manaRegenTime=0;effects.push_back({player,"ODNOWA",1.2f,2});sound(Cue::Potion,player);say("Źródło odnowiło zdrowie i manę.");return true;
}
bool toggleTowerRune(int index){
 if(!towerObjectReachable(index)||towerFloor!=3||towerFurnishings(towerFloor)[index].type!=9||index>2)return false;
 towerRuneMask^=1<<index;sound(Cue::Ui,player);say(towerRuneMask==7?"Trzy pieczęci płoną. Blask Serca Świtu jest ustabilizowany.":"Runiczny kamień odpowiada na twój dotyk.");return true;
}
int bagCount(Kind kind)const{int count=0;for(const auto& value:bag)if(!value.empty()&&value.kind==kind)count+=value.count;return count;}
bool brewTowerPotion(int index,Kind output){
 if(!towerObjectReachable(index)||towerFloor!=2||towerFurnishings(towerFloor)[index].type!=7||(output!=Kind::Health&&output!=Kind::Mana))return false;
 Kind ingredient=output==Kind::Health?Kind::Mushroom:Kind::ForestEssence;
 if(bagCount(ingredient)<3){say(output==Kind::Health?"Potrzebujesz 3 borowików.":"Potrzebujesz 3 esencji lasu.");return false;}
 auto result=bag;int remaining=3;
 for(auto& value:result)if(value.kind==ingredient&&!value.empty()){int count=std::min(remaining,value.count);value.count-=count;remaining-=count;if(value.count==0)value={};if(!remaining)break;}
 int target=-1;for(int i=0;i<BagSize;i++)if(result[i].kind==output&&result[i].tier==0&&result[i].count<99){target=i;break;}
 if(target<0)for(int i=0;i<BagSize;i++)if(result[i].empty()){target=i;break;}
 if(target<0){say("Zwolnij pole lub miejsce w stosie mikstur.");return false;}
 if(result[target].empty())result[target]=item(output);else result[target].count++;
 bag=result;sound(Cue::Pickup,player);effects.push_back({towerFurnishings(towerFloor)[index].pos,"MIKSTURA GOTOWA",1.2f,2});say("Mikstura gotowa — znajdziesz ją w plecaku.");return true;
}
