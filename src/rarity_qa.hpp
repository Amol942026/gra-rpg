#pragma once
bool runRarityQA(){
 fs::create_directories("qa-v39");std::ofstream report("qa-v39/rarity-ui-tests.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const char* message){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<message<<'\n';report.flush();};
 auto reset=[](){
  game=Game{};game.props.clear();game.enemies.clear();game.ground.clear();game.forage.clear();game.player={20,20};
  welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=weaponWheelOpen=false;intro={};towerStory={};
  dragging={};mouse={0,0};corpsePage=0;corpsePageOwner=-1;nearbyPage=0;hovered={};
 };
 check(!audioSystem.available,"loot diagnostic runs without opening the laptop audio device");
 RenderTexture2D sample=LoadRenderTexture(W,H);
 RenderTexture2D sheet=LoadRenderTexture(1520,1120);BeginTextureMode(sheet);ClearBackground(Ink);txt("ŁUP — RZADKOŚĆ I NOWE WYPOSAŻENIE",28,22,28,Gold);EndTextureMode();
 const Kind examples[]={Kind::WolfFang,Kind::WolfKnife,Kind::MoonBow,Kind::EchoRing};
 for(int k=1;k<ItemKindCount;k++){
  reset();Item value=item(static_cast<Kind>(k));game.bag[10]=value;Rectangle slot=bagRect(10);mouse={slot.x+20,slot.y+20};
  auto text=itemTooltipText(value,{Place::Bag,10});Rectangle box=tooltipBounds(text);
  float size=20;while(size>14&&textWidth(text.name,size)>box.width-34)size--;
  check(textWidth(text.name,size)<=box.width-34&&textWidth(text.label,14)<=box.width-34&&textWidth(text.stat,14)<=box.width-34&&textWidth(text.description,14)<=box.width-34&&textWidth(text.instruction,14)<=box.width-34&&textWidth(text.chance,14)<=box.width-34,"all item text fits its real tooltip without clipped names or descriptions");
  check(text.label==rarityName(itemRarity(value))&&!text.chance.empty(),"every item tooltip exposes its actual rarity and central drop chance");
  BeginTextureMode(sample);ClearBackground(Ink);tooltip();EndTextureMode();
  Image im=LoadImageFromTexture(sample.texture);ImageFlipVertical(&im);Color* pixels=LoadImageColors(im);int colored=0;
  for(int y=int(box.y)+34;y<int(box.y)+59;y++)for(int x=int(box.x)+17;x<int(box.x+box.width)-15;x++){
   Color c=pixels[y*W+x];colored+=std::abs(int(c.r)-text.color.r)<3&&std::abs(int(c.g)-text.color.g)<3&&std::abs(int(c.b)-text.color.b)<3;
  }
  check(colored>8,"the GPU renders each item name in the required rarity color");
  UnloadImageColors(pixels);
  for(int r=0;r<4;r++)if(value.kind==examples[r]){
   ImageCrop(&im,{box.x,box.y,box.width,box.height});Texture2D t=LoadTextureFromImage(im);
   BeginTextureMode(sheet);DrawTexture(t,28+r*374,82,WHITE);EndTextureMode();UnloadTexture(t);
  }
  UnloadImage(im);
 }
 BeginTextureMode(sheet);
 for(int n=0;n<int(BeastGear.size());n++){
  int col=n%4,row=n/4;float x=28+col*374.f,y=310+row*110.f;Item value=item(static_cast<Kind>(int(Kind::WolfKnife)+n));
  panel({x,y,358,96},Panel);icon(value,x+28,y+40,.92f);txt(itemName(value),x+59,y+10,14,rarity(value));
  txt(std::string(rarityName(itemRarity(value)))+" · "+std::to_string(itemDropChance(value))+"%",x+59,y+37,14,Muted);
  txt(std::string(gearSlot(value.kind)==0||gearSlot(value.kind)==5?"Atak +":"Pancerz +")+std::to_string(power(value)),x+59,y+63,14,Gold);
 }
 EndTextureMode();saveTexture(sheet,"qa-v39/new-gear-and-rarities.png");UnloadRenderTexture(sheet);UnloadRenderTexture(sample);
 reset();prepareCanvas(1.f);game.bag[10]=item(Kind::MoonBow);check(game.bindWeapon({Place::Bag,10},0),"a new bow is assignable through the real wheel model");
 weaponWheelOpen=true;mouse={WheelCenter.x,WheelCenter.y-147};Slot wheelSource=hoverSlot();
 check(wheelSource.place==Place::Bag&&wheelSource.index==10&&game.get(wheelSource)->kind==Kind::MoonBow,"hovering a weapon sector resolves the actual named inventory item for its rarity tooltip");
 auto wheelText=itemTooltipText(*game.get(wheelSource),wheelSource);
 check(wheelText.name==itemName(item(Kind::MoonBow))&&wheelText.label=="Bardzo rzadki","weapon wheel hover exposes the full item name and rarity");
 renderScene();saveTexture(canvas,"qa-v39/new-bow-wheel-tooltip.png");
 Rectangle wheelBox=tooltipBounds(wheelText);Image wheelImage=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&wheelImage);
 Color stripe=GetImageColor(wheelImage,int(wheelBox.x)+1,int(wheelBox.y)+48);
 check(stripe.r==wheelText.color.r&&stripe.g==wheelText.color.g&&stripe.b==wheelText.color.b,"real weapon-wheel rendering keeps the item tooltip above the opaque wheel sectors");
 UnloadImage(wheelImage);
 for(float factor:{.8f,1.f,1.2f}){
  reset();prepareCanvas(factor);V pos=game.player+V{1,0};game.enemies.push_back({909,3,pos,pos,0,250});game.enemies.back().alive=false;
  Enemy* body=game.findCorpse(909);
  for(int i=0;i<CorpseLootSize;i++)body->loot[i]=item(static_cast<Kind>(int(Kind::WolfKnife)+i));
  check(game.openCorpse(909)&&corpsePages()==3,"a full body exposes all fifteen slots across three HUD pages");
  mouse={lootRect(4).x+20,lootRect(4).y+20};check(hoverSlot().index==4,"first page points to its actual fifth item");
  mouse={1390,679};leftClickGameplay({});check(corpsePage==1,"real next-page mouse handler selects the second corpse page");
  leftClickGameplay({});check(corpsePage==2,"real next-page mouse handler selects the third corpse page");
  leftClickGameplay({});check(corpsePage==2,"next-page button cannot exceed the body page count");
  mouse={lootRect(4).x+20,lootRect(4).y+20};check(hoverSlot().index==14&&hoverSlot().owner==909,"third-page mouse hit preserves the actual slot and corpse ID");
  hovered={};renderScene();saveTexture(canvas,TextFormat("qa-v39/corpse-page-three-%d.png",int(factor*100)));
  beginDrag(hoverSlot());mouse={bagRect(24).x+20,bagRect(24).y+20};finishDrag();
  check(game.bag[24].kind==Kind::GelAmulet&&body->loot[14].empty(),"last-page loot drags through real pointer handlers into the far-right backpack slot");
  mouse={1300,679};leftClickGameplay({});check(corpsePage==1,"previous-page button restores the second page");
  game.closeCorpse();check(game.openCorpse(909),"reopening the body preserves remaining contents");syncCorpsePage();
  // Resetting the page on a reopen is optional; it must stay valid and resolve the same owner.
  check(corpsePage>=0&&corpsePage<corpsePages(),"reopened corpse page stays within the remaining contents");
  game.enemies.push_back({910,1,pos,pos,0,54});game.enemies.back().alive=false;game.enemies.back().loot[0]=item(Kind::WolfFang);
  game.openCorpse(910);syncCorpsePage();check(corpsePage==0&&corpsePages()==1,"opening a different corpse resets pagination and cannot show another body's items");
  game.bag[20]=item(Kind::EchoRing);mouse={bagRect(20).x+20,bagRect(20).y+20};renderScene();saveTexture(canvas,TextFormat("qa-v39/purple-tooltip-%d.png",int(factor*100)));
 }
 reset();prepareCanvas(1.f);report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
