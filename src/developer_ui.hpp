#pragma once
#ifdef EMBERFALL_PLAYER_BUILD
// No menu, shortcut handler or hit target is linked into the player edition.
Rectangle developerButton(){return {-100,-100,0,0};}
bool developerInput(){return false;}
void drawDeveloperUi(){}
#else
Rectangle developerButton(){return intro.active()||towerStory.active()?Rectangle{1000,77,168,34}:Rectangle{28,155,158,31};}
Rectangle developerCard(int index){int column=index<8?0:index<16?1:2;int row=index<8?index:index<16?index-8:index-16;return {180.f+column*372,231.f+row*49,336,43};}
Rectangle developerKitButton(){return {180,626,1080,40};}
Rectangle developerNewButton(){return {330,674,373,47};}
Rectangle developerMoveButton(){return {737,674,373,47};}
Rectangle developerNormalButton(){return {471,743,498,35};}
Rectangle developerCloseButton(){return {1230,116,40,35};}
void openDeveloperMenu(){game.cancelBow();dragging={};audioDragging=-1;developerMenuOpen=true;}
void closeDeveloperMenu(){developerMenuOpen=false;developerError.clear();}
bool startDeveloperSession(bool reset){
 if(!developerStart(game,developerSelection,reset,developerKit,std::random_device{}())){developerError="Nie ma bezpiecznego miejsca startu. Wybierz inną lokację.";return false;}
 intro={};towerStory={};clearStoryMenus();developerActive=true;developerMenuOpen=false;developerError.clear();
 statDraft={};skillDraft={};draftForLevel=skillDraftLevel=-1;mouse={0,0};
 startupLog.write((std::string("Developer session: ")+DeveloperDestinations[developerSelection].id+(reset?" / new character":" / current character")).c_str());return true;
}
void developerNormalGame(){closeDeveloperMenu();developerActive=false;game=Game(std::random_device{}());startIntro();}
bool clickDeveloperMenu(){
 for(int i=0;i<DeveloperDestinationCount;i++)if(inside(developerCard(i))){developerSelection=i;developerError.clear();game.sound(Cue::Ui,game.player);return true;}
 if(inside(developerKitButton())){developerKit=!developerKit;return true;}
 if(inside(developerCloseButton())){closeDeveloperMenu();return true;}
 if(inside(developerNewButton()))return startDeveloperSession(true);
 if(inside(developerMoveButton()))return startDeveloperSession(false);
 if(inside(developerNormalButton())){developerNormalGame();return true;}return true;
}
bool developerInput(){
 if(!IsWindowFocused())return developerMenuOpen;
 if(IsKeyPressed(KEY_F3)){if(developerMenuOpen)closeDeveloperMenu();else openDeveloperMenu();return true;}
 if(developerMenuOpen){
  if(IsKeyPressed(KEY_ESCAPE)){closeDeveloperMenu();return true;}
  if(IsKeyPressed(KEY_ENTER)){startDeveloperSession(true);return true;}
  if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))clickDeveloperMenu();return true;
 }
 if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&!towerPanelOpen&&inside(developerButton())){openDeveloperMenu();return true;}
 return false;
}
void drawDeveloperUi(){
 if(!towerPanelOpen){Rectangle button=developerButton();panel(button,inside(button)?Card:Panel);center("DEV · F3",button.x+button.width/2,button.y+8,14,Gold);}
 if(developerActive&&!towerPanelOpen&&!developerMenuOpen&&!intro.active()&&!towerStory.active())txt("SESJA TESTOWA",29,195,14,Gold);
 if(!developerMenuOpen)return;
 rect(0,0,W,H,rgb(6,12,18,224));panel({140,100,1160,714},rgb(21,30,36));line(180,122,1210,122,Gold);
 center("TRYB DEWELOPERSKI",720,143,30,Text,true);center("Wybierz lokację i rozpocznij test bez przechodzenia fabuły.",720,186,17,Muted);
 txt("LAS MCHU",185,212,14,Gold);txt("WIEŻA ELDRINA / MORZE PIASKÓW",557,212,14,Gold);txt("KALDERA POPIOŁU",929,212,14,Gold);
 for(int i=0;i<DeveloperDestinationCount;i++){
  Rectangle card=developerCard(i);bool selected=i==developerSelection;panel(card,selected?rgb(39,53,54):inside(card)?Card:Panel);DrawRectangleLinesEx(card,1,selected?Gold:Line);
  txt(DeveloperDestinations[i].name,card.x+13,card.y+3,18,selected?Text:Muted);txt(DeveloperDestinations[i].hint,card.x+13,card.y+25,14,selected?Gold:Muted);
 }
 Rectangle kit=developerKitButton();panel(kit,inside(kit)?Card:Panel);DrawRectangleLinesEx({kit.x+13,kit.y+12,19,19},1,developerKit?Green:Line);if(developerKit)txt("+",kit.x+16,kit.y+9,18,Green);
 txt("ZESTAW TESTOWY",kit.x+45,kit.y+12,16,developerKit?Green:Muted);
 txt("100 strzał · 15 składników · 10 mikstur",kit.x+251,kit.y+12,14,Muted);

 for(bool reset:{true,false}){Rectangle action=reset?developerNewButton():developerMoveButton();panel(action,inside(action)?Card:Panel);center(reset?"NOWA SESJA TESTOWA":"PRZENIEŚ OBECNĄ POSTAĆ",action.x+action.width/2,action.y+13,17,Gold);}
 center("Nowa sesja resetuje wyprawę; przeniesienie zachowuje plecak, poziom i rangi.",720,726,14,Muted);
 Rectangle normal=developerNormalButton();panel(normal,inside(normal)?Card:Panel);center("NOWA ZWYKŁA GRA — PROLOG",720,normal.y+9,15,Muted);
 panel(developerCloseButton(),inside(developerCloseButton())?Card:Panel);center("X",1250,123,18,Gold);
 if(!developerError.empty())center(developerError,720,785,15,rgb(244,157,131));
}

#endif
