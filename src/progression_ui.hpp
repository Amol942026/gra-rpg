#pragma once

Rectangle statsButton(){return {1238,16,169,29};}
Rectangle statAdjust(int row,bool add){return {add?1010.f:956.f,273.f+row*56,42,36};}
Rectangle statsConfirm(){return {520,685,400,47};}
Rectangle spellChoice(){return {389,331,662,142};}
Rectangle spellConfirm(){return {540,496,360,46};}

int draftRemaining(){return game.attributePoints-std::accumulate(statDraft.begin(),statDraft.end(),0);}
void openStats(){
    if(game.skillAllocationPending()){openSkills();return;}
    game.cancelBow();weaponWheelOpen=false;skillsOpen=false;statsOpen=true;spellBookOpen=false;statDraft={};draftForLevel=game.level;dragging={};
}
void syncAdvancement(){
 if(!game.allocationRequired)return;
 if(game.attributePoints>0){if(!statsOpen||draftForLevel!=game.level)openStats();}
 else if(game.skillAllocationPending()){if(!skillsOpen||skillDraftLevel!=game.level)openSkills();}
}
void openSpellBook(){
    if(game.allocationRequired||game.dead())return;
    game.cancelBow();weaponWheelOpen=false;skillsOpen=false;spellBookOpen=true;statsOpen=false;dragging={};audioSystem.play(Cue::Ui);
}
bool castSelectedSpell(){
    V aim{};WorldHover hit=pickWorld(mouse);
    if(hit.kind==HoverKind::Enemy){if(Enemy* e=game.findEnemy(hit.index))aim=e->pos-game.player;}
    else if(worldPointerAllowed(mouse))aim=toWorld(mouse)-game.player;
    return game.spell(aim);
}
bool adjustStat(int row,bool add){
    if(!game.allocationRequired||row<0||row>=AttributeCount)return false;
    if(add){if(draftRemaining()<=0)return false;statDraft[row]++;}
    else{if(statDraft[row]<=0)return false;statDraft[row]--;}
    audioSystem.play(Cue::Ui);return true;
}
bool confirmStats(){
    bool advancing=game.allocationRequired;
    if(advancing&&!game.distribute(statDraft))return false;
    statsOpen=false;statDraft={};draftForLevel=-1;if(advancing&&game.skillAllocationPending())openSkills();audioSystem.play(Cue::Ui);return true;
}
bool clickProgression(){
    if(skillsOpen)return clickSkills();
    if(statsOpen){
        for(int i=0;i<AttributeCount;i++)for(bool add:{false,true})if(inside(statAdjust(i,add))){adjustStat(i,add);return true;}
        if(inside(statsConfirm()))confirmStats();
        return true;
    }
    if(spellBookOpen){
        if(inside(spellChoice())||inside(spellConfirm())){game.chooseSpell(Spell::Fireball);spellBookOpen=false;audioSystem.play(Cue::Ui);}
        return true;
    }
    if(!welcome&&!help&&!paused&&!game.dead()&&inside(skillsButton())){openSkills();return true;}
    if(!welcome&&!help&&!paused&&!game.dead()&&inside(statsButton())){openStats();return true;}
    return false;
}
bool progressionInput(){
    syncAdvancement();
    if(statsOpen){
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))clickProgression();
        if(IsKeyPressed(KEY_ENTER))confirmStats();
        if(!game.allocationRequired&&(IsKeyPressed(KEY_ESCAPE)||IsKeyPressed(KEY_P)))statsOpen=false;
        return true;
    }
    if(skillsOpen){
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))clickSkills();
        if(IsKeyPressed(KEY_ENTER))confirmSkills();
        if(!game.allocationRequired&&(IsKeyPressed(KEY_ESCAPE)||IsKeyPressed(KEY_U)))skillsOpen=false;
        return true;
    }
    if(spellBookOpen){
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))clickProgression();
        if(IsKeyPressed(KEY_ENTER)){game.chooseSpell(Spell::Fireball);spellBookOpen=false;}
        if(IsKeyPressed(KEY_ESCAPE)||IsKeyPressed(KEY_K))spellBookOpen=false;
        return true;
    }
    if(!welcome&&!help&&!paused&&!game.dead()){
        if(IsKeyPressed(KEY_U)){openSkills();return true;}
        if(IsKeyPressed(KEY_P)){openStats();return true;}
        if(IsKeyPressed(KEY_K)){openSpellBook();return true;}
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&clickProgression())return true;
    }
    return false;
}
void drawStatsMenu(){
    const char* names[]={"Siła","Moc magiczna","Obrona","Obrona magiczna","Wytrzymałość","Mana"};
    const char* descriptions[]={"+2 obrażenia ataku fizycznego za punkt.","+3 obrażenia magiczne za punkt.",
        "+1 obrony fizycznej za punkt.","+1 obrony magicznej za punkt.","+10 maksymalnego HP za punkt.","+5 maksymalnej many za punkt."};
    rect(0,0,W,H,rgb(6,15,18,215));panel({296,101,848,666},Panel);line(324,122,1116,122,Gold);
    center(game.allocationRequired?TextFormat("AWANS — POZIOM %d",game.level):"STATYSTYKI BOHATERA",720,147,30,Text,true);
    center("5 punktów za każdy zdobyty poziom",720,193,17,Muted);
    center(game.allocationRequired?TextFormat("Pozostało: %d / %d punktów",draftRemaining(),game.attributePoints):"Kolejne punkty zdobędziesz po awansie.",720,226,19,Gold);
    for(int i=0;i<AttributeCount;i++){
        float y=266+i*56.f;line(330,y+53,1106,y+53,Line);
        txt(names[i],336,y+2,20,Text);txt(descriptions[i],336,y+28,14,Muted);
        center(std::to_string(game.attributes[i]+statDraft[i]),899,y+13,22,statDraft[i]?Green:Gold);
        for(bool add:{false,true}){
            Rectangle r=statAdjust(i,add);bool enabled=game.allocationRequired&&(add?draftRemaining()>0:statDraft[i]>0);
            panel(r,enabled&&inside(r)?Card:Panel);center(add?"+":"-",r.x+r.width/2,r.y+5,24,enabled?Gold:Line);
        }
    }
    center(TextFormat("Atak %d   ·   Magia %d   ·   HP %d   ·   Mana %d",game.damage()+statDraft[0]*2,
        game.magicDamage()+statDraft[1]*3,game.maxHp()+statDraft[4]*10,game.maxMana()+statDraft[5]*5),720,630,17,Text);
    Rectangle button=statsConfirm();bool ready=!game.allocationRequired||draftRemaining()==0;
    panel(button,ready&&inside(button)?rgb(53,73,57):Card);
    center(game.allocationRequired?"DALEJ — WYBÓR UMIEJĘTNOŚCI":"WRÓĆ DO GRY",720,699,18,ready?Gold:Muted);
    center(game.allocationRequired?"Świat jest zatrzymany. Rozdaj wszystkie punkty i zatwierdź.":"ENTER / ESC / P — wróć do gry",720,742,14,Muted);
}
void drawSpellBook(){
    rect(0,0,W,H,rgb(6,15,18,215));panel({340,213,760,454},Panel);line(368,234,1072,234,Gold);
    center("KSIĘGA CZARÓW",720,259,30,Text,true);center("Wybierz czar używany klawiszem C",720,302,17,Muted);
    Rectangle card=spellChoice();panel(card,inside(card)?Card:Panel);DrawRectangleLinesEx(card,1,Gold);
    fireIcon(442,405,1.5f,game.time*3);txt("Kula ognia",487,347,25,Text,true);
    txt(TextFormat("Obrażenia magiczne: %d   ·   Koszt: 10 many",game.magicDamage()),487,388,17,Gold);
    txt("Zasięg: 8 pól   ·   Odnowienie: 0,8 s",487,419,16,Muted);
    txt("Celuj kursorem. Kula wybucha po trafieniu.",487,446,15,Muted);
    panel(spellConfirm(),inside(spellConfirm())?Card:Panel);center("WYBIERZ KULĘ OGNIA",720,509,18,Gold);
    center(game.selectedSpell==Spell::Fireball?"Wybrany czar: Kula ognia":"Nie wybrano czaru",720,569,16,Green);
    center(TextFormat("Mana: %d / %d   ·   Regeneracja: 1 punkt co 5 s",int(game.mana),game.maxMana()),720,598,15,Blue);
    center("Świat jest zatrzymany. ESC / K — wróć bez zmiany.",720,625,15,Muted);
}
void drawProgression(){syncAdvancement();if(statsOpen)drawStatsMenu();else if(skillsOpen)drawSkillsMenu();else if(spellBookOpen)drawSpellBook();}
