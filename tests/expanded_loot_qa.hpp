#pragma once
template<class Check>void expandedLootChecks(const fs::path& root,Check check){
 fs::create_directories(root/"equipment");fs::create_directories(root/"items");
 auto reset=[](){game=Game{};game.level=20;intro={};towerStory={};clearTransientUi();welcome=help=paused=false;game.props.clear();game.enemies.clear();game.forage.clear();game.player={20,20};mouse={0,0};game.messageTime=0;};
 auto point=[](Rectangle r){mouse={r.x+r.width/2,r.y+r.height/2};};
 RenderTexture2D sample=LoadRenderTexture(400,320);auto capture=[&](bool portrait=false){actor_art::finishShadowFrame();BeginTextureMode(sample);ClearBackground(BLANK);hero(200,270,2.1f,portrait);EndTextureMode();Image im=LoadImageFromTexture(sample.texture);ImageFlipVertical(&im);return im;};
 auto different=[](Image a,Image b){Color* x=LoadImageColors(a);Color* y=LoadImageColors(b);int n=0;for(int i=0;i<a.width*a.height;i++)n+=x[i].r!=y[i].r||x[i].g!=y[i].g||x[i].b!=y[i].b||x[i].a!=y[i].a;UnloadImageColors(x);UnloadImageColors(y);return n;};
 std::ofstream manifest(root/"items.tsv");manifest<<"id\tname\tshape\trarity\tchance\tpower\tbeast\n";
 int rendered=0;RenderTexture2D sheet=LoadRenderTexture(1440,900);
 for(int k=int(Kind::SlimeBlade);k<ItemKindCount;k++){
  Kind kind=Kind(k);int slot=gearSlot(kind),owner=-1;for(int type=0;type<EnemyTypeCount;type++)for(auto rule:enemyLootTable(type))if(rule.kind==kind)owner=type;
  reset();game.equipment[slot]={};Image before=capture(),portraitBefore=capture(true);game.bag[10]=item(kind);
  point(bagRect(10));leftClickGameplay({},10);dragging={};leftClickGameplay({},10.15);
  check(game.equipment[slot].kind==kind&&game.bag[10].empty(),"real double LPM equips new gear to its correct slot: "+itemName(item(kind)));
  Image after=capture(),portraitAfter=capture(true);
  check(different(before,after)>3&&different(portraitBefore,portraitAfter)>3,"new item visibly changes both equipped world hero and portrait");
  check(ExportImage(after,(root/"equipment"/(std::to_string(k)+".png")).string().c_str()),"new equipped hero frame written");
  int index=k-int(Kind::SlimeBlade);if(index%12==0){BeginTextureMode(sheet);ClearBackground(Ink);txt("NOWY ŁUP BESTII",24,17,28,Gold,true);EndTextureMode();}
  Texture2D picture=LoadTextureFromImage(after);float x=20+(index%4)*355.f,y=70+((index%12)/4)*270.f;
  BeginTextureMode(sheet);panel({x,y,340,250},Panel);float size=17;std::string name=itemName(item(kind));while(size>15&&textWidth(name,size)>314)size--;txt(name,x+12,y+12,size,rarity(item(kind)));
  txt(std::string(rarityName(itemRarity(kind)))+" · "+std::to_string(itemDropChance(kind))+"%",x+12,y+42,15,Gold);txt(std::string(slot==0||slot==5?"Atak +":"Pancerz +")+std::to_string(power(item(kind))),x+12,y+65,15,Muted);
  DrawTexturePro(picture,{0,0,400,320},{x+17,y+70,250,200},{0,0},0,WHITE);icon(item(kind),x+294,y+126,.85f);EndTextureMode();UnloadTexture(picture);
  if(index%12==11)saveTexture(sheet,(root/("new-items-"+std::to_string(index/12+1)+".png")).string());
  UnloadImage(before);UnloadImage(after);UnloadImage(portraitBefore);UnloadImage(portraitAfter);
  unsigned seed=0;bool found=false;for(;seed<20000&&!found;seed++){std::mt19937 random(seed);for(Item value:rollEnemyLoot(owner,random))found|=value.kind==kind;}
  check(found,"seeded new item appears in its source beast's random loot");reset();game.bag={};game.rng.seed(seed-1);V at=game.player+V{.5f,0};game.enemies.push_back({910,owner,at,at,1,enemyInfo(owner).health});game.hit(game.enemies[0],10000);game.openCorpse(910);
  int source=-1;for(int n=0;n<CorpseLootSize;n++)if(game.enemies[0].loot[n].kind==kind)source=n;
  check(source>=0,"actual killed beast contains the new item");corpsePage=source/5;corpsePageOwner=910;point(lootRect(source%5));
  leftClickGameplay({},20);dragging={};leftClickGameplay({},20.15);
  check(game.bag[0].kind==kind&&game.enemies[0].loot[source].empty(),"double LPM loots the new item from its real corpse page");point(bagRect(0));leftClickGameplay({},21);dragging={};leftClickGameplay({},21.15);
  check(game.equipment[slot].kind==kind,"looted new gear equips through the same actual mouse handler");
  game.bag[10]=item(kind);mouse={bagRect(10).x+20,bagRect(10).y+20};renderScene();saveTexture(canvas,(root/"items"/(std::to_string(k)+".png")).string());
  auto text=itemTooltipText(item(kind),{Place::Bag,10});Rectangle box=tooltipBounds(text);check(textWidth(text.description,14)<=box.width-34&&text.chance.find(std::to_string(itemDropChance(kind))+"%")!=std::string::npos,"new tooltip fits and shows its exact central rarity chance");
  manifest<<k<<'\t'<<itemName(item(kind))<<'\t'<<int(archetype(kind))<<'\t'<<int(itemRarity(kind))<<'\t'<<itemDropChance(kind)<<'\t'<<power(item(kind))<<'\t'<<owner<<'\n';rendered++;
 }
 UnloadRenderTexture(sample);UnloadRenderTexture(sheet);check(rendered==36,"all 36 additional items rendered, equipped and looted");
 check(runRarityQA(),"entire expanded catalogue fits real rarity tooltips");check(runTradeQA(),"all catalogue item values and merchant interactions remain valid");
 std::ofstream out(root/"expansion.json");out<<"{\"status\":\"PASS\",\"new_items\":"<<rendered<<",\"catalogue_items\":"<<ItemKindCount<<",\"equipment_variants\":"<<EquipmentKindCount<<",\"audio_opened\":false}";
}
