#pragma once
// Included inside Game: both allocation phases share the existing pause guard.
int skillRank(Skill skill)const{int i=int(skill);return i>=0&&i<SkillCount?skillRanks[i]:0;}
bool skillAllocationPending()const{return allocationRequired&&attributePoints==0&&skillPoints>0;}
bool trainSkills(const SkillRanks& points){
 if(dead()||!skillAllocationPending())return false;
 int total=0;for(int p:points){if(p<0||p>skillPoints)return false;total+=p;}
 if(total!=skillPoints||skillRanks[int(Skill::Healing)]+points[int(Skill::Healing)]>1)return false;
 for(int i=0;i<SkillCount;i++)skillRanks[i]+=points[i];
 skillPoints=0;allocationRequired=false;say("Umiejętności rozwinięte. Wyprawa trwa dalej.");sound(Cue::Ui,player);return true;
}
float strongCutMultiplier()const{return 1.5f+.1f*std::max(0,skillRank(Skill::StrongCut)-1);}
float strongCutDamage()const{return damage()*strongCutMultiplier();}
int piercingTargets()const{return 2+skillRank(Skill::PiercingArrow);}
Skill skillForSlot(int slot)const{
 if(slot!=0)return Skill(slot);
 switch(archetype(equipment[0].kind)){
 case Kind::Axe:return Skill::Cleave;
 case Kind::Dagger:return Skill::DoubleThrust;
 case Kind::Spear:return Skill::Skewer;
 default:return Skill::StrongCut;
 }
}
float skillCooldown(Skill skill)const{
 switch(skill){
 case Skill::StrongCut:return strongCutCd;
 case Skill::PiercingArrow:return piercingArrowCd;
 case Skill::Healing:return healingCd;
 default:return weaponSkillCds[int(skill)-SkillSlotCount];
 }
}
bool skillWeaponMatches(Skill skill)const{
 return skill==Skill::Healing||(!equipment[0].empty()&&archetype(equipment[0].kind)==skillProfile(skill).weapon);
}
bool skillAllowed(Skill skill)const{
 if(dead()||knockedBack()||allocationRequired||storyBlocked()||rolling()||skillRank(skill)<=0||!skillWeaponMatches(skill))return false;
 if(skill==Skill::Healing)return healingCd<=0&&mana>=HealingCost&&hp<maxHp();
 if(inTower()||sandTrapped())return false;
 if(skill==Skill::PiercingArrow)return piercingArrowCd<=0&&mana>=PiercingArrowCost&&ammoCount()>0;
 return skillCooldown(skill)<=0&&attackCd<=0&&mana>=skillProfile(skill).mana;
}
bool strongSwingActive()const{return swingStrong&&swing>0;}
bool empoweredPose()const{return strongSwingActive()||(archetype(equipment[0].kind)==Kind::Bow&&(piercingArrowArmed||(lastBowPiercing&&bowReleaseTime>0)));}
bool strongCut(V aim={}){
 if(!skillAllowed(Skill::StrongCut))return false;
 cancelMeleeHold();aimMelee(aim);faceThrust();
 animation.swordStow=0;
 mana-=StrongCutCost;strongCutCd=StrongCutCooldown;attackCd=.6f;meleeDuration=StrongCutDuration;swing=StrongCutDuration;swingStrong=true;sound(Cue::HeavySwing,player);
 hitMeleeArea(strongCutDamage(),true);
 return true;
}
bool useMeleeSkill(Skill skill,V aim){
 if(skill==Skill::StrongCut)return strongCut(aim);
 if(int(skill)<SkillSlotCount||!skillAllowed(skill))return false;
 const auto& profile=skillProfile(skill);
 cancelMeleeHold();aimMelee(aim);faceThrust();
 if(skill==Skill::Skewer)startSpear();else startAxeOrDagger(profile.weapon);
 swingStrong=true;mana-=profile.mana;
 weaponSkillCds[int(skill)-SkillSlotCount]=profile.cooldown;
 return true;
}
bool preparePiercingArrow(){
 if(piercingArrowArmed){cancelBow();say("Następny strzał będzie zwykły.");return true;}
 if(!skillAllowed(Skill::PiercingArrow))return false;
 cancelBow();piercingArrowArmed=true;say("Przeszywająca strzała: naciągnij i puść LPM (20 many).");sound(Cue::Ui,player);return true;
}

bool heal(){
 if(!skillAllowed(Skill::Healing))return false;
 cancelBow();float restored=std::min(HealingAmount,float(maxHp())-hp);
 mana-=HealingCost;hp+=restored;healingCd=HealingCooldown;healingPulse=HealingDuration;
 effects.push_back({player,"+"+std::to_string(int(std::round(restored)))+" HP",1.3f,1,false,false,110});
 sound(Cue::Potion,player);say("Leczenie — odzyskano "+std::to_string(int(std::round(restored)))+" HP.");return true;
}
