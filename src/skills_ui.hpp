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
 center(game.skillAllocationPending()?"Kliknij umiejętność, aby przypisać punkt. COFNIJ anuluje wybór.":"U lub ESC zamyka okno. Leczenie wystarczy poznać raz.",720,647,16,Muted);
 bool ready=!game.skillAllocationPending()||skillDraftRemaining()==0;panel(skillsConfirm(),ready&&inside(skillsConfirm())?rgb(53,73,57):Card);
 center(game.skillAllocationPending()?"ZATWIERDŹ I WRÓĆ DO GRY":"WRÓĆ DO GRY",720,690,18,ready?Gold:Muted);
 center(game.skillAllocationPending()?"Gra pozostaje zatrzymana do rozdania punktów umiejętności.":"Z / X — umiejętności broni · V — Leczenie",720,749,14,Muted);
}
void drawSkillHud(){
 const char* keys[]={"Z · 10 MP","X · 20 MP","V · 20 MP"};const char* labels[]={"CIĘCIE","STRZAŁA","LECZENIE"};
 for(int i=0;i<SkillCount;i++){
  Skill skill=static_cast<Skill>(i);Rectangle button=skillActionRect(i);bool learned=game.skillRank(skill)>0,weapon=game.skillWeaponMatches(skill),ready=game.skillAllowed(skill),armed=i==1&&game.piercingArrowArmed;
  panel(button,armed?rgb(31,62,66):Panel);DrawRectangleLinesEx(button,1,armed?rgb(126,215,232):ready&&inside(button)?Gold:Line);
  skillIcon(skill,button.x+33,button.y+23,.72f,learned&&weapon?1.f:.3f);
  if(!learned){
   float x=button.x+33,y=button.y+23;
   ellipseOutline(x,y-4,5,6,Muted);
   rect(x-8,y-3,16,13,Panel);DrawRectangleLinesEx({x-8,y-3,16,13},1,Muted);
   rect(x-1,y,2,5,Muted);
  }
  float cooldown=i==0?game.strongCutCd/StrongCutCooldown:i==1?game.piercingArrowCd/PiercingArrowCooldown:game.healingCd/HealingCooldown;
  if(cooldown>0)rect(button.x+1,button.y+1,button.width-2,(button.height-2)*std::clamp(cooldown,0.f,1.f),Fade(Ink,.65f));
  center(keys[i],button.x+32,button.y+46,14,learned&&weapon?Gold:Muted);
  center(learned?labels[i]:"ZABLOK.",button.x+32,870,learned?14:12,Muted);
  if(!inside(button)||welcome||help||paused||statsOpen||skillsOpen||spellBookOpen||weaponWheelOpen||intro.active()||towerStory.active()||towerPanelOpen||developerMenuOpen)continue;
  float x=button.x,y=635;panel({x,y,356,147},rgb(12,22,25,250));txt(skillName(skill),x+15,y+13,20,Text);
  txt(TextFormat("%s · Koszt %d many",i==2?(learned?"Poznane":"Nieznane"):TextFormat("Ranga %d",game.skillRank(skill)),i==0?10:20),x+15,y+46,15,Gold);
  txt(!learned?"Naucz się po awansie (U).":!weapon?(i==0?"Zablokowane: wymagany miecz.":"Zablokowane: wymagany łuk."):i==0?"Z: wykonaj mocne cięcie.":i==1?"X: przygotuj, LPM: naciągnij i puść.":"V: przywróć 30 HP, niezależnie od broni.",x+15,y+75,15,Muted);
  txt(i==0?"150% obrażeń na randze 1.":i==1?"Obrażenia takie jak zwykły strzał.":"Pełne HP: nie zużywa many.",x+15,y+104,15,i==2?Green:Gold);
  if(armed)txt("Przeszywająca strzała gotowa",x+15,y+128,14,Green);
 }
}
