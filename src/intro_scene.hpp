#pragma once
#include "intro_art.hpp"

Rectangle introSkipButton(){return {1190,77,170,34};}
Rectangle introDialogueBox(){return {112,626,1216,222};}
Rectangle introNextButton(){Rectangle box=introDialogueBox();return {box.x+box.width-34-162,794,162,40};}
void startIntro(){
    developerActive=developerMenuOpen=false;developerError.clear();
    towerPanelOpen=false;towerPanelObject=-1;towerPage=0;game.cancelBow();intro.begin();towerStory={};welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=weaponWheelOpen=false;
    dragging={};audioDragging=-1;nearbyPage=0;hovered={};
}
void applyIntroArrival(){
    if(!intro.takeArrival())return;
    game.player={8,19};game.animation=AnimationState{};game.cancelBow();dragging={};mouse={0,0};
    game.say("Serce Świtu: znajdź Kamiennego Strażnika na północy Lasu Mchu.");
    startupLog.write("Prologue complete: hero teleported to the forest");
}
void introAction(bool advance,bool skip,float dt,bool focused=true){
    if(!intro.active()||!focused)return;
    if(skip)intro.skip();else if(advance)intro.advance();
    intro.update(dt);applyIntroArrival();
}
void introInput(float dt){
    bool advance=IsKeyPressed(KEY_ENTER)||IsKeyPressed(KEY_SPACE)||IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool skip=IsKeyPressed(KEY_ESCAPE)||(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&inside(introSkipButton()));
    introAction(advance,skip,dt,IsWindowFocused());
}
void drawStoryPortal(float x,float y,float size,float time,float energy){
    Color violet=rgb(172,126,246);float wave=std::sin(time*3)*.5f+.5f;
    ellipse(x,y,90*size,31*size,Fade(violet,.10f*energy));
    for(int ring=0;ring<3;ring++)ellipseOutline(x,y,(58+ring*12+wave*3)*size,(20+ring*4)*size,Fade(violet,energy*(.75f-ring*.18f)));
    for(int i=0;i<12;i++){
        float a=i*PI/6+time*.35f;float px=x+std::cos(a)*72*size,py=y+std::sin(a)*25*size;
        diamond(px,py,3*size,6*size,Fade(i%3?Gold:Text,energy*.9f));
    }
    for(int i=0;i<25;i++){
        float rise=std::fmod(time*(.18f+(i%4)*.02f)+i*.043f,1.f);
        float px=x+std::sin(i*2.7f+time*.5f)*(28+energy*35)*size;
        float py=y-rise*(110+energy*135)*size;
        DrawCircleV({px,py},(1.5f+i%3)*size,Fade(violet,energy*(1-rise)*.7f));
    }
}
void drawStoryMage(float x,float y,float size,float time,float casting){
    intro_art::mage(x,y,size,time,casting);
}
std::vector<std::string> storyRows(const std::string& text,float width,float size){
    std::istringstream words(text);std::string word,row;std::vector<std::string> rows;
    while(words>>word){
        std::string next=row.empty()?word:row+" "+word;
        if(!row.empty()&&textWidth(next,size)>width){rows.push_back(row);row=word;}else row=next;
    }
    if(!row.empty())rows.push_back(row);return rows;
}
void storyWrappedText(const std::string& text,float x,float y,float width,float size,Color color){
    for(const std::string& row:storyRows(text,width,size)){txt(row,x,y,size,color);y+=size*1.5f;}
}
void drawStoryDialogue(const StoryLine& current,size_t turn,size_t total,const std::string& text,bool complete,bool controls){
    Rectangle box=introDialogueBox();panel(box,rgb(15,20,34,248));DrawRectangleLinesEx(box,1.5f,rgb(116,91,135));
    Color speaker=current.mage?rgb(191,148,243):Gold;
    rect(box.x,box.y,4,box.height,speaker);
    txt(current.mage?"ELDRIN":"WĘDROWIEC",146,647,23,speaker,true);
    txt(current.mage?"Strażnik ostatniej pieczęci":"Strażnik żaru",146,681,14,Muted);
    txt(TextFormat("%02d / %02d",int(turn)+1,int(total)),1222,653,14,Muted);
    storyWrappedText(text,146,716,1140,21,Text);
    if(controls){
        txt("ENTER / SPACJA / LPM — dalej     ·     Dialog przewija się również automatycznie",146,811,14,Muted);
        Rectangle next=introNextButton();DrawRectangleRounded(next,.22f,12,inside(next)?rgb(72,53,91):rgb(37,32,51));
        DrawRectangleRoundedLinesEx(next,.22f,12,1,Fade(speaker,.3f));
        center(complete?"DALEJ":"POKAŻ TEKST",next.x+next.width/2,next.y+10,14,speaker);
    }
}
void drawIntroDialogue(){
    if(intro.phase!=IntroPhase::Dialogue&&intro.phase!=IntroPhase::Charging)return;
    drawStoryDialogue(intro.current(),intro.line,IntroDialogue.size(),intro.phase==IntroPhase::Charging?intro.current().text:intro.visibleText(),intro.textComplete(),intro.phase==IntroPhase::Dialogue);
}
void drawIntroChamber(){
    float t=intro.time,charge=intro.phase==IntroPhase::Charging?storyEase(intro.age/IntroSequence::ChargeTime):intro.phase==IntroPhase::Departing?1:0;
    intro_art::observatory(t);
    for(int i=0;i<5;i++){
        float a=-PI*.5f+i*2*PI/5+t*.12f,x=720+std::cos(a)*83,y=337+std::sin(a)*28+std::sin(t*2+i)*5;
        line(720,351,x,y,Fade(Gold,.18f));ellipse(x,y+12,15,5,Fade(rgb(169,126,216),.12f));
        diamond(x,y,8,18,rgb(216,186,130));diamond(x-2,y-3,3,9,rgb(255,235,188));
    }
    drawStoryPortal(510,539,1.2f,t,.25f+charge*.75f);
    AnimationState pose;pose.direction=Direction::E;pose.idleTime=t;
    actor_art::draw(0,510,532-charge*24,2.5f,pose,0,!game.equipment[1].empty(),!game.equipment[2].empty(),archetype(game.equipment[0].kind));
    drawStoryMage(930,505,2.7f,t,charge);
    if(charge>0){
        for(int i=0;i<5;i++)ellipse(510,460-charge*50,58+i*13,90+i*16,Fade(rgb(168,116,242),charge*.025f));
        intro_art::stroke({847-charge*24,373-charge*35},{741,349},{609,379},{510,482-charge*24},
                          2+charge*3,Fade(rgb(191,153,250),charge*.45f));
    }
    rect(0,0,W,55,rgb(5,8,16));rect(0,860,W,40,rgb(5,8,16));
    txt("EMBERFALL",79,79,29,Gold,true);txt("PROLOG  /  OSTATNIA PIECZĘĆ",80,121,14,Muted);
    if(intro.phase==IntroPhase::Opening){center("Obserwatorium na skraju Otchłani",720,581,22,Text,true);center("Gdy gasną pieczęcie, ktoś musi podtrzymać światło.",720,616,16,Muted);}
    drawIntroDialogue();
    if(intro.phase==IntroPhase::Opening||intro.phase==IntroPhase::Dialogue){
        Rectangle skip=introSkipButton();panel(skip,inside(skip)?Card:rgb(25,24,38));center("POMIŃ · ESC",skip.x+skip.width/2,skip.y+10,14,Muted);
    }
    if(intro.fade()>0)rect(0,0,W,H,Fade(intro.phase==IntroPhase::Opening?rgb(5,8,16):rgb(226,214,249),intro.fade()));
}
void drawIntroArrival(){
    Vector2 p=screen(game.player);float energy=1-storyEase(intro.age/IntroSequence::ArrivalTime);
    rect(0,0,W,H,Fade(rgb(219,233,255),intro.fade()));
    intro_art::column({p.x,p.y+4},intro.time,energy);
    rect(0,0,W,55,rgb(5,8,16));rect(0,860,W,40,rgb(5,8,16));
    center("LAS MCHU",W*.5f,88,29,Gold,true);center("Pierwszy krok ku Sercu Świtu",W*.5f,127,17,Muted);
}
