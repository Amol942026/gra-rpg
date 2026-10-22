#pragma once
#include "save_game.hpp"

enum class MenuPage {Home,Load,Save,Options,Credits,Exit,Overwrite,ReplaceSession};
MenuPage menuPage=MenuPage::Home;
bool exitRequested=false;
int menuSelection=0,menuSlot=0;
float menuClock=0;
std::string menuNotice;
std::array<ember::saves::Entry,3> menuSaves{};
// Empty in production. Native QA supplies its own isolated save directory.
fs::path menuSaveRoot;
ember::saves::Store menuStore(){return menuSaveRoot.empty()?ember::saves::Store{}:ember::saves::Store{menuSaveRoot};}
bool gameMenuActive(){return welcome||paused;}
void menuPageTo(MenuPage page){menuPage=page;menuSelection=0;menuNotice.clear();}
void refreshMenuSaves(){
 try{auto store=menuStore();for(int i=0;i<3;i++)menuSaves[i]=store.inspect(i);}catch(...){menuSaves={};menuNotice="Katalog zapisów jest niedostępny.";}
}
void clearTransientUi(){
 dragging={};inventoryClicks.clear();audioOpen=statsOpen=skillsOpen=spellBookOpen=weaponWheelOpen=help=false;
 audioDragging=-1;towerPanelOpen=false;towerPanelObject=-1;towerPage=nearbyPage=corpsePage=0;corpsePageOwner=-1;hovered={};
 statDraft={};skillDraft={};draftForLevel=skillDraftLevel=-1;
}
void openPauseMenu(){game.cancelBow();clearTransientUi();paused=true;menuPageTo(MenuPage::Home);}
void resumeFromMenu(){paused=false;menuPageTo(MenuPage::Home);}
void menuNewGame(){game=Game(std::random_device{}());clearTransientUi();menuPageTo(MenuPage::Home);startIntro();}
void menuSave(){
 try{menuStore().save(menuSlot,{game,intro,towerStory,{}});menuPageTo(MenuPage::Save);refreshMenuSaves();menuNotice="Zapisano wyprawę w slocie "+std::to_string(menuSlot+1)+".";}
 catch(const std::exception&){menuNotice="Nie udało się zapisać. Sprawdź miejsce na dysku i dostęp do folderu zapisów.";}
}
void menuLoad(){
 try{
  bool backup=false;auto saved=menuStore().load(menuSlot,&backup);saved.game.limitForestForage();
  clearTransientUi();game=std::move(saved.game);intro=saved.intro;towerStory=saved.tower;
  // Never replay queued sound events from the frame before saving.
  game.sounds.clear();developerActive=developerMenuOpen=false;developerError.clear();welcome=false;paused=true;menuPageTo(MenuPage::Home);
  menuNotice=backup?"Odzyskano poprzedni zapis z kopii zapasowej. ESC — kontynuuj.":"Wczytano wyprawę. ESC — kontynuuj.";
 }catch(const std::exception&){menuNotice="Nie udało się wczytać. Bieżąca wyprawa pozostała bez zmian.";}
}
void menuBack(){
 if(menuPage==MenuPage::Home){if(paused)resumeFromMenu();return;}
 if(menuPage==MenuPage::Overwrite)menuPageTo(MenuPage::Save);
 else if(menuPage==MenuPage::ReplaceSession)menuPageTo(MenuPage::Load);
 else menuPageTo(MenuPage::Home);
}
Rectangle menuHomeButton(int i){return welcome?Rectangle{946,309.f+i*75,364,58}:Rectangle{518,285.f+i*78,404,60};}
Rectangle menuBackButton(){return {559,718,322,46};}
Rectangle menuSlotButton(int i){return {380,300.f+i*116,680,96};}
Rectangle menuOption(int i){return {395,292.f+i*72,650,54};}
Rectangle menuConfirmButton(int i){return {405.f+i*325,532,305,58};}
void menuHomeAction(int index){
 if(welcome){if(index==0){menuNewGame();return;}if(index==1){menuPageTo(MenuPage::Load);refreshMenuSaves();return;}if(index==2)menuPageTo(MenuPage::Options);if(index==3)menuPageTo(MenuPage::Credits);if(index==4)menuPageTo(MenuPage::Exit);}
 else{if(index==0){menuPageTo(MenuPage::Save);refreshMenuSaves();return;}if(index==1){menuPageTo(MenuPage::Load);refreshMenuSaves();return;}if(index==2)menuPageTo(MenuPage::Options);if(index==3)menuPageTo(MenuPage::Exit);}
}
void activateMenuRow(int row){
 if(menuPage==MenuPage::Home){menuHomeAction(row);return;}
 if(menuPage==MenuPage::Load||menuPage==MenuPage::Save){
  if(row==3){menuBack();return;}menuSlot=row;
  if(menuPage==MenuPage::Save){if(menuSaves[row].exists)menuPageTo(MenuPage::Overwrite);else menuSave();}
  else if(!menuSaves[row].valid)menuNotice=menuSaves[row].exists?"Zapis jest uszkodzony lub pochodzi z niezgodnej wersji gry.":"Ten slot jest pusty. Najpierw rozpocznij nową grę.";
  else if(welcome)menuLoad();else menuPageTo(MenuPage::ReplaceSession);
  return;
 }
 if(menuPage==MenuPage::Options){
  if(row==0)audioSystem.toggleMusic();if(row==1)audioSystem.toggleEffects();if(row==2)toggleFullScreen();if(row==3)audioSystem.toggleMute();if(row==4)menuBack();return;
 }
 if(menuPage==MenuPage::Credits){menuBack();return;}
 if(row==1){menuBack();return;}
 if(menuPage==MenuPage::Exit)exitRequested=true;
 else if(menuPage==MenuPage::Overwrite)menuSave();
 else if(menuPage==MenuPage::ReplaceSession)menuLoad();
}
int menuRowCount(){switch(menuPage){case MenuPage::Home:return 5;case MenuPage::Load:case MenuPage::Save:return 4;case MenuPage::Options:return 5;case MenuPage::Credits:return 1;default:return 2;}}
Rectangle menuRowRect(int i){
 if(menuPage==MenuPage::Home)return !welcome&&i==4?menuBackButton():menuHomeButton(i);
 if(menuPage==MenuPage::Load||menuPage==MenuPage::Save)return i==3?menuBackButton():menuSlotButton(i);
 if(menuPage==MenuPage::Options)return i==4?menuBackButton():menuOption(i);
 if(menuPage==MenuPage::Credits)return menuBackButton();return menuConfirmButton(i);
}
// Shared event dispatcher makes mouse and keyboard use precisely the same actions.
bool gameMenuEvent(bool escape,int vertical,bool accept,bool click,int horizontal=0){
 if(!gameMenuActive()){if(escape){openPauseMenu();return true;}return false;}
 if(escape){menuBack();return true;}
 int count=menuRowCount();if(vertical)menuSelection=(menuSelection+vertical+count)%count;
 if(menuPage==MenuPage::Options&&horizontal&&menuSelection<2){float& value=menuSelection==0?audioSystem.musicVolume:audioSystem.effectsVolume;value=std::clamp(value+horizontal*.05f,0.f,1.f);}
 if(click){for(int i=0;i<count;i++)if(inside(menuRowRect(i))){menuSelection=i;
   if(menuPage==MenuPage::Options&&i<2&&mouse.x>=640){float& value=i==0?audioSystem.musicVolume:audioSystem.effectsVolume;value=std::clamp((mouse.x-660)/280,0.f,1.f);return true;}
   if(menuPage==MenuPage::Home&&!welcome&&i==4)resumeFromMenu();else activateMenuRow(i);return true;}}
 if(accept){if(menuPage==MenuPage::Home&&!welcome&&menuSelection==4)resumeFromMenu();else activateMenuRow(menuSelection);}
 return true;
}
bool menuInput(float dt){
 menuClock+=dt;
 if(!IsWindowFocused()){if(!welcome&&!paused)openPauseMenu();return true;}
 if(gameMenuActive()&&(GetMouseDelta().x!=0||GetMouseDelta().y!=0))for(int i=0;i<menuRowCount();i++)if(inside(menuRowRect(i)))menuSelection=i;
 bool handled=gameMenuEvent(IsKeyPressed(KEY_ESCAPE),(IsKeyPressed(KEY_DOWN)||IsKeyPressed(KEY_TAB))?1:IsKeyPressed(KEY_UP)?-1:0,IsKeyPressed(KEY_ENTER)||IsKeyPressed(KEY_KP_ENTER),IsMouseButtonPressed(MOUSE_BUTTON_LEFT),IsKeyPressed(KEY_RIGHT)?1:IsKeyPressed(KEY_LEFT)?-1:0);
 if(handled&&menuPage==MenuPage::Options&&IsMouseButtonDown(MOUSE_BUTTON_LEFT))for(int i=0;i<2;i++)if(inside(menuOption(i))&&mouse.x>=660&&mouse.x<=940){float& value=i==0?audioSystem.musicVolume:audioSystem.effectsVolume;value=std::clamp((mouse.x-660)/280,0.f,1.f);}
 return handled;
}
void menuButton(Rectangle r,const std::string& label,int row,bool enabled=true){
 if(welcome&&menuPage==MenuPage::Home){
  bool active=menuSelection==row;
  if(active){DrawRectangleGradientH(int(r.x),int(r.y),int(r.width),int(r.height),rgb(124,105,64,90),rgb(48,66,62,25));line(r.x,r.y,r.x,r.y+r.height,Gold,2);}
  line(r.x+14,r.y+r.height+5,r.x+r.width-10,r.y+r.height+5,Fade(Gold,active?.42f:.13f));
  txt(TextFormat("%02d",row+1),r.x+18,r.y+20,14,active?Gold:shade(Muted,.65f),true);
  txt(label,r.x+63,r.y+17,23,active?rgb(251,226,175):Text);
  if(active)diamond(r.x+r.width-16,r.y+r.height/2,3,6,Gold);
  return;
 }
 bool selected=menuSelection==row;panel(r,selected?rgb(44,52,49):rgb(20,29,32,244));
 DrawRectangleLinesEx(r,1,enabled&&selected?Gold:Line);
 if(selected&&enabled){rect(r.x,r.y,3,r.height,Gold);diamond(r.x+22,r.y+r.height*.5f,3,6,Gold);}
 center(label,r.x+r.width*.5f,r.y+r.height*.5f-12,22,enabled?selected?Gold:Text:Muted);
}
void menuHeading(const std::string& title,const std::string& subtitle){
 panel({320,158,800,626},rgb(15,24,29,250));DrawRectangleLinesEx({332,170,776,602},1,Fade(Gold,.3f));
 line(365,201,690,201,Fade(Gold,.65f));diamond(720,201,7,11,Gold);line(750,201,1075,201,Fade(Gold,.65f));
 center(title,720,221,32,Text,true);center(subtitle,720,268,16,Muted);
}
void drawGameMenu(){
 if(!gameMenuActive())return;
 if(welcome){
  rect(0,0,W,H,rgb(6,12,21,35));
  DrawRectangleGradientH(0,0,780,H,rgb(5,13,23,100),rgb(5,13,23,0));
  if(menuPage==MenuPage::Home){
  txt("EMBERFALL",96,95,92,rgb(245,218,166),true);
  line(103,212,652,212,Fade(Gold,.6f));diamond(677,212,4,7,Gold);
  txt("ECHA ZAPOMNIANYCH RUIN",106,233,19,rgb(195,212,198));
  txt("PIĘĆ CZĘŚCI. JEDNA NADZIEJA.",102,723,23,Gold,true);
  txt("Gdy gasną pieczęcie, ktoś musi podtrzymać światło.",105,768,18,Text);
  }
 }else rect(0,0,W,H,rgb(5,12,19,211));
 if(menuPage==MenuPage::Home){
  Rectangle frame=welcome?Rectangle{906,174,444,585}:Rectangle{464,158,512,626};panel(frame,rgb(10,24,29,237));DrawRectangleLinesEx({frame.x+10,frame.y+10,frame.width-20,frame.height-20},1,Fade(Gold,.25f));
  float cx=frame.x+frame.width/2;diamond(cx,frame.y+38,5,9,Gold);
  center(welcome?"OSTATNIA PIECZĘĆ":"CHWILA ODDECHU",cx,frame.y+62,welcome?25:29,Text,true);
  if(welcome)center("Twoja opowieść zaczyna się tutaj",cx,frame.y+100,16,Muted);
  else center(intro.active()?"Opowieść Eldrina":game.inTower()?"Schronienie w wieży Eldrina":game.inDesert()?"Morze Piasków":"Las Mchu",cx,frame.y+108,16,Muted);
  const char* mainLabels[]={"Nowa gra","Wczytaj grę","Opcje","Twórcy","Wyjdź z gry"};
  const char* pauseLabels[]={"Zapisz grę","Wczytaj grę","Opcje","Wyjdź z gry"};
  for(int i=0;i<(welcome?5:4);i++)menuButton(menuHomeButton(i),welcome?mainLabels[i]:pauseLabels[i],i);
  if(!welcome)menuButton(menuBackButton(),"Wróć do gry · ESC",4);
 }else if(menuPage==MenuPage::Load||menuPage==MenuPage::Save){
  bool save=menuPage==MenuPage::Save;menuHeading(save?"KRONIKA WĘDROWCA":"POWRÓT DO KRONIKI",save?"Wybierz miejsce zapisu wyprawy":"Wybierz zapisaną wyprawę");
  for(int i=0;i<3;i++){auto r=menuSlotButton(i);auto& s=menuSaves[i];bool active=menuSelection==i;
   panel(r,active?rgb(40,50,47):Card);DrawRectangleLinesEx(r,1,active?Gold:Line);txt("0"+std::to_string(i+1),r.x+20,r.y+22,30,Gold,true);
   txt(s.valid?s.location:s.exists?"Zapis niedostępny":"Pusty slot",r.x+87,r.y+15,22,s.valid||save?Text:Muted);
   txt(s.valid?"Poziom "+std::to_string(s.level)+"  ·  "+s.date+(s.backup?"  ·  kopia zapasowa":""):s.exists?"Plik zachowano na dysku":save?"Zapisz tutaj bieżącą wyprawę":"Brak zapisanej wyprawy",r.x+88,r.y+54,16,Muted);
  }menuButton(menuBackButton(),"Wstecz · ESC",3);
 }else if(menuPage==MenuPage::Options){
  menuHeading("PRZY OGNISKU","Dostosuj oprawę swojej wyprawy");
  for(int i=0;i<4;i++){auto r=menuOption(i);panel(r,menuSelection==i?rgb(40,50,47):Card);DrawRectangleLinesEx(r,1,menuSelection==i?Gold:Line);
   txt(i==0?"Muzyka":i==1?"Efekty dźwiękowe":i==2?"Tryb ekranu":"Wyciszenie",r.x+15,r.y+14,19,Text);
   if(i<2){float v=i==0?audioSystem.musicVolume:audioSystem.effectsVolume;bool enabled=i==0?audioSystem.musicEnabled():audioSystem.effectsEnabled();
    txt(enabled?"WŁ.":"WYŁ.",r.x+185,r.y+17,14,enabled?Green:Muted);rect(660,r.y+25,280,5,Line);rect(660,r.y+25,280*v,5,Gold);DrawCircleV({660+280*v,r.y+27},6,Text);txt(TextFormat("%d%%",int(v*100+.5f)),970,r.y+15,17,Gold);
   }else center(i==2?(fullScreen()?"Pełny ekran · F11":"Okno · F11"):(audioSystem.muted?"Włączone":"Wyłączone"),874,r.y+16,17,Gold);
  }
  center("Kliknij nazwę, aby przełączyć kanał. Suwak / ← →: głośność.",720,609,15,Muted);
  center("F1 podczas gry: przewodnik oraz opis sterowania.",720,642,15,Muted);
  menuButton(menuBackButton(),"Gotowe · ESC",4);
 }else if(menuPage==MenuPage::Credits){
  menuHeading("TWÓRCY","Ludzie i narzędzia, dzięki którym płonie żar");
  center("EMBERFALL — ECHA ZAPOMNIANYCH RUIN",720,325,23,Gold,true);
  center("Autorski projekt Amadeusza · realizacja z pomocą Codexa",720,373,20,Text);
  center("Grafika i animacja: Krita · Synfig Studio · Laigter",720,430,19,Text);
  center("Muzyka i dźwięk: LMMS · Csound · Audacity",720,466,19,Text);
  center("Silnik: raylib — Ramon Santamaria i współtwórcy",720,523,18,Muted);
  center("Cinzel — Natanael Gama  ·  Lato — Łukasz Dziedzic",720,558,17,Muted);
  center("Manrope — Mikhail Sharanda i współtwórcy",720,591,17,Muted);
  center("Pełne informacje o licencjach: LICENSES.md oraz assets/licenses",720,650,15,Gold);
  menuButton(menuBackButton(),"Wstecz · ESC",0);
 }else{
  bool exiting=menuPage==MenuPage::Exit,overwriting=menuPage==MenuPage::Overwrite;
  menuHeading(exiting?"ŻAR POCZEKA":overwriting?"NADPISAĆ KRONIKĘ?":"WCZYTAĆ WYPRAWĘ?",exiting?"Zakończyć grę?":overwriting?"Slot "+std::to_string(menuSlot+1)+" zawiera już zapis":"Powrót do zapisanej wyprawy");
  center(welcome?"Do świata Emberfall możesz wrócić w każdej chwili.":overwriting?"Poprzedni zapis zachowamy jako kopię zapasową.":"Niezapisany postęp bieżącej wyprawy zostanie utracony.",720,399,19,Text);
  center(exiting&&!welcome?"Wybierz Anuluj, aby wrócić i zapisać grę.":"",720,439,17,Muted);
  menuButton(menuConfirmButton(0),exiting?"Wyjdź z gry":overwriting?"Zapisz grę":"Wczytaj grę",0);
  menuButton(menuConfirmButton(1),"Anuluj · ESC",1);
 }
 if(!menuNotice.empty())center(menuNotice,720,805,16,Gold);
 rect(0,846,W,54,rgb(6,12,21,245));line(80,849,1360,849,Fade(Gold,.3f));txt("PIECZĘĆ POPIOŁU",82,867,14,Muted);center("↑ ↓ / TAB — wybierz    ·    ENTER / LPM — zatwierdź",720,867,14,Muted);txt("EMBERFALL",1259,867,14,Gold,true);
}
