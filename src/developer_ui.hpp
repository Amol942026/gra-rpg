#pragma once
#ifdef EMBERFALL_PLAYER_BUILD
// No menu, shortcut handler or hit target is linked into the player edition.
bool developerInput(){return false;}
void drawDeveloperUi(){}
#else
Rectangle developerCard(int index){int column=index<8?0:index<16?1:2;int row=index<8?index:index<16?index-8:index-16;return {180.f+column*372,231.f+row*49,336,43};}
Rectangle developerNewButton(){return {welcome?533.5f:330.f,667,373,47};}
Rectangle developerMoveButton(){return {737,667,373,47};}
Rectangle developerCloseButton(){return {1230,116,40,35};}
void openDeveloperMenu(){quickPickerSlot=-1;game.cancelBow();dragging={};audioDragging=-1;developerMenuOpen=true;}
void closeDeveloperMenu(){developerMenuOpen=false;developerError.clear();}
bool startDeveloperSession(bool reset){
 if(welcome&&!reset)return false;
 if(!developerStart(game,developerSelection,reset,std::random_device{}())){developerError="Nie ma bezpiecznego miejsca startu. Wybierz inną lokację.";return false;}
 intro={};towerStory={};clearStoryMenus();developerActive=true;developerMenuOpen=false;developerError.clear();
 statDraft={};skillDraft={};draftForLevel=skillDraftLevel=-1;mouse={0,0};
 startupLog.write((std::string("Developer session: ")+DeveloperDestinations[developerSelection].id+(reset?" / new character":" / current character")).c_str());return true;
}
bool clickDeveloperMenu(){
 for(int i=0;i<DeveloperDestinationCount;i++)if(inside(developerCard(i))){developerSelection=i;developerError.clear();game.sound(Cue::Ui,game.player);return true;}
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
 center("TRYB DEWELOPERSKI",720,143,30,Text,true);center("Wybierz lokację i rozpocznij test bez przechodzenia fabuły.",720,186,17,Muted);
 txt("LAS MCHU",185,212,14,Gold);txt("WIEŻA ELDRINA / MORZE PIASKÓW",557,212,14,Gold);txt("KALDERA POPIOŁU",929,212,14,Gold);
 for(int i=0;i<DeveloperDestinationCount;i++){
  Rectangle card=developerCard(i);bool selected=i==developerSelection;panel(card,selected?rgb(39,53,54):inside(card)?Card:Panel);DrawRectangleLinesEx(card,1,selected?Gold:Line);
  txt(DeveloperDestinations[i].name,card.x+13,card.y+3,18,selected?Text:Muted);txt(DeveloperDestinations[i].hint,card.x+13,card.y+25,14,selected?Gold:Muted);
 }

 for(bool reset:{true,false}){if(welcome&&!reset)continue;Rectangle action=reset?developerNewButton():developerMoveButton();panel(action,inside(action)?Card:Panel);center(reset?"NOWA SESJA TESTOWA":"PRZENIEŚ OBECNĄ POSTAĆ",action.x+action.width/2,action.y+13,17,Gold);}
 panel(developerCloseButton(),inside(developerCloseButton())?Card:Panel);center("X",1250,123,18,Gold);
 if(!developerError.empty())center(developerError,720,753,15,rgb(244,157,131));
}

#endif
