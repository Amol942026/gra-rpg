#pragma once

bool audioOpen=false;int audioDragging=-1;
Rectangle audioButton(){return {804,855,211,30};}
Rectangle audioToggle(int channel){return {804.f+channel*109,799,102,49};}
Rectangle audioSlider(int index){return {504,414.f+index*72,400,8};}

bool clickAudioHud() {
    for(int channel=0;channel<2;channel++)if(inside(audioToggle(channel))){
        if(channel==0)audioSystem.toggleMusic();else audioSystem.toggleEffects();
        audioSystem.play(Cue::Ui);return true;
    }
    if(inside(audioButton())){
        audioOpen=!audioOpen;audioDragging=-1;dragging={};audioSystem.play(Cue::Ui);return true;
    }
    return false;
}

bool audioInput() {
    if(IsKeyPressed(KEY_M)){audioSystem.toggleMute();audioSystem.play(Cue::Ui);}
    if(IsKeyPressed(KEY_F2)){
        audioOpen=!audioOpen;audioDragging=-1;dragging={};audioSystem.play(Cue::Ui);return true;
    }
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&clickAudioHud())return true;
    if(!audioOpen)return false;
    if(IsKeyPressed(KEY_ESCAPE)||IsKeyPressed(KEY_ENTER)||
       (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&inside({610,584,220,42}))){
        audioOpen=false;audioDragging=-1;audioSystem.play(Cue::Ui);return true;
    }
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        for(int i=0;i<2;i++){Rectangle r=audioSlider(i);r.y-=14;r.height=36;if(inside(r))audioDragging=i;}
        if(inside({504,534,400,32})){audioSystem.toggleMute();audioSystem.play(Cue::Ui);}
    }
    if(audioDragging>=0&&IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
        Rectangle r=audioSlider(audioDragging);float value=std::clamp((mouse.x-r.x)/r.width,0.f,1.f);
        if(audioDragging==0)audioSystem.musicVolume=value;else audioSystem.effectsVolume=value;
    }
    if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
        if(audioDragging==1)audioSystem.play(Cue::Pickup);
        audioDragging=-1;
    }
    return true;
}

void drawAudioButton() {
    for(int channel=0;channel<2;channel++){
        Rectangle r=audioToggle(channel);
        bool enabled=channel==0?audioSystem.musicEnabled():audioSystem.effectsEnabled();
        Color state=enabled?Green:Muted;
        panel(r,inside(r)?Card:Panel);
        DrawRectangleLinesEx(r,1,inside(r)?Gold:enabled?shade(Green,.65f):Line);
        center(channel==0?"MUZYKA":"EFEKTY",r.x+r.width/2,r.y+7,14,Text);
        DrawCircle(int(r.x+24),int(r.y+35),3,state);
        center(enabled?"WŁ.":"WYŁ.",r.x+r.width/2+6,r.y+26,14,state);
    }
    Rectangle r=audioButton();panel(r,inside(r)?Card:Panel);
    center("GŁOŚNOŚĆ  ·  F2",r.x+r.width/2,r.y+7,14,Gold);
}
void drawAudioPanel() {
    if(!audioOpen)return;
    rect(0,0,W,H,rgb(6,15,18,218));panel({440,276,560,376},Panel);
    line(468,295,972,295,Gold);center("DŹWIĘKI RUIN",720,317,28,Text,true);
    center(audioSystem.available?"Dopasuj muzykę i odgłosy wyprawy.":"Brak dostępnego urządzenia dźwiękowego.",720,361,16,Muted);
    for(int i=0;i<2;i++){
        Rectangle r=audioSlider(i);float value=i==0?audioSystem.musicVolume:audioSystem.effectsVolume;
        bool enabled=i==0?audioSystem.musicEnabled():audioSystem.effectsEnabled();
        txt(i==0?"MUZYKA":"EFEKTY",r.x,r.y-30,14,Gold);
        if(!enabled)txt("WYŁĄCZONE",r.x+100,r.y-30,14,Muted);
        txt(TextFormat("%d%%",int(std::round(value*100))),926,r.y-6,16,Text);
        rect(r.x,r.y,r.width,r.height,Line);rect(r.x,r.y,r.width*value,r.height,Gold);
        DrawCircleV({r.x+value*r.width,r.y+4},8,Text);
    }
    center(audioSystem.muted?"[ M ]  Włącz dźwięk":"[ M ]  Wycisz wszystko",720,539,17,audioSystem.muted?Gold:Muted);
    panel({610,584,220,42},inside({610,584,220,42})?Card:Panel);center("GOTOWE",720,595,17,Gold);
}
