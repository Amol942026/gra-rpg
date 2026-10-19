#pragma once

void clearStoryMenus(){
    game.cancelBow();game.closeCorpse();welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=weaponWheelOpen=false;
    towerPanelOpen=false;towerPanelObject=-1;towerPage=0;dragging={};audioDragging=-1;nearbyPage=0;hovered={};
}
void startTowerReturn(){if(game.takeTowerReturn()){clearStoryMenus();towerStory.beginReturn(game.dawnFragments());}}
bool talkToMage(){
    if(!gameplayInputAllowed()||dragging.place!=Place::None||!game.inTower())return false;
    if(!game.mageReachable()){game.say("Podejdź bliżej Eldrina, aby porozmawiać (E / PPM).");return false;}
    clearStoryMenus();towerStory.beginTalk(game.canTravelDesert(),game.dawnFragments());game.sound(Cue::Ui,game.player);return true;
}
void towerAction(bool advance,bool skip,float dt,bool focused=true){
    if(!towerStory.active()||!focused)return;
    if(skip)towerStory.skip();else if(advance)towerStory.advance();
    towerStory.update(dt);
    if(towerStory.takeArrival()){game.enterMageTower();mouse={0,0};startupLog.write("First Dawn shard collected: hero returned to Eldrin's tower");}
    if(towerStory.takeDesertArrival()){
        if(game.travelToDesert()){dragging={};nearbyPage=0;mouse={0,0};startupLog.write("Eldrin dialogue choice: hero teleported to the desert");}
        else towerStory.enter(TowerPhase::Finished);
    }
    if(towerStory.takeCompletion())game.finishTowerReturn();
}
Rectangle desertTravelButton(){return {395,496,650,64};}
Rectangle desertStayButton(){return {395,579,650,46};}
bool clickMageTravelChoice(){
 if(towerStory.phase!=TowerPhase::Choice)return false;
 if(inside(desertTravelButton())){if(!game.canTravelDesert())return false;game.sound(Cue::Spell,game.player);return towerStory.chooseDesert();}
 if(inside(desertStayButton())){towerStory.skip();return true;}return false;
}
void towerInput(float dt){
 bool pressed=IsMouseButtonPressed(MOUSE_BUTTON_LEFT),focused=IsWindowFocused();
 if(towerStory.phase==TowerPhase::Choice){if(focused){if(IsKeyPressed(KEY_ESCAPE))towerStory.skip();else if(pressed)clickMageTravelChoice();}towerAction(false,false,dt,focused);return;}
 towerAction(IsKeyPressed(KEY_ENTER)||IsKeyPressed(KEY_SPACE)||pressed,IsKeyPressed(KEY_ESCAPE)||(pressed&&inside(introSkipButton())),dt,focused);
}
void drawMageTravelChoice(){
 panel({320,172,800,515},rgb(22,27,39));line(350,199,1090,199,Gold);
 center("ELDRIN — NASTĘPNA WYPRAWA",720,223,27,Text,true);
 center("MORZE PIASKÓW",720,272,24,Gold,true);
 storyWrappedText(game.dawnFragments()>=2?"Żywa Wydma została pokonana. Mogę przenieść cię z powrotem na pustynię po pozostawiony łup.":"Za piaskami czekają skorpiony, skarabeusze i kobry. Żywa Wydma strzeże drugiej części Serca Świtu. W miejscu przybycia zostawię bramę powrotną.",380,321,680,20,Text);
 center(TextFormat("Odzyskane części Serca Świtu: %d / 5",game.dawnFragments()),720,443,17,Muted);
 Rectangle travel=desertTravelButton();panel(travel,inside(travel)?rgb(74,61,41):rgb(49,42,34));DrawRectangleLinesEx(travel,1.5f,Gold);center("Przenieś mnie na pustynię.",720,travel.y+19,23,Gold);
 Rectangle stay=desertStayButton();panel(stay,inside(stay)?Card:Panel);center("Jeszcze zostanę w wieży.",720,stay.y+12,18,Muted);
 center("Wybierz odpowiedź myszą · ESC — zostań w wieży",720,652,15,Muted);
}
void drawTowerDialogue(){
    if(towerStory.phase!=TowerPhase::Dialogue)return;
    drawStoryDialogue(towerStory.current(),towerStory.line,towerStory.lineCount(),towerStory.visibleText(),towerStory.textComplete(),true);
}
void drawTowerChamber(){
    float t=towerStory.time;intro_art::observatory(t,false);world_shadows::draw();intro_art::observatoryObjects(t);
    // Four pale fragments are only a projection; the recovered piece glows gold.
    for(int i=0;i<5;i++){
        float a=-PI*.5f+i*2*PI/5+t*.12f,x=720+std::cos(a)*83,y=337+std::sin(a)*28+std::sin(t*2+i)*5;
        Color c=i<game.dawnFragments()?Gold:rgb(115,114,147,110);line(720,351,x,y,Fade(c,.24f));diamond(x,y,8,18,c);
        if(i<game.dawnFragments()){intro_art::glow(x,y,29,29,Fade(Gold,.18f));diamond(x-2,y-3,3,9,rgb(255,240,188));}
    }
    AnimationState pose;pose.direction=Direction::E;pose.idleTime=t;
    actor_art::shadowCacheKey=0;
    actor_art::draw(0,510,532,2.5f,pose,0,!game.equipment[1].empty(),!game.equipment[2].empty(),archetype(game.equipment[0].kind));
    actor_art::shadowCacheKey=-1;
    drawStoryMage(930,505,2.7f,t,0);
    if(towerStory.phase==TowerPhase::Opening)intro_art::column({510,539},t,1-storyEase(towerStory.age/TowerSequence::OpeningTime));
    rect(0,0,W,55,rgb(5,8,16));rect(0,860,W,40,rgb(5,8,16));
    txt("EMBERFALL",79,79,29,Gold,true);txt(game.dawnFragments()>=2?"SERCE ŚWITU / DRUGI BLASK":"SERCE ŚWITU / PIERWSZY BLASK",80,121,14,Muted);
    if(towerStory.phase==TowerPhase::Opening){center("Wieża Eldrina",720,581,27,Text,true);center(game.dawnFragments()>=2?"Dwie części odzyskane. Pieczęć nabiera sił.":"Jedna część odzyskana. Nadzieja powraca.",720,616,16,Muted);}
    drawTowerDialogue();
    if(towerStory.phase==TowerPhase::Opening||towerStory.phase==TowerPhase::Dialogue){
        Rectangle skip=introSkipButton();panel(skip,inside(skip)?Card:rgb(25,24,38));center("POMIŃ · ESC",skip.x+skip.width/2,skip.y+10,14,Muted);
    }
    if(towerStory.fade()>0)rect(0,0,W,H,Fade(BLACK,towerStory.fade()));
}
void drawTowerStoryOverlay(){
    if(!towerStory.active())return;
    if(!towerStory.returning){
        rect(0,0,W,H,rgb(5,8,16,95));
        if(towerStory.phase==TowerPhase::Choice){drawMageTravelChoice();return;}
        if(towerStory.phase==TowerPhase::TravelOut||towerStory.phase==TowerPhase::TravelIn){
            Vector2 p=screen(game.player);intro_art::column(p,towerStory.time,towerStory.phase==TowerPhase::TravelOut?storyEase(towerStory.age/.9f):1-storyEase(towerStory.age/1.2f));
            center(towerStory.phase==TowerPhase::TravelOut?"Eldrin otwiera przejście…":"Morze Piasków",WorldW*.5f,557,26,Gold,true);
            if(towerStory.fade()>0)rect(0,0,W,H,Fade(BLACK,towerStory.fade()));return;
        }
        drawTowerDialogue();
        Rectangle skip=introSkipButton();panel(skip,inside(skip)?Card:rgb(25,24,38));center("ZAKOŃCZ · ESC",skip.x+skip.width/2,skip.y+10,14,Muted);
    }else if(towerStory.fade()>0)rect(0,0,W,H,Fade(BLACK,towerStory.fade()));
}
