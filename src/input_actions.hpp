#pragma once

// The sword tile indicates the world-click attack; UI clicks never swing it.
bool gameplayInputAllowed(){return !developerMenuOpen&&!towerPanelOpen&&!intro.active()&&!towerStory.active()&&!game.storyBlocked()&&!weaponWheelOpen&&!welcome&&!help&&!paused&&!audioOpen&&!statsOpen&&!skillsOpen&&!spellBookOpen&&quickPickerSlot<0&&!game.allocationRequired&&!game.dead();}
void activateMouseAction(int index,V movement) {
    if(index==1)openSpellBook();
    if(index==4)game.startRoll(movement);
}
V combatAimFromPointer(){
 WorldHover hit=pickWorld(mouse);if(hit.kind==HoverKind::Enemy)if(Enemy* enemy=game.findEnemy(hit.index))return enemy->pos-game.player;
 return toWorld({mouse.x,mouse.y+35})-game.player;
}
bool releaseWorldBow(){
 if(!game.bowDrawing)return false;
 if(!gameplayInputAllowed()||!worldPointerAllowed(mouse)||dragging.place!=Place::None){game.cancelBow();return false;}
 game.aimBow(combatAimFromPointer());return game.releaseBow(true);
}
bool releaseWorldAttack(){
 if(game.bowDrawing)return releaseWorldBow();
 if(!gameplayInputAllowed()||!worldPointerAllowed(mouse)||dragging.place!=Place::None){game.cancelMeleeHold();return false;}
 if(!game.holdingMelee())return false;
 return game.releaseMeleeHold(combatAimFromPointer());
}
bool clickWorldOrInventory(double now=GetTime()) {
    if(!gameplayInputAllowed()||dragging.place!=Place::None){inventoryClicks.clear();return false;}
    Slot slot=hoverSlot();Item* value=game.get(slot);
    if(slot.place!=Place::None){
        if(value&&!value->empty()){
            bool quick=slot.place==Place::Corpse||slot.place==Place::Chest||(slot.place==Place::Bag&&gearSlot(value->kind)>=0);
            if(quick){if(inventoryClicks.press(slot,*value,now,mouse.x,mouse.y))return game.quickTransfer(slot);}
            else inventoryClicks.clear();
            beginDrag(slot);
        }else inventoryClicks.clear();
        return false;
    }
    inventoryClicks.clear();
    if(!worldPointerAllowed(mouse))return false;
    WorldHover hit=pickWorld(mouse);if(hit.kind==HoverKind::Mage)return talkToMage();
    if(hit.kind==HoverKind::Fountain)return game.useFountain(hit.index);
    if(hit.kind==HoverKind::Chest)return game.openChest(hit.index);
    if(hit.kind==HoverKind::TowerObject||hit.kind==HoverKind::TowerStair)return useTowerTarget(hit);
        if(hit.kind==HoverKind::Prop&&game.props[hit.index].type==13){bool ok=game.inVolcano()?game.useVolcanoPortal():game.useDesertPortal();if(ok){dragging={};nearbyPage=0;hovered={};}return ok;}
    V aim=combatAimFromPointer();
    return archetype(game.equipment[0].kind)==Kind::Bow?game.beginBow(aim):game.beginMeleeHold(aim);
}
bool rightClickWorldOrInventory(){
    inventoryClicks.clear();
    if(!gameplayInputAllowed()||dragging.place!=Place::None)return false;
    game.cancelBow();
    if(worldPointerAllowed(mouse)){
        WorldHover hit=pickWorld(mouse);
        if(hit.kind==HoverKind::Fountain)return game.useFountain(hit.index);
    if(hit.kind==HoverKind::Chest)return game.openChest(hit.index);
        if(hit.kind==HoverKind::Mage)return talkToMage();
        if(hit.kind==HoverKind::TowerObject||hit.kind==HoverKind::TowerStair)return useTowerTarget(hit);
        if(hit.kind==HoverKind::Prop&&game.props[hit.index].type==13){bool ok=game.inVolcano()?game.useVolcanoPortal():game.useDesertPortal();if(ok){dragging={};nearbyPage=0;hovered={};}return ok;}
        if(hit.kind==HoverKind::Corpse)return game.openCorpse(hit.index);
        if(hit.kind==HoverKind::Forage)return game.use({Place::Forage,hit.index});
        if(hit.kind==HoverKind::Ground)return game.use({Place::Ground,hit.index});
        return false;
    }
    return game.use(hoverSlot());
}
bool castGameplaySpell(){return gameplayInputAllowed()&&dragging.place==Place::None&&castSelectedSpell();}
bool activateGameplaySkill(Skill skill){
 if(!gameplayInputAllowed()||dragging.place!=Place::None)return false;
 bool used=skill==Skill::StrongCut?game.strongCut(worldPointerAllowed(mouse)?combatAimFromPointer():game.facing):skill==Skill::PiercingArrow?game.preparePiercingArrow():skill==Skill::Healing&&game.heal();
 if(!used){game.say(game.sandTrapped()&&skill!=Skill::Healing?"Piaskowe więzienie blokuje ataki.":game.skillRank(skill)<=0?"Najpierw naucz się tej umiejętności (U).":!game.skillWeaponMatches(skill)?"Ta umiejętność wymaga odpowiedniej broni.":skill==Skill::Healing&&game.hp>=game.maxHp()?"Zdrowie jest już pełne.":"Za mało many, brak strzał lub odnowienie umiejętności.");game.sound(Cue::Reject,game.player);}
 return used;
}
bool activateSkillSlot(Skill skill){
 if(!gameplayInputAllowed()||dragging.place!=Place::None)return false;
 if(game.skillAssigned[int(skill)])return activateGameplaySkill(skill);
 game.say("Brak przypisania. Kliknij kafelek, aby wybrać umiejętność.");return false;
}
bool activatePotionSlot(int slot){
 if(!gameplayInputAllowed())return false;
 const Kind kind=game.potionAssigned[slot];
 if(kind!=Kind::None)return game.potion(kind);
 game.say("Brak przypisania. Kliknij kafelek, aby wybrać miksturę.");return false;
}
constexpr int QuickPickerSlotCount=SkillCount+int(DefaultPotions.size());
Rectangle quickPickerTile(int slot){
 return slot<SkillCount?skillActionRect(slot):actionRect(slot-SkillCount+2);
}
void leftClickGameplay(V movement,double now=GetTime()){
    if(!gameplayInputAllowed()||dragging.place!=Place::None){inventoryClicks.clear();return;}
    for(int i=0;i<QuickPickerSlotCount;i++)if(inside(quickPickerTile(i))){
        inventoryClicks.clear();game.cancelBow();quickPickerSlot=i;return;
    }
    if((game.openedCorpse>=0||game.openedChest>=0)&&inside(corpseCloseRect())){game.closeCorpse();return;}
    if((game.findCorpse(game.openedCorpse)||game.findChest(game.openedChest))&&inside(takeAllLootRect())){
        inventoryClicks.clear();game.cancelBow();game.takeAllLoot();syncCorpsePage();return;
    }
    for(int i=0;i<5;i++)if(inside(actionRect(i))){activateMouseAction(i,movement);return;}
    if(mouse.y>=668&&mouse.y<696&&mouse.x>=1286&&mouse.x<1404){if(game.findCorpse(game.openedCorpse)){syncCorpsePage();corpsePage=std::clamp(corpsePage+(mouse.x<1344?-1:1),0,corpsePages()-1);}else nearbyPage+=mouse.x<1344?-1:1;return;}
    clickWorldOrInventory(now);
}
