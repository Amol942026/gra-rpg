#pragma once

// The sword tile indicates the world-click attack; UI clicks never swing it.
bool gameplayInputAllowed(){return !welcome&&!help&&!paused&&!audioOpen&&!statsOpen&&!spellBookOpen&&!game.allocationRequired&&!game.dead();}
void activateMouseAction(int index,V movement) {
    if(index==1)openSpellBook();
    if(index==2)game.potion(Kind::Health);
    if(index==3)game.potion(Kind::Mana);
    if(index==4)game.startRoll(movement);
}
bool clickWorldOrInventory() {
    if(!gameplayInputAllowed()||dragging.place!=Place::None)return false;
    Slot slot=hoverSlot();Item* value=game.get(slot);
    if(slot.place!=Place::None){
        if(value&&!value->empty())beginDrag(slot);
        return false;
    }
    return worldPointerAllowed(mouse)&&game.attack();
}
bool rightClickWorldOrInventory(){
    if(!gameplayInputAllowed()||dragging.place!=Place::None)return false;
    if(worldPointerAllowed(mouse)){
        WorldHover hit=pickWorld(mouse);
        if(hit.kind==HoverKind::Corpse)return game.openCorpse(hit.index);
        if(hit.kind==HoverKind::Ground)return game.use({Place::Ground,hit.index});
        return false;
    }
    return game.use(hoverSlot());
}
bool castGameplaySpell(){return gameplayInputAllowed()&&dragging.place==Place::None&&castSelectedSpell();}
void leftClickGameplay(V movement){
    if(!gameplayInputAllowed()||dragging.place!=Place::None)return;
    if(game.openedCorpse>=0&&inside(corpseCloseRect())){game.closeCorpse();return;}
    for(int i=0;i<5;i++)if(inside(actionRect(i))){activateMouseAction(i,movement);return;}
    if(game.openedCorpse<0&&mouse.y>=668&&mouse.y<696&&mouse.x>=1290&&mouse.x<1404){nearbyPage+=mouse.x<1340?-1:1;return;}
    clickWorldOrInventory();
}
