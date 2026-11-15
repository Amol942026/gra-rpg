#pragma once
#ifdef EMBERFALL_PLAYER_BUILD
// No menu, shortcut handler or hit target is linked into the player edition.
bool developerInput(){return false;}
void drawDeveloperUi(){}
#else
constexpr WorldLocation DeveloperTabs[]={WorldLocation::Forest,WorldLocation::Desert,WorldLocation::Volcano,WorldLocation::MageTower};
constexpr const char* DeveloperTabNames[]={"Wielka Knieja","Morze Piasków","Kaldera Popiołu","Wieża Eldrina"};
WorldLocation developerTab=WorldLocation::Forest;
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
 if(developerSelection>=0&&developerSelection<DeveloperDestinationCount)
  developerTab=DeveloperDestinations[developerSelection].location;
 developerMenuOpen=true;
}
void closeDeveloperMenu(){developerMenuOpen=false;developerError.clear();}
bool startDeveloperSession(bool reset){
 if(welcome&&!reset)return false;
 if(!developerStart(game,developerSelection,reset,std::random_device{}())){developerError="Nie ma bezpiecznego miejsca startu. Wybierz inną lokację.";return false;}
 intro={};towerStory={};clearStoryMenus();bagPage=0;developerActive=true;developerMenuOpen=false;developerError.clear();
 statDraft={};skillDraft={};draftForLevel=skillDraftLevel=-1;mouse={0,0};
 startupLog.write((std::string("Developer session: ")+DeveloperDestinations[developerSelection].id+(reset?" / new character":" / current character")).c_str());return true;
}
bool clickDeveloperMenu(){
 for(int i=0;i<int(std::size(DeveloperTabs));i++)if(inside(developerTabRect(i))){
  developerTab=DeveloperTabs[i];developerError.clear();
  if(developerSelection<0||developerSelection>=DeveloperDestinationCount||DeveloperDestinations[developerSelection].location!=developerTab)
   for(int dest=0;dest<DeveloperDestinationCount;dest++)if(DeveloperDestinations[dest].location==developerTab){developerSelection=dest;break;}
  return true;
 }
 for(int i=0;i<DeveloperDestinationCount;i++)if(DeveloperDestinations[i].location==developerTab&&inside(developerCard(i))){
  developerSelection=i;developerError.clear();game.sound(Cue::Ui,game.player);return true;
 }
 if(inside(developerCloseButton())){closeDeveloperMenu();return true;}
 if(inside(developerNewButton()))return startDeveloperSession(true);
 if(!welcome&&inside(developerMoveButton()))return startDeveloperSession(false);
 return true;
}
bool developerInput(){
 if(!IsWindowFocused())return developerMenuOpen;
 if(IsKeyPressed(KEY_F3)){if(developerMenuOpen)closeDeveloperMenu();else openDeveloperMenu();return true;}
 if(developerMenuOpen){
  if(IsKeyPressed(KEY_ESCAPE)){closeDeveloperMenu();return true;}
  if(IsKeyPressed(KEY_ENTER)){startDeveloperSession(true);return true;}
  if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))clickDeveloperMenu();return true;
 }
 return false;
}
void drawDeveloperUi(){
 if(!developerMenuOpen)return;
 rect(0,0,W,H,rgb(6,12,18,224));panel({140,100,1160,681},rgb(21,30,36));line(180,122,1210,122,Gold);
 center("TRYB DEWELOPERSKI",720,143,30,Text,true);
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
 panel(developerCloseButton(),inside(developerCloseButton())?Card:Panel);center("X",1250,123,18,Gold);
 if(!developerError.empty())center(developerError,720,753,15,rgb(244,157,131));
}

#endif
