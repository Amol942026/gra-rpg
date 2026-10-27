#pragma once

Rectangle statsButton(){return {1238,16,169,29};}
Rectangle statAdjust(int row,bool add){return {add?951.f:878.f,273.f+row*56,add?72.f:68.f,36};}
Rectangle statAddFive(int row){return {1028,273.f+row*56,84,36};}
Rectangle statRow(int row){return {330,266.f+row*56,782,54};}
Rectangle statsConfirm(){return {520,685,400,47};}
Rectangle spellChoice(){return {389,331,662,142};}
Rectangle spellConfirm(){return {540,496,360,46};}

int draftRemaining(){return game.attributePoints-std::accumulate(statDraft.begin(),statDraft.end(),0);}
void openStats(){
    invalidatePausedWorld();
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
bool adjustStat(int row,bool add,int amount=1){
    if(!game.allocationRequired||row<0||row>=AttributeCount||amount<=0)return false;
    if(add){if(draftRemaining()<amount)return false;statDraft[row]+=amount;}
    else{if(statDraft[row]<amount)return false;statDraft[row]-=amount;}
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
        for(int i=0;i<AttributeCount;i++)if(inside(statAddFive(i))){adjustStat(i,true,5);return true;}
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
struct AttributeHelp {const char* name;const char* effect;const char* detail;const char* five;};
constexpr AttributeHelp AttributeHints[]={
 {"Siła","+2 obrażenia fizyczne za punkt.","Wzmacnia miecze, topory, sztylety i łuki. Zwiększa też obrażenia Mocnego cięcia i Przeszywającej strzały. Nie wpływa na obrażenia magii.","5 punktów: +10 do ataku fizycznego."},
 {"Moc magiczna","+3 obrażenia magiczne za punkt.","Zwiększa obrażenia Kuli ognia. Nie podnosi kosztu czaru ani nie wzmacnia Leczenia, które zawsze odnawia 30 HP.","5 punktów: +15 obrażeń Kuli ognia."},
 {"Obrona","+1 obrony fizycznej za punkt.","Chroni przed trafieniami fizycznymi. Każdy punkt obrony zmniejsza takie obrażenia o 0,45 HP. Trafienie zadaje co najmniej 2 HP. Nie chroni przed magią.","5 punktów: +5 obrony, do 2,25 HP mniej na cios."},
 {"Obrona magiczna","+1 obrony magicznej za punkt.","Chroni przed atakami magicznymi. Każdy punkt zmniejsza ich obrażenia o 0,45 HP. Trafienie zadaje co najmniej 2 HP. Nie zmniejsza obrażeń fizycznych.","5 punktów: +5 obrony, do 2,25 HP mniej na czar."},
 {"Wytrzymałość","+10 maksymalnego HP za punkt.","Zwiększa maksymalne zdrowie. Po zatwierdzeniu każdy punkt dodaje także 10 do obecnego HP, zachowując wcześniejszy ubytek zdrowia.","5 punktów: +50 maksymalnego i obecnego HP."},
 {"Mana","+5 maksymalnej many za punkt.","Zwiększa zasób many i po zatwierdzeniu dodaje tyle samo do obecnej many. Nie zmienia kosztów zdolności ani regeneracji: 1 punkt many co 5 sekund.","5 punktów: +25 maksymalnej i obecnej many."}
};
static_assert(std::size(AttributeHints)==AttributeCount);
std::vector<std::string> attributeRows(const std::string& text,float width){
 std::istringstream words(text);std::string word,row;std::vector<std::string> rows;
 while(words>>word){std::string next=row.empty()?word:row+" "+word;if(!row.empty()&&textWidth(next,16)>width){rows.push_back(row);row=word;}else row=next;}
 if(!row.empty())rows.push_back(row);return rows;
}
int hoveredAttribute(){for(int i=0;i<AttributeCount;i++)if(inside(statRow(i)))return i;return -1;}
const std::vector<std::string>& attributeDetailRows(int i){
 static std::array<std::vector<std::string>,AttributeCount> rows;static float cachedScale=-1;
 if(cachedScale!=renderScale){for(int index=0;index<AttributeCount;index++)rows[index]=attributeRows(AttributeHints[index].detail,418);cachedScale=renderScale;}
 return rows[i];
}
int attributePreviewValue(int i){switch(Attribute(i)){
 case Attribute::Strength:return game.damage()+statDraft[i]*2;case Attribute::MagicPower:return game.magicDamage()+statDraft[i]*3;
 case Attribute::Defense:return game.defense()+statDraft[i];case Attribute::MagicDefense:return game.magicDefense()+statDraft[i];
 case Attribute::Vitality:return game.maxHp()+statDraft[i]*10;case Attribute::Mana:return game.maxMana()+statDraft[i]*5;default:return 0;}}
Rectangle attributeTooltipBounds(int i){
 float width=450,height=139+attributeDetailRows(i).size()*22;
 float x=mouse.x+20,y=mouse.y+20;if(x+width>W-12)x=mouse.x-width-20;if(y+height>H-12)y=mouse.y-height-20;
 return {std::clamp(x,8.f,W-width-12),std::clamp(y,8.f,H-height-12),width,height};
}
void drawAttributeTooltip(){
 if(!statsOpen||welcome||paused||help||audioOpen)return;int i=hoveredAttribute();if(i<0)return;
 const auto& info=AttributeHints[i];Rectangle box=attributeTooltipBounds(i);panel(box,rgb(12,22,25,253));rect(box.x,box.y,3,box.height,Gold);
 txt(info.name,box.x+16,box.y+12,20,Text);txt(info.effect,box.x+16,box.y+42,15,Gold);
 int base=attributePreviewValue(i)-statDraft[i]*(i==0?2:i==1?3:i==4?10:i==5?5:1);
 txt(TextFormat("Wartość teraz: %d  →  %d w podglądzie",base,attributePreviewValue(i)),box.x+16,box.y+68,16,statDraft[i]?Green:Muted);
 float y=box.y+101;for(const auto& row:attributeDetailRows(i)){txt(row,box.x+16,y,16,Text);y+=22;}
 txt(info.five,box.x+16,box.y+box.height-28,15,Green);
}
void drawStatsMenu(){
    rect(0,0,W,H,rgb(6,15,18,215));panel({296,101,848,666},Panel);line(324,122,1116,122,Gold);
    center(game.allocationRequired?TextFormat("AWANS — POZIOM %d",game.level):"STATYSTYKI BOHATERA",720,147,30,Text,true);
    center("5 punktów za każdy zdobyty poziom",720,193,17,Muted);
    center(game.allocationRequired?TextFormat("Pozostało: %d / %d punktów",draftRemaining(),game.attributePoints):"Kolejne punkty zdobędziesz po awansie.",720,226,19,Gold);
    for(int i=0;i<AttributeCount;i++){
        float y=266+i*56.f;line(330,y+53,1106,y+53,Line);
        txt(AttributeHints[i].name,336,y+2,20,Text);txt(AttributeHints[i].effect,336,y+28,14,Muted);
        center(std::to_string(game.attributes[i]+statDraft[i]),839,y+13,22,statDraft[i]?Green:Gold);
        for(bool add:{false,true}){
            Rectangle r=statAdjust(i,add);bool enabled=game.allocationRequired&&(add?draftRemaining()>0:statDraft[i]>0);
            if(enabled){panel(r,inside(r)?Card:Panel);center(add?"DODAJ":"COFNIJ",r.x+r.width/2,r.y+10,14,Gold);}
        }
        if(game.allocationRequired&&draftRemaining()>=5){Rectangle r=statAddFive(i);panel(r,inside(r)?Card:Panel);center("DODAJ 5",r.x+r.width/2,r.y+10,14,Gold);}
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
