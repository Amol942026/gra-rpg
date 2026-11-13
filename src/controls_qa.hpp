#pragma once

bool runControlsQA(){
    fs::create_directories("qa-v13");std::ofstream report("qa-v13/controls-tests.txt");int checks=0,failures=0;
    auto check=[&](bool ok,const char* label){checks++;if(!ok)failures++;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
    auto reset=[](){game=Game{};game.props.clear();game.enemies.clear();game.ground.clear();game.player={15,15};
        game.learnedSpells=spellBit(Spell::Fireball);game.chooseSpell(Spell::Fireball);
        welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=false;quickPickerSlot=-1;dragging={};inventoryClicks.clear();mouse={0,0};nearbyPage=0;};
    auto pointAt=[](Rectangle r){mouse={r.x+r.width/2,r.y+r.height/2};};
    reset();prepareCanvas(1.f);
    for(int i=0;i<3;i++){V p=game.player+V{float(i-1),-.8f};game.enemies.push_back({i,0,p,p,500,500});game.enemies.back().windup=100;}
    Vector2 target=screen(game.enemies[0].pos);mouse={target.x,target.y-18};
    leftClickGameplay({});
    check(game.holdingMelee()&&game.swing==0&&releaseWorldAttack()&&game.swing>0&&game.attackCd>0,"short LPM release starts a sword swing");
    for(const auto& e:game.enemies)check(e.hp<500,"one LPM press damages every nearby enemy");
    float hp=game.enemies[0].hp;
    for(int i=0;i<120;i++)game.update(1.f/60,{},false);
    check(game.enemies[0].hp==hp&&game.swing==0,"held mouse without another press never repeats the swing");
    leftClickGameplay({});releaseWorldAttack();check(game.enemies[0].hp<hp,"a new LPM tap starts another swing after cooldown");

    reset();V position=game.player+V{3,-2};game.enemies.push_back({44,0,position,position,100,100});
    target=screen(position);mouse={target.x,target.y-18};
    check(castGameplaySpell()&&game.mana==90&&game.fireballs.size()==1&&game.swing==0,"C casts the selected spell for ten mana without a sword swing");
    check(!game.fireballs.empty()&&distance(normalized(game.fireballs.front().velocity),normalized(position-game.player))<.001f,"C aims at the actual hovered monster position");
    check(!castGameplaySpell()&&game.mana==90&&game.fireballs.size()==1,"spell cooldown rejects an immediate second click without spending mana");
    game.spellCd=0;game.mana=9;
    check(!castGameplaySpell()&&game.mana==9,"C cannot cast with insufficient mana");

    reset();game.addGround(item(Kind::Sword),game.player+V{-1,0});
    target=screen(game.ground[0].pos);mouse={target.x,target.y-14};leftClickGameplay({});
    check(dragging.place==Place::Ground&&game.swing==0,"LPM on ground loot begins dragging without attacking");
    check(!castGameplaySpell()&&game.mana==100,"C cannot cast while an item is being dragged");
    pointAt(bagRect(7));finishDrag();check(game.bag[7].kind==Kind::Sword&&dragging.place==Place::None,"ground loot still drops into the backpack");
    pointAt(bagRect(0));leftClickGameplay({});
    check(dragging.place==Place::Bag&&game.swing==0,"LPM on a backpack item keeps inventory drag behavior");
    finishDrag();game.hp=20;int count=game.bag[0].count;
    check(rightClickWorldOrInventory()&&game.hp==70&&game.bag[0].count==count-1&&game.mana==100,"PPM on a backpack potion drinks it without casting");
    pointAt(bagRect(12));leftClickGameplay({});
    check(game.swing==0&&!rightClickWorldOrInventory()&&game.mana==100,"empty inventory cells consume clicks without combat");
    reset();game.addGround(item(Kind::Mana),game.player+V{-1,0});
    target=screen(game.ground[0].pos);mouse={target.x,target.y-14};
    check(!rightClickWorldOrInventory()&&game.mana==100&&game.ground[0].value.count==1&&game.fireballs.empty(),"PPM on a ground potion at full mana does not cast or consume it");

    reset();
    for(int i=0;i<5;i++){
        pointAt(actionRect(i));leftClickGameplay({});
        check(game.swing==0,"action-bar clicks never attack");
        check(game.fireballs.empty(),"action-bar clicks never cast into the world");
        spellBookOpen=false;quickPickerEvent(true,false);
    }
    for(int slot=0;slot<int(DefaultPotions.size());slot++){
        reset();game.hp=20;game.mana=30;
        const int picker=SkillCount+slot;
        const auto bounds=quickPickerBounds(picker,2);
        const int count=game.bag[slot].count;
        const auto untouched=[&](){return game.hp==20&&game.mana==30&&game.bag[slot].count==count&&game.potionCd==0;};
        pointAt(actionRect(slot+2));leftClickGameplay({});
        check(quickPickerSlot==picker&&untouched(),"LPM on each potion tile opens its menu without using the potion");
        check(!worldPointerAllowed({600,430})&&!activatePotionSlot(slot)&&untouched(),"potion picker blocks combat and potion hotkeys");
        renderScene();saveTexture(canvas,slot==0?"qa-v13/health-potion-picker.png":"qa-v13/mana-potion-picker.png");
        pointAt(quickPickerOption(bounds,1));quickPickerEvent(false,true);
        check(quickPickerSlot<0&&game.potionAssigned[slot]==Kind::None&&untouched(),"LPM can clear a potion assignment without drinking");
        check(!activatePotionSlot(slot)&&untouched(),"unassigned potion hotkey consumes nothing");
        pointAt(actionRect(slot+2));leftClickGameplay({});
        pointAt(quickPickerOption(bounds,0));quickPickerEvent(false,true);
        check(quickPickerSlot<0&&game.potionAssigned[slot]==DefaultPotions[slot]&&untouched(),"LPM assigns the potion without drinking");
        const int otherCount=game.bag[1-slot].count;
        check(activatePotionSlot(slot)&&game.bag[slot].count==count-1&&game.bag[1-slot].count==otherCount&&
              game.hp==(slot==0?70:20)&&game.mana==(slot==1?65:30),"hotkey drinks only the assigned potion");
        reset();game.hp=20;game.mana=30;game.potionAssigned[slot]=Kind::None;
        pointAt(bagRect(slot));
        check(rightClickWorldOrInventory()&&game.bag[slot].count==count-1,"PPM in the backpack works independently of HUD assignment");
        reset();game.bag[slot]={};game.hp=20;game.mana=30;
        check(!activatePotionSlot(slot)&&game.hp==20&&game.mana==30&&game.bag[1-slot].count==otherCount,
              "a missing assigned potion never falls back to the other kind");
    }
    reset();pointAt(actionRect(2));leftClickGameplay({});
    pointAt(actionRect(3));quickPickerEvent(false,true);
    check(quickPickerSlot==SkillCount+1,"LPM switches directly between potion pickers");
    pointAt(skillActionRect(0));quickPickerEvent(false,true);
    check(quickPickerSlot==0,"LPM switches from a potion picker to a skill picker");
    mouse={400,390};quickPickerEvent(false,true);
    check(quickPickerSlot<0&&game.swing==0,"outside LPM closes the picker without attacking");
    pointAt(actionRect(2));leftClickGameplay({});
    check(quickPickerEvent(true,false)&&quickPickerSlot<0&&!paused,"Escape closes the picker before the pause menu");
    reset();
    for(Vector2 p:{Vector2{minimapRect().x+90,minimapRect().y+118},Vector2{90,710},Vector2{180,55},Vector2{-20,400}}){
        mouse=p;leftClickGameplay({});
        check(!rightClickWorldOrInventory()&&game.mana==100&&game.swing==0,"minimap, journal, title and outside-window clicks do not trigger combat");
    }
    for(bool* blocked:{&welcome,&help,&paused,&audioOpen,&statsOpen,&skillsOpen,&spellBookOpen}){
        reset();*blocked=true;mouse={650,450};leftClickGameplay({});
        check(!rightClickWorldOrInventory()&&!castGameplaySpell()&&game.swing==0&&game.fireballs.empty(),"each modal state blocks combat and corpse interactions");
    }
    reset();game.allocationRequired=true;game.attributePoints=5;mouse={650,450};leftClickGameplay({});
    check(!rightClickWorldOrInventory()&&!game.startRoll({1,0})&&game.swing==0,"mandatory advancement blocks mouse attacks, spells and rolling");
    reset();game.hp=0;mouse={650,450};leftClickGameplay({});
    check(!rightClickWorldOrInventory()&&game.swing==0,"death blocks both mouse combat buttons");

    reset();game.startRoll({1,-1});mouse={650,450};leftClickGameplay({});
    check(!castGameplaySpell()&&game.swing==0&&game.mana==100,"the existing roll blocks sword and spell actions");
    for(int i=0;i<8;i++)game.update(.05f,{},false,true);
    const V runRollEnd=game.player;
    const bool runRollFinished=!game.rolling();
    reset();game.startRoll({1,-1});
    for(int i=0;i<8;i++)game.update(.05f,{},false,false);
    check(runRollFinished&&!game.rolling()&&distance(game.player,V{15,15})>1.f&&
          distance(game.player,runRollEnd)<.002f,"holding run leaves roll distance unchanged");
    for(float factor:{.8f,1.f,1.2f}){
        reset();prepareCanvas(factor);renderScene();saveTexture(canvas,TextFormat("qa-v13/controls-hud-%d.png",int(factor*100)));
        welcome=true;renderScene();saveTexture(canvas,TextFormat("qa-v13/controls-help-%d.png",int(factor*100)));
    }
    reset();openSpellBook();renderScene();saveTexture(canvas,"qa-v13/controls-spell-book.png");
    report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
