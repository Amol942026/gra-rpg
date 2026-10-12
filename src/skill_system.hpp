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
bool skillWeaponMatches(Skill skill)const{if(skill==Skill::Healing)return true;return !equipment[0].empty()&&equipment[0].kind==(skill==Skill::StrongCut?Kind::Sword:Kind::Bow);}
bool skillAllowed(Skill skill)const{
 if(dead()||allocationRequired||storyBlocked()||rolling()||skillRank(skill)<=0||!skillWeaponMatches(skill))return false;
 if(skill==Skill::Healing)return healingCd<=0&&mana>=HealingCost&&hp<maxHp();
 if(inTower())return false;
 if(skill==Skill::StrongCut)return strongCutCd<=0&&attackCd<=0&&mana>=StrongCutCost;
 if(skill==Skill::PiercingArrow)return piercingArrowCd<=0&&mana>=PiercingArrowCost&&ammoCount()>0;
 return false;
}
bool strongSwingActive()const{return swingStrong&&swing>0;}
bool empoweredPose()const{return (equipment[0].kind==Kind::Sword&&strongSwingActive())||(equipment[0].kind==Kind::Bow&&(piercingArrowArmed||(lastBowPiercing&&bowReleaseTime>0)));}
bool strongCut(){
 if(!skillAllowed(Skill::StrongCut))return false;
 mana-=StrongCutCost;strongCutCd=StrongCutCooldown;attackCd=.6f;meleeDuration=StrongCutDuration;swing=StrongCutDuration;swingStrong=true;sound(Cue::Swing,player);
 // Capture damage before a kill can award a level during this same area cut.
 const float amount=strongCutDamage();const bool critical=rollCritical();
 for(auto& enemy:enemies)if(enemy.alive&&distance(player,enemy.pos)<=MeleeRange+.00001f)hit(enemy,amount,DamageType::Physical,critical);
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
