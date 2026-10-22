#pragma once
bool runMushroomQA(){
 fs::create_directories("qa-v22");std::ofstream report("qa-v22/mushroom-tests.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const char* label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
 game=Game{};welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=false;dragging={};nearbyPage=0;game.messageTime=0;
 check(!audioSystem.available,"mushroom diagnostic opens no audio device");
 check(game.forage.size()==MushroomCount,"real game creates 48 collectible mushrooms");
 V plant=game.forage[0].pos;game.player=plant+V{1,0};refreshDisplayLayout();mouse={0,0};renderScene();saveTexture(canvas,"qa-v22/mushrooms-in-forest.png");
 game.props.clear();game.enemies.clear();game.forage.resize(2);game.forage[1].pos=plant+V{0,1};game.hp=40;
 auto pointAt=[](Rectangle r){mouse={r.x+r.width/2,r.y+r.height/2};};
 auto pointAtPlant=[](const Ground& m){Vector2 p=screen(m.pos);for(int y=-26;y<=0;y+=2)for(int x=-14;x<=14;x+=2){Vector2 q{p.x+x,p.y+y};WorldHover h=pickWorld(q);if(h.kind==HoverKind::Forage&&h.index==m.id){mouse=q;return true;}}return false;};
 check(pointAtPlant(game.forage[0]),"visible mushroom can be picked with the real world pointer handler");
 hovered=pickWorld(mouse);Inspection info=inspectWorld(hovered);
 check(info.name=="Borowik"&&info.kind=="Grzyb jadalny"&&info.detail.find("10 HP")!=std::string::npos,"hover HUD identifies edible mushroom and healing amount");
 check(!rightClickWorldOrInventory()&&game.hp==40&&!game.forage[0].value.empty(),"PPM on growing mushroom asks player to collect it first");
 leftClickGameplay({});check(dragging.place==Place::Forage&&game.swing==0,"LPM on mushroom starts dragging without a sword attack");
 V player=game.player;game.player=plant+V{8,0};pointAt(bagRect(24));finishDrag();
 check(!game.forage[0].value.empty()&&game.bag[24].empty()&&dragging.place==Place::None,"walking out of range cancels collection without losing mushroom");
 game.player=player;pointAtPlant(game.forage[0]);leftClickGameplay({});pointAt(bagRect(24));finishDrag();
 check(game.forage[0].value.empty()&&game.bag[24].kind==Kind::Mushroom&&game.hp==40,"map-to-backpack drag harvests mushroom without healing immediately");
 auto slots=nearbySlots();int j=-1;for(int i=0;i<int(slots.size());i++)if(slots[i].place==Place::Forage&&slots[i].index==game.forage[1].id)j=i;
 check(j>=0,"uncollected mushrooms appear in the nearby HUD tray");
 if(j>=0){nearbyPage=j/5;pointAt(lootRect(j%5));check(hoverSlot().place==Place::Forage,"nearby HUD slot points to a growing mushroom");leftClickGameplay({});pointAt(bagRect(24));finishDrag();}
 check(game.bag[24].count==2&&game.forage[1].value.empty(),"HUD-to-backpack drag stacks the second harvested mushroom");
 pointAt(bagRect(24));hovered={};renderScene();saveTexture(canvas,"qa-v22/collected-mushrooms.png");
 check(rightClickWorldOrInventory()&&game.hp==50&&game.bag[24].count==1&&game.mana==100,"PPM on food heals ten HP and consumes exactly one without magic");
 check(!rightClickWorldOrInventory()&&game.hp==50&&game.bag[24].count==1,"immediate repeat use is blocked by cooldown");
 game.potionCd=0;game.hp=100;check(!rightClickWorldOrInventory()&&game.bag[24].count==1,"full health preserves collected food");
 game.hp=96;check(rightClickWorldOrInventory()&&game.hp==100&&game.bag[24].empty(),"last mushroom heals up to maximum HP then clears its slot");
 mouse={0,0};renderScene();saveTexture(canvas,"qa-v22/mushroom-healing.png");
 game.bag[24]=item(Kind::Mushroom);game.potionCd=0;game.hp=40;pointAt(bagRect(24));paused=true;
 check(!rightClickWorldOrInventory()&&game.hp==40&&game.bag[24].count==1,"paused game cannot eat food");paused=false;game=Game{};mouse={0,0};dragging={};hovered={};
 report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
