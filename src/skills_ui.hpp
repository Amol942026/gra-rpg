#pragma once
Rectangle skillCard(int index){return {312,237.f+index*133,816,126};}
Rectangle skillAdjust(int index,bool add){Rectangle card=skillCard(index);return {add?1056.f:984.f,card.y+74,64,32};}
Rectangle skillsConfirm(){return {492,677,456,46};}
int skillDraftRemaining(){return game.skillPoints-std::accumulate(skillDraft.begin(),skillDraft.end(),0);}
void openSkills(){
 invalidatePausedWorld();
 if(game.dead())return;
 if(game.allocationRequired&&game.attributePoints>0){openStats();return;}
 game.cancelBow();weaponWheelOpen=statsOpen=spellBookOpen=false;skillsOpen=true;skillDraft={};skillDraftLevel=game.level;dragging={};
}
bool adjustSkill(int index,bool add){
 if(!game.skillAllocationPending()||index<0||index>=SkillCount)return false;
 if(add){if(skillDraftRemaining()<=0||(index==int(Skill::Healing)&&game.skillRank(Skill::Healing)+skillDraft[index]>=1))return false;skillDraft[index]++;}
 else{if(skillDraft[index]<=0)return false;skillDraft[index]--;}
 audioSystem.play(Cue::Ui);return true;
}
bool confirmSkills(){
 if(game.skillAllocationPending()&&!game.trainSkills(skillDraft))return false;
 if(game.allocationRequired)return false;
 skillsOpen=false;skillDraft={};skillDraftLevel=-1;audioSystem.play(Cue::Ui);return true;
}
bool clickSkills(){
 for(int i=0;i<SkillCount;i++){
  for(bool add:{false,true})if(inside(skillAdjust(i,add))){adjustSkill(i,add);return true;}
  if(inside(skillCard(i))){
   if(i==int(Skill::Healing)&&game.skillRank(Skill::Healing)>=1)return true;
   if(game.skillPoints==1&&skillDraftRemaining()==0&&skillDraft[i]==0)skillDraft={};
   adjustSkill(i,true);return true;
  }
 }
 if(inside(skillsConfirm()))confirmSkills();return true;
}
std::array<RenderTexture2D,SkillCount> skillIconArt{};
void skillIcon(Skill skill,float x,float y,float size=1,float alpha=1){
 const auto& art=skillIconArt[int(skill)];
 if(art.id){DrawTexturePro(art.texture,{0,0,128,-128},{x-32*size,y-32*size,64*size,64*size},{0,0},0,Fade(WHITE,alpha));return;}
 if(skill==Skill::Healing){
  Color c=Fade(rgb(129,235,161),alpha);ellipse(x,y,23*size,23*size,Fade(rgb(38,94,63),alpha));
  ellipseOutline(x,y,23*size,23*size,c);rect(x-5*size,y-16*size,10*size,32*size,c);rect(x-16*size,y-5*size,32*size,10*size,c);
  for(int i=0;i<4;i++)diamond(x+std::cos(i*PI/2+PI/4)*28*size,y+std::sin(i*PI/2+PI/4)*28*size,3*size,4*size,Fade(Gold,alpha));return;
 }
 Color color=skill==Skill::StrongCut?rgb(255,163,83):rgb(126,215,232);
 icon(item(skill==Skill::StrongCut?Kind::Sword:Kind::Arrow),x,y,size,alpha);
 if(skill==Skill::StrongCut){for(int i=0;i<3;i++)line(x+(-19+i*6)*size,y+(9+i*3)*size,x+(8+i*6)*size,y+(-18+i*3)*size,Fade(color,alpha*.7f),2*size);}
 else{for(int i=0;i<3;i++)DrawCircleLines(int(x+(-17+i*17)*size),int(y+(9-i*9)*size),7*size,Fade(color,alpha*.65f));}
}
void makeSkillArt(){
 // Rasterize the new HUD icons once; attacks and projectiles remain animated geometry.
 for(int i=0;i<SkillCount;i++){
  RenderTexture2D art=LoadRenderTexture(128,128);if(!art.id)continue;
  BeginTextureMode(art);ClearBackground(BLANK);BeginMode2D({{0,0},{0,0},0,2});
  skillIcon(static_cast<Skill>(i),32,32);EndMode2D();EndTextureMode();
  SetTextureFilter(art.texture,TEXTURE_FILTER_BILINEAR);skillIconArt[i]=art;
 }
}
void unloadSkillArt(){for(auto& art:skillIconArt){if(art.id)UnloadRenderTexture(art);art={};}}
void drawSkillsMenu(){
 rect(0,0,W,H,rgb(6,15,18,215));panel({280,100,880,682},Panel);line(310,121,1130,121,Gold);
 center(game.skillAllocationPending()?TextFormat("UMIEJĘTNOŚCI — POZIOM %d",game.level):"UMIEJĘTNOŚCI BOHATERA",720,147,28,Text,true);
 center("1 punkt za każdy zdobyty poziom",720,190,17,Muted);
 center(game.skillAllocationPending()?TextFormat("Punkty do rozdania: %d / %d",skillDraftRemaining(),game.skillPoints):"Zdobywaj poziomy, aby uczyć się i rozwijać umiejętności.",720,213,17,Gold);
 const char* costs[]={"Miecz · 10 many · Z","Łuk · 20 many + 1 strzała · X","Każda broń · 20 many · V"};
 const char* descriptions[]={"Silniejszy zamach trafia wszystkich w zasięgu miecza.","Przygotuj X, następnie naciągnij i puść LPM.","Przywraca 30 HP, do maksymalnego zdrowia."};
 const char* details[]={"Kolejna ranga: +10% obrażeń · odnowienie 1,2 s.","Kolejna ranga: +1 cel · odnowienie 1,5 s.","Jedna ranga · odnowienie 1 s · przy pełnym HP bez kosztu."};
 for(int i=0;i<SkillCount;i++){
  Skill skill=static_cast<Skill>(i);Rectangle card=skillCard(i);int rank=game.skillRank(skill)+skillDraft[i];
  panel(card,inside(card)?Card:Panel);DrawRectangleLinesEx(card,1,skillDraft[i]?Green:Line);
  skillIcon(skill,363,card.y+64,1.15f);txt(skillName(skill),415,card.y+7,24,Text,true);
  txt(costs[i],415,card.y+38,15,Gold);txt(descriptions[i],415,card.y+61,15,Muted);
  txt(i==0?TextFormat("Obrażenia: %.0f%% zwykłego ataku",(1.5f+.1f*std::max(0,rank-1))*100):i==1?TextFormat("Przebija do %d celów · obrażenia zwykłej strzały",2+std::max(1,rank)):"Efekt: +30 HP · nie wymaga miecza ani łuku",415,card.y+83,15,i==2?rgb(136,239,158):Gold);
  txt(details[i],415,card.y+105,14,Muted);
  center(i==2?(rank?"POZNANE":"NIEZNANE"):TextFormat("RANGA %d",rank),1048,card.y+37,15,skillDraft[i]?Green:rank?Gold:Muted);
  for(bool add:{false,true}){Rectangle button=skillAdjust(i,add);bool enabled=game.skillAllocationPending()&&(add?skillDraftRemaining()>0&&(i!=2||rank<1):skillDraft[i]>0);if(enabled){panel(button,inside(button)?Card:Panel);center(add?"DODAJ":"COFNIJ",button.x+button.width/2,button.y+9,13,Gold);}}
 }
 if(game.skillAllocationPending())center("Kliknij umiejętność, aby przypisać punkt. COFNIJ anuluje wybór.",720,647,16,Muted);
 bool ready=!game.skillAllocationPending()||skillDraftRemaining()==0;panel(skillsConfirm(),ready&&inside(skillsConfirm())?rgb(53,73,57):Card);
 center(game.skillAllocationPending()?"ZATWIERDŹ I WRÓĆ DO GRY":"WRÓĆ DO GRY",720,690,18,ready?Gold:Muted);
 if(game.skillAllocationPending())center("Gra pozostaje zatrzymana do rozdania punktów umiejętności.",720,749,14,Muted);
}
constexpr const char* skillSlotKeys[]={"Z","X","V"};
constexpr const char* skillSlotCategories[]={"W ZWARCIU","DYSTANSOWE","POMOCNICZE"};
struct PotionPickerChoices {
 std::array<Kind,std::size(PotionTypes)+1> kinds{};
 int count=0;
};
PotionPickerChoices potionPickerChoices(int slot){
 PotionPickerChoices choices;
 for(const auto& type:PotionTypes)if(type.slot==slot)choices.kinds[choices.count++]=type.kind;
 choices.kinds[choices.count++]=Kind::None;
 return choices;
}
Rectangle quickPickerBounds(int slot,int rowCount){
 const float height=78.f+57.f*rowCount;
 return {quickPickerTile(slot).x,792.f-height,350,height};
}
Rectangle quickPickerOption(Rectangle bounds,int row){
 return {bounds.x+12,bounds.y+52.f+57.f*row,bounds.width-24,49};
}
bool quickPickerEvent(bool dismiss,bool click){
 if(quickPickerSlot<0)return false;
 if(dismiss){quickPickerSlot=-1;return true;}
 if(!click)return true;
 const int slot=quickPickerSlot;
 for(int i=0;i<QuickPickerSlotCount;i++)if(inside(quickPickerTile(i))){
  quickPickerSlot=i==slot?-1:i;return true;
 }
 const bool skill=slot<SkillCount;
 const auto potions=potionPickerChoices(slot-SkillCount);
 const int count=skill?2:potions.count;
 const auto bounds=quickPickerBounds(slot,count);
 for(int row=0;row<count;row++)if(inside(quickPickerOption(bounds,row))){
  if(skill){
   if(row==0&&game.skillRank(Skill(slot))<=0)return true;
   game.skillAssigned[slot]=row==0;
  }else game.potionAssigned[slot-SkillCount]=potions.kinds[row];
  quickPickerSlot=-1;audioSystem.play(Cue::Ui);return true;
 }
 if(!inside(bounds))quickPickerSlot=-1;
 return true;
}
void drawQuickPicker(){
 if(quickPickerSlot<0)return;
 const int slot=quickPickerSlot;
 const bool isSkill=slot<SkillCount;
 const int potionSlot=slot-SkillCount;
 const auto potions=potionPickerChoices(potionSlot);
 const int count=isSkill?2:potions.count;
 const auto bounds=quickPickerBounds(slot,count);
 panel(bounds,rgb(14,23,28,252));DrawRectangleLinesEx(bounds,1,Fade(Gold,.8f));
 const char* category=isSkill?skillSlotCategories[slot]:potionSlot==0?"MIKSTURY ZDROWIA":"MIKSTURY MANY";
 const char* key=isSkill?skillSlotKeys[slot]:potionSlot==0?"1":"2";
 txt(category,bounds.x+16,bounds.y+13,17,Gold,true);
 txt(key,bounds.x+bounds.width-31,bounds.y+14,17,Gold);
 line(bounds.x+12,bounds.y+42,bounds.x+bounds.width-12,bounds.y+42,Line);
 for(int i=0;i<count;i++){
  const auto row=quickPickerOption(bounds,i);
  const Kind potion=isSkill?Kind::None:potions.kinds[i];
  const bool hasItem=isSkill?i==0:potion!=Kind::None;
  const bool enabled=!isSkill||!hasItem||game.skillRank(Skill(slot))>0;
  const bool selected=isSkill?game.skillAssigned[slot]==hasItem:game.potionAssigned[potionSlot]==potion;
  panel(row,enabled&&inside(row)?rgb(45,56,52):Card);
  DrawRectangleLinesEx(row,1,selected?Gold:Line);
  if(!hasItem)line(row.x+15,row.y+25,row.x+29,row.y+25,Muted,2);
  else if(isSkill)skillIcon(Skill(slot),row.x+22,row.y+25,.53f,enabled?1.f:.42f);
  else icon(item(potion),row.x+22,row.y+25,.62f);
  const std::string title=!hasItem?"Bez przypisania":isSkill?skillName(Skill(slot)):itemName(item(potion));
  txt(title,row.x+47,row.y+4,17,enabled?Text:Muted);
  const char* detail=!enabled?"Zablokowana · poznaj w menu U":selected?"Przypisane":hasItem?"Przypisz do kafelka":"Wyłącz skrót dla tej kategorii";
  txt(detail,row.x+47,row.y+25,14,enabled&&selected?Gold:Muted);
 }
 txt("ESC — zamknij",bounds.x+15,bounds.y+bounds.height-25,14,Muted);
}
void drawSkillHud(){
 const char* labels[]={"CIĘCIE","STRZAŁA","LECZENIE"};
 for(int i=0;i<SkillCount;i++){
  Skill skill=Skill(i);Rectangle button=skillActionRect(i);const bool assigned=game.skillAssigned[i];
  bool learned=game.skillRank(skill)>0,weapon=game.skillWeaponMatches(skill);
  bool armed=assigned&&skill==Skill::PiercingArrow&&game.piercingArrowArmed;
  const bool highlighted=quickPickerSlot==i||(gameplayInputAllowed()&&inside(button));
  panel(button,armed?rgb(31,62,66):highlighted?Card:Panel);
  DrawRectangleLinesEx(button,1,armed?rgb(126,215,232):highlighted?Gold:Line);
  if(assigned){
   skillIcon(skill,button.x+33,button.y+23,.72f,learned&&weapon?1.f:.3f);
   if(!learned){
    float x=button.x+33,y=button.y+23;
    ellipseOutline(x,y-4,5,6,Muted);
    rect(x-8,y-3,16,13,Panel);DrawRectangleLinesEx({x-8,y-3,16,13},1,Muted);
    rect(x-1,y,2,5,Muted);
   }
   float cooldown=i==0?game.strongCutCd/StrongCutCooldown:i==1?game.piercingArrowCd/PiercingArrowCooldown:game.healingCd/HealingCooldown;
   if(cooldown>0)rect(button.x+1,button.y+1,button.width-2,(button.height-2)*std::clamp(cooldown,0.f,1.f),Fade(Ink,.65f));
  }else line(button.x+24,button.y+23,button.x+42,button.y+23,Muted,2);
  center(assigned?TextFormat("%s · %d MP",skillSlotKeys[i],i==0?10:20):skillSlotKeys[i],button.x+32,button.y+46,14,assigned&&learned&&weapon?Gold:Muted);
  center(assigned?(learned?labels[i]:"ZABLOK."):"PUSTY",button.x+32,870,14,Muted);
  if(!inside(button)||!gameplayInputAllowed())continue;
  const float x=button.x,y=608;
  panel({x,y,378,174},rgb(12,22,25,250));
  txt(assigned?skillName(skill):"Bez przypisania",x+15,y+13,20,Text);
  if(assigned){
   txt(TextFormat("%s · Koszt %d many",i==2?(learned?"Poznane":"Nieznane"):TextFormat("Ranga %d",game.skillRank(skill)),i==0?10:20),x+15,y+45,15,Gold);
   txt(!learned?"Naucz się po awansie (U).":!weapon?(i==0?"Wymagany miecz.":"Wymagany łuk."):i==0?"Z wykonuje mocne cięcie.":i==1?"X przygotowuje strzałę. Naciągnij i puść LPM.":"V przywraca do 30 HP.",x+15,y+72,15,Muted);
   txt(i==0?"150% obrażeń na randze 1.":i==1?"Obrażenia takie jak zwykły strzał.":"Pełne HP: bez kosztu many.",x+15,y+98,15,i==2?Green:Gold);
   if(armed)txt("Przeszywająca strzała gotowa",x+15,y+146,14,Green);
  }
  txt("LPM — wybierz przypisanie",x+15,y+(assigned?121:45),15,Gold);
 }
}
