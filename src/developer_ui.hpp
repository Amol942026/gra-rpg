#pragma once
#ifdef EMBERFALL_PLAYER_BUILD
// No menu, shortcut handler or hit target is linked into the player edition.
bool developerInput(){return false;}
void drawDeveloperUi(){}
void drawDeveloperSummonHud(){}
bool developerSummonHudHit(Vector2){return false;}
bool clickDeveloperSummonHud(){return false;}
#else
constexpr WorldLocation DeveloperTabs[]={WorldLocation::Forest,WorldLocation::Desert,WorldLocation::Volcano,WorldLocation::MageTower};
constexpr const char* DeveloperTabNames[]={"Wielka Knieja","Morze Piasków","Kaldera Popiołu","Wieża Eldrina"};
WorldLocation developerTab=WorldLocation::Forest;
bool developerSummonMenu=false;
int developerSummonPage=0;
constexpr int DeveloperSummonPageSize=24;
constexpr int DeveloperSummonPages=(EnemyTypeCount+DeveloperSummonPageSize-1)/DeveloperSummonPageSize;
const auto DeveloperSummonTypes=[]{
 std::array<int,EnemyTypeCount> types{};
 int index=0;
 for(bool bosses:{true,false})for(int type=0;type<EnemyTypeCount;type++)
  if(enemyBoss(type)==bosses)types[index++]=type;
 return types;
}();
std::string developerSummonNotice;
bool developerSummonSucceeded=false;
Rectangle developerSummonTab(){return {974,142,222,36};}
constexpr Rectangle developerSummonHudRect(){
 const Rectangle map=minimapRect();
 return {map.x,map.y+map.height+9,map.width,34};
}
Rectangle developerSummonCard(int slot){return {180.f+(slot%3)*367,222.f+(slot/3)*51,346,44};}
Rectangle developerSummonPageButton(bool next){return {next?969.f:180.f,659,133,34};}
Rectangle developerSummonReturn(){return {533.5f,677,373,42};}
Rectangle developerTabRect(int index){return {180.f+index*274,194,258,38};}
Rectangle developerCard(int index){
 if(DeveloperDestinations[index].location!=developerTab)return {-1,-1,0,0};
 int visible=0;
 for(int i=0;i<index;i++)if(DeveloperDestinations[i].location==developerTab)visible++;
 return {180.f+(visible%3)*367,248.f+(visible/3)*46,346,40};
}
Rectangle developerNewButton(){return {welcome?533.5f:330.f,667,373,47};}
Rectangle developerMoveButton(){return {737,667,373,47};}
Rectangle developerCloseButton(){return {1230,116,40,35};}
void openDeveloperMenu(){
 quickPickerSlot=-1;game.cancelBow();dragging={};audioDragging=-1;
 developerSummonMenu=false;developerSummonPage=0;developerSummonNotice.clear();
 if(developerSelection>=0&&developerSelection<DeveloperDestinationCount)
  developerTab=DeveloperDestinations[developerSelection].location;
 developerMenuOpen=true;
}
void closeDeveloperMenu(){developerMenuOpen=false;developerError.clear();}
bool developerSummonHudHit(Vector2 point){
 return game.developerSession&&gameplayInputAllowed()&&CheckCollisionPointRec(point,developerSummonHudRect());
}
bool clickDeveloperSummonHud(){
 if(!developerSummonHudHit(mouse))return false;
 inventoryClicks.clear();
 game.cancelMeleeHold();
 openDeveloperMenu();
 developerSummonMenu=true;
 return true;
}
void drawDeveloperSummonHud(){
 if(!game.developerSession||!gameplayInputAllowed())return;
 const Rectangle button=developerSummonHudRect();
 panel(button,inside(button)?rgb(75,61,38):rgb(48,41,29));
 DrawRectangleLinesEx(button,1,Gold);
 center("PRZYZYWANIE BESTII",button.x+button.width/2,button.y+8,15,Gold);
}
bool startDeveloperSession(bool reset){
 if(welcome&&!reset)return false;
 if(!developerStart(game,developerSelection,reset,std::random_device{}())){developerError="Nie ma bezpiecznego miejsca startu. Wybierz inną lokację.";return false;}
 intro={};towerStory={};clearStoryMenus();bagPage=0;developerActive=true;developerMenuOpen=false;developerError.clear();
 statDraft={};skillDraft={};draftForLevel=skillDraftLevel=-1;mouse={0,0};
 startupLog.write((std::string("Developer session: ")+DeveloperDestinations[developerSelection].id+(reset?" / new character":" / current character")).c_str());return true;
}
bool clickDeveloperSummonMenu(){
 if(inside(developerSummonReturn())){closeDeveloperMenu();return true;}
 if(DeveloperSummonPages>1)for(bool next:{false,true})if(inside(developerSummonPageButton(next))){
  developerSummonPage=std::clamp(developerSummonPage+(next?1:-1),0,DeveloperSummonPages-1);
  return true;
 }
 for(int slot=0;slot<DeveloperSummonPageSize;slot++){
  const int index=developerSummonPage*DeveloperSummonPageSize+slot;
  if(index>=EnemyTypeCount||!inside(developerSummonCard(slot)))continue;
  const int type=DeveloperSummonTypes[index];
  developerSummonSucceeded=summonDeveloperEnemy(game,type);
  developerSummonNotice=developerSummonSucceeded?std::string("Przyzwano: ")+enemyInfo(type).name:
   game.inTower()?"Przyzywanie jest dostępne w lokacjach walki.":"Brak wolnego miejsca obok bohatera. Spróbuj w innym miejscu.";
  return true;
 }
 return true;
}
bool clickDeveloperMenu(){
 if(inside(developerCloseButton())){closeDeveloperMenu();return true;}
 if(game.developerSession&&!welcome&&inside(developerSummonTab())){
  developerSummonMenu=!developerSummonMenu;
  developerSummonNotice.clear();
  return true;
 }
 if(developerSummonMenu)return clickDeveloperSummonMenu();
 for(int i=0;i<int(std::size(DeveloperTabs));i++)if(inside(developerTabRect(i))){
  developerTab=DeveloperTabs[i];developerError.clear();
  if(developerSelection<0||developerSelection>=DeveloperDestinationCount||DeveloperDestinations[developerSelection].location!=developerTab)
   for(int dest=0;dest<DeveloperDestinationCount;dest++)if(DeveloperDestinations[dest].location==developerTab){developerSelection=dest;break;}
  return true;
 }
 for(int i=0;i<DeveloperDestinationCount;i++)if(DeveloperDestinations[i].location==developerTab&&inside(developerCard(i))){
  developerSelection=i;developerError.clear();game.sound(Cue::Ui,game.player);return true;
 }
 if(inside(developerNewButton()))return startDeveloperSession(true);
 if(!welcome&&inside(developerMoveButton()))return startDeveloperSession(false);
 return true;
}
bool developerInput(){
 if(!IsWindowFocused())return developerMenuOpen;
 if(IsKeyPressed(KEY_F3)){if(developerMenuOpen)closeDeveloperMenu();else openDeveloperMenu();return true;}
 if(developerMenuOpen){
  if(IsKeyPressed(KEY_ESCAPE)){closeDeveloperMenu();return true;}
  if(IsKeyPressed(KEY_ENTER)&&!developerSummonMenu){startDeveloperSession(true);return true;}
  if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))clickDeveloperMenu();return true;
 }
 return false;
}
void drawDeveloperSummonMenu(){
 const char* hint=game.inTower()?"Przyzywanie jest dostępne w lokacjach walki. Wybierz krainę w zakładce Lokacje.":
  "Kliknij nazwę przeciwnika, a potem wróć do walki.";
 center(hint,720,188,18,Muted);
 for(int slot=0;slot<DeveloperSummonPageSize;slot++){
  const int index=developerSummonPage*DeveloperSummonPageSize+slot;
  if(index>=EnemyTypeCount)break;
  const int type=DeveloperSummonTypes[index];
  const Rectangle r=developerSummonCard(slot);
  const bool boss=enemyBoss(type),active=inside(r)&&!game.inTower();
  panel(r,active?Card:Panel);
  DrawRectangleLinesEx(r,1,boss&&!game.inTower()?Gold:Line);
  txt(enemyInfo(type).name,r.x+13,r.y+12,18,game.inTower()?Muted:boss?Gold:Text);
  if(boss)txt("BOSS",r.x+r.width-52,r.y+15,12,game.inTower()?Muted:Gold);
 }
 if(DeveloperSummonPages>1){
  for(bool next:{false,true}){
   Rectangle r=developerSummonPageButton(next);panel(r,inside(r)?Card:Panel);
   center(next?"DALEJ >":"< WSTECZ",r.x+r.width/2,r.y+8,14,Gold);
  }
  center(TextFormat("%d / %d",developerSummonPage+1,DeveloperSummonPages),720,649,14,Muted);
 }
 const Rectangle back=developerSummonReturn();
 panel(back,inside(back)?Card:Panel);center(game.inTower()?"WRÓĆ DO GRY":"WRÓĆ DO WALKI",720,back.y+11,17,Gold);
 if(!developerSummonNotice.empty())center(developerSummonNotice,720,743,17,developerSummonSucceeded?Green:Gold);
}
void drawDeveloperUi(){
 if(!developerMenuOpen)return;
 rect(0,0,W,H,rgb(6,12,18,224));panel({140,100,1160,681},rgb(21,30,36));line(180,122,1210,122,Gold);
 center(developerSummonMenu?"PRZYZYWANIE POTWORÓW":"TRYB DEWELOPERSKI",720,143,30,Text,true);
 panel(developerCloseButton(),inside(developerCloseButton())?Card:Panel);center("X",1250,123,18,Gold);
 if(game.developerSession&&!welcome){
  const Rectangle tab=developerSummonTab();
  panel(tab,inside(tab)?Card:Panel);DrawRectangleLinesEx(tab,1,Gold);
  center(developerSummonMenu?"LOKACJE":"PRZYZWIJ POTWORA",tab.x+tab.width/2,tab.y+9,16,Gold);
 }
 if(developerSummonMenu){drawDeveloperSummonMenu();return;}
 for(int i=0;i<int(std::size(DeveloperTabs));i++){
  const auto r=developerTabRect(i);const bool active=developerTab==DeveloperTabs[i];
  panel(r,active?rgb(39,53,54):inside(r)?Card:Panel);
  DrawRectangleLinesEx(r,1,active?Gold:Line);
  center(DeveloperTabNames[i],r.x+r.width/2,r.y+10,17,active?Gold:Text);
 }
 for(int i=0;i<DeveloperDestinationCount;i++){
  if(DeveloperDestinations[i].location!=developerTab)continue;
  Rectangle card=developerCard(i);bool selected=i==developerSelection;panel(card,selected?rgb(39,53,54):inside(card)?Card:Panel);DrawRectangleLinesEx(card,1,selected?Gold:Line);
  txt(DeveloperDestinations[i].name,card.x+13,card.y+9,18,selected?Gold:Text);
 }

 for(bool reset:{true,false}){if(welcome&&!reset)continue;Rectangle action=reset?developerNewButton():developerMoveButton();panel(action,inside(action)?Card:Panel);center(reset?"NOWA SESJA TESTOWA":"PRZENIEŚ OBECNĄ POSTAĆ",action.x+action.width/2,action.y+13,17,Gold);}
 if(!developerError.empty())center(developerError,720,753,15,rgb(244,157,131));
}

#endif
