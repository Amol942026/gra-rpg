#pragma once

bool runControlsQA(){
    fs::create_directories("qa-v13");std::ofstream report("qa-v13/controls-tests.txt");int checks=0,failures=0;
    auto check=[&](bool ok,const char* label){checks++;if(!ok)failures++;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
    auto reset=[](){game=Game{};game.props.clear();game.enemies.clear();game.ground.clear();game.player={15,15};
        welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=false;dragging={};mouse={0,0};nearbyPage=0;};
    auto pointAt=[](Rectangle r){mouse={r.x+r.width/2,r.y+r.height/2};};
    reset();prepareCanvas(1.f);
    for(int i=0;i<3;i++){V p=game.player+V{float(i-1),-.8f};game.enemies.push_back({i,0,p,p,500,500});game.enemies.back().windup=100;}
    Vector2 target=screen(game.enemies[0].pos);mouse={target.x,target.y-18};
    leftClickGameplay({});
    check(game.swing>0&&game.attackCd>0,"LPM world press starts a sword swing");
    for(const auto& e:game.enemies)check(e.hp<500,"one LPM press damages every nearby enemy");
    float hp=game.enemies[0].hp;
    for(int i=0;i<120;i++)game.update(1.f/60,{},false);
    check(game.enemies[0].hp==hp&&game.swing==0,"held mouse without another press never repeats the swing");
    leftClickGameplay({});check(game.enemies[0].hp<hp,"a new LPM press starts another swing after cooldown");

    reset();V position=game.player+V{3,-2};game.enemies.push_back({44,0,position,position,100,100});
    target=screen(position);mouse={target.x,target.y-18};
    check(castGameplaySpell()&&game.mana==90&&game.fireballs.size()==1&&game.swing==0,"C casts the selected spell for ten mana without a sword swing");
    check(distance(normalized(game.fireballs[0].velocity),normalized(position-game.player))<.001f,"C aims at the actual hovered monster position");
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
        spellBookOpen=false;
    }
    reset();
    for(Vector2 p:{Vector2{920,210},Vector2{90,710},Vector2{180,55},Vector2{-20,400}}){
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
    check(distance(game.player,V{15,15}+normalized(V{1,-1})*2.8f)<.002f&&!game.rolling(),"holding run leaves roll distance unchanged");
    for(float factor:{.8f,1.f,1.2f}){
        reset();prepareCanvas(factor);renderScene();saveTexture(canvas,TextFormat("qa-v13/controls-hud-%d.png",int(factor*100)));
        welcome=true;renderScene();saveTexture(canvas,TextFormat("qa-v13/controls-help-%d.png",int(factor*100)));
    }
    reset();openSpellBook();renderScene();saveTexture(canvas,"qa-v13/controls-spell-book.png");
    report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
