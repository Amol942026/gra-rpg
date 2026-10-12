#pragma once

void clearStoryMenus(){
    game.cancelBow();game.closeCorpse();welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=weaponWheelOpen=false;
    towerPanelOpen=false;towerPanelObject=-1;towerPage=0;dragging={};audioDragging=-1;nearbyPage=0;hovered={};
}
void startTowerReturn(){if(game.takeTowerReturn()){clearStoryMenus();towerStory.beginReturn();}}
bool talkToMage(){
    if(!gameplayInputAllowed()||dragging.place!=Place::None||!game.inTower())return false;
    if(!game.mageReachable()){game.say("Podejdź bliżej Eldrina, aby porozmawiać (E / PPM).");return false;}
    clearStoryMenus();towerStory.beginTalk();game.sound(Cue::Ui,game.player);return true;
}
void towerAction(bool advance,bool skip,float dt,bool focused=true){
    if(!towerStory.active()||!focused)return;
    if(skip)towerStory.skip();else if(advance)towerStory.advance();
    towerStory.update(dt);
    if(towerStory.takeArrival()){game.enterMageTower();mouse={0,0};startupLog.write("First Dawn shard collected: hero returned to Eldrin's tower");}
    if(towerStory.takeCompletion())game.finishTowerReturn();
}
void towerInput(float dt){
    bool pressed=IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    towerAction(IsKeyPressed(KEY_ENTER)||IsKeyPressed(KEY_SPACE)||pressed,
                IsKeyPressed(KEY_ESCAPE)||(pressed&&inside(introSkipButton())),dt,IsWindowFocused());
}
void drawTowerDialogue(){
    if(towerStory.phase!=TowerPhase::Dialogue)return;
    drawStoryDialogue(towerStory.current(),towerStory.line,towerStory.lineCount(),towerStory.visibleText(),towerStory.textComplete(),true);
}
void drawTowerChamber(){
    float t=towerStory.time;intro_art::observatory(t);
    // Four pale fragments are only a projection; the recovered piece glows gold.
    for(int i=0;i<5;i++){
        float a=-PI*.5f+i*2*PI/5+t*.12f,x=720+std::cos(a)*83,y=337+std::sin(a)*28+std::sin(t*2+i)*5;
        Color c=i==0?Gold:rgb(115,114,147,110);line(720,351,x,y,Fade(c,.24f));diamond(x,y,8,18,c);
        if(i==0){intro_art::glow(x,y,29,29,Fade(Gold,.18f));diamond(x-2,y-3,3,9,rgb(255,240,188));}
    }
    AnimationState pose;pose.direction=Direction::E;pose.idleTime=t;
    actor_art::draw(0,510,532,2.5f,pose,0,!game.equipment[1].empty(),!game.equipment[2].empty(),game.equipment[0].kind);
    drawStoryMage(930,505,2.7f,t,0);
    if(towerStory.phase==TowerPhase::Opening)intro_art::column({510,539},t,1-storyEase(towerStory.age/TowerSequence::OpeningTime));
    rect(0,0,W,55,rgb(5,8,16));rect(0,860,W,40,rgb(5,8,16));
    txt("EMBERFALL",79,79,29,Gold,true);txt("SERCE ŚWITU  /  PIERWSZY BLASK",80,121,14,Muted);
    if(towerStory.phase==TowerPhase::Opening){center("Wieża Eldrina",720,581,27,Text,true);center("Jedna część odzyskana. Nadzieja powraca.",720,616,16,Muted);}
    drawTowerDialogue();
    if(towerStory.phase==TowerPhase::Opening||towerStory.phase==TowerPhase::Dialogue){
        Rectangle skip=introSkipButton();panel(skip,inside(skip)?Card:rgb(25,24,38));center("POMIŃ · ESC",skip.x+skip.width/2,skip.y+10,14,Muted);
    }
    if(towerStory.fade()>0)rect(0,0,W,H,Fade(BLACK,towerStory.fade()));
}
void drawTowerStoryOverlay(){
    if(!towerStory.active())return;
    if(!towerStory.returning){
        rect(0,0,W,H,rgb(5,8,16,95));drawTowerDialogue();
        Rectangle skip=introSkipButton();panel(skip,inside(skip)?Card:rgb(25,24,38));center("ZAKOŃCZ · ESC",skip.x+skip.width/2,skip.y+10,14,Muted);
    }else if(towerStory.fade()>0)rect(0,0,W,H,Fade(BLACK,towerStory.fade()));
}
