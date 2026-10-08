#pragma once

bool runCorpseQA(){
    fs::create_directories("qa-v13");std::ofstream report("qa-v13/corpse-tests.txt");int checks=0,failures=0;
    auto check=[&](bool ok,const char* text){checks++;if(!ok)failures++;report<<(ok?"PASS: ":"FAIL: ")<<text<<'\n';report.flush();};
    auto reset=[](){game=Game{};game.props.clear();game.enemies.clear();game.ground.clear();game.player={15,15};
        welcome=help=paused=audioOpen=statsOpen=spellBookOpen=false;dragging={};mouse={0,0};nearbyPage=0;};
    auto pointAt=[](Rectangle r){mouse={r.x+r.width/2,r.y+r.height/2};};
    auto pointAtBody=[](int id){
        const Enemy* e=game.findCorpse(id);if(!e)return false;Vector2 p=screen(e->pos);
        for(int y=-40;y<=30;y+=2)for(int x=-58;x<=58;x+=2){
            Vector2 q{p.x+x,p.y+y};WorldHover h=pickWorld(q);
            if(h.kind==HoverKind::Corpse&&h.index==id){mouse=q;return true;}
        }
        return false;
    };
    reset();prepareCanvas(1.f);const V offsets[]={{-2,0},{0,-2},{2,0},{0,2}};
    for(int type=0;type<4;type++){
        V p=game.player+offsets[type];game.enemies.push_back({101+type,type,p,p,1,100});
        game.enemies.back().animation.direction=static_cast<Direction>(type*2);
        game.hit(game.enemies.back(),30);
        if(game.allocationRequired){Attributes points{};points[0]=game.attributePoints;game.distribute(points);}
    }
    check(game.ground.empty()&&game.kills==4,"four real kills leave only bodies with no scattered ground loot");
    for(int type=0;type<4;type++){
        int id=101+type;check(pointAtBody(id),"actual fallen artwork can be picked for every monster species");
        Inspection info=inspectWorld(pickWorld(mouse));
        check(info.source==HoverKind::Corpse&&info.name==std::string("Ciało: ")+enemyInfo(type).name&&info.maxHp==0,"hover HUD identifies the body without living combat stats");
        float mana=game.mana;check(rightClickWorldOrInventory()&&game.openedCorpse==id,"PPM on the body opens its own loot panel");
        check(game.mana==mana&&game.fireballs.empty()&&game.swing==0,"opening bodies never casts or swings a weapon");
        pointAt(lootRect(0));Slot slot=hoverSlot();
        check(slot.place==Place::Corpse&&slot.owner==id&&slot.index==0&&game.get(slot)->kind==Kind::Gold,"HUD slots bind to the opened corpse by stable ID");
        mouse={0,0};renderScene();saveTexture(canvas,TextFormat("qa-v13/body-%d.png",type));
    }
    // Make one body deterministic to check the complete drag and stacking path.
    game.openCorpse(101);Enemy* body=game.findCorpse(101);body->loot={};
    body->loot[0]=item(Kind::Gold,0,11);body->loot[1]=item(Kind::Health,0,4);body->loot[2]=item(Kind::Sword,1);
    pointAt(lootRect(0));leftClickGameplay({});
    check(dragging.place==Place::Corpse&&dragging.owner==101&&game.swing==0,"LPM on corpse loot starts dragging without attacking");
    check(!castGameplaySpell()&&!rightClickWorldOrInventory()&&game.mana==game.maxMana(),"dragging a corpse item blocks C magic and PPM actions");
    pointAt(bagRect(8));renderScene();saveTexture(canvas,"qa-v13/corpse-drag-to-bag.png");finishDrag();
    check(body->loot[0].empty()&&game.bag[8].kind==Kind::Gold&&game.bag[8].count==11&&dragging.place==Place::None,"dragging to backpack removes the same item from the corpse");
    pointAt(lootRect(1));game.hp=20;int count=body->loot[1].count;
    check(!rightClickWorldOrInventory()&&game.hp==20&&body->loot[1].count==count,"PPM inside corpse loot cannot drink uncollected potions");
    game.bag[10]=item(Kind::Health,0,98);leftClickGameplay({});pointAt(bagRect(10));finishDrag();
    check(game.bag[10].count==99&&body->loot[1].count==3,"partial backpack stacking leaves excess visible in the body");
    pointAt(lootRect(2));leftClickGameplay({});pointAt(equipRect(0));renderScene();saveTexture(canvas,"qa-v13/corpse-invalid-drop.png");finishDrag();
    check(body->loot[2].kind==Kind::Sword&&game.equipment[0].tier==0,"corpse weapon must be collected in backpack before equipping");
    pointAt(lootRect(2));leftClickGameplay({});pointAt(bagRect(0));finishDrag();
    check(body->loot[2].kind==Kind::Sword&&game.bag[0].kind==Kind::Health,"dropping on a different occupied bag slot preserves both items");
    pointAt(lootRect(2));leftClickGameplay({});mouse={650,440};finishDrag();
    check(body->loot[2].kind==Kind::Sword&&game.ground.empty(),"dropping corpse loot on the map cancels without scattering it");
    pointAt(corpseCloseRect());leftClickGameplay({});
    check(game.openedCorpse==-1&&body->loot[2].kind==Kind::Sword,"close button preserves uncollected corpse contents");
    pointAtBody(101);rightClickWorldOrInventory();
    check(game.openedCorpse==101&&body->loot[2].kind==Kind::Sword,"body reopens with its remaining contents");
    pointAt(lootRect(2));leftClickGameplay({});pointAt(bagRect(12));finishDrag();
    pointAt(lootRect(1));leftClickGameplay({});pointAt(bagRect(13));finishDrag();
    check(body->loot[1].empty()&&body->loot[2].empty()&&game.findCorpse(101),"emptying a body keeps the corpse available on the map");
    mouse={0,0};renderScene();saveTexture(canvas,"qa-v13/empty-body.png");
    pointAtBody(102);rightClickWorldOrInventory();check(game.openedCorpse==102,"PPM on another body switches the HUD to that body");
    game.player={19,15};game.update(.01f,{},false);check(game.openedCorpse==-1,"leaving loot range closes the panel");
    check(pointAtBody(101)&&!rightClickWorldOrInventory()&&game.openedCorpse==-1,"distant body cannot be opened even when visible under the cursor");
    game.player={15,15};pointAtBody(101);paused=true;
    check(!rightClickWorldOrInventory()&&!castGameplaySpell(),"pause blocks body opening and C magic");paused=false;
    game.props.push_back({game.enemies[0].pos,0,1});
    Vector2 p=screen(game.enemies[0].pos);mouse={p.x,p.y-9};
    check(pickWorld(mouse).kind!=HoverKind::Corpse,"a foreground tree prevents picking a body through its artwork");game.props.clear();
    // C aims at living targets; PPM on them has no combat action.
    V target=game.player+V{3,-2};game.enemies.push_back({200,0,target,target,100,100});
    p=screen(target);mouse={p.x,p.y-18};game.mana=100;game.spellCd=0;
    check(!rightClickWorldOrInventory()&&game.mana==100&&game.fireballs.empty(),"PPM over a living monster no longer casts magic");
    check(castGameplaySpell()&&game.mana==90&&game.fireballs.size()==1,"C handler casts the chosen fireball for ten mana");
    check(distance(normalized(game.fireballs[0].velocity),normalized(target-game.player))<.001f,"C fireball aims at the actual living monster under the cursor");
    game.enemies.pop_back();game.fireballs.clear();game.mana=100;game.spellCd=0;game.openCorpse(104);mouse={0,0};
    for(float factor:{.8f,1.f,1.2f}){prepareCanvas(factor);renderScene();saveTexture(canvas,TextFormat("qa-v13/corpse-panel-%d.png",int(factor*100)));}
    report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
