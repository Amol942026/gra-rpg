#pragma once
Rectangle towerCloseButton(){return {1069,165,53,38};}
Rectangle towerPageButton(bool next){return {next?960.f:330.f,665,150,43};}
Rectangle towerRecipeButton(int index){return {351.f+index*405,487,320,44};}
bool useTowerTarget(WorldHover hit){
 if(!gameplayInputAllowed()||!game.inTower())return false;
 if(hit.kind==HoverKind::TowerStair){if(!game.climbTower(hit.index)){game.say("Podejdź do schodów, aby zmienić piętro.");return false;}dragging={};nearbyPage=0;hovered={};return true;}
 if(hit.kind!=HoverKind::TowerObject)return false;
 const auto& objects=towerFurnishings(game.towerFloor);
 if(hit.index<0||hit.index>=int(objects.size())||!towerInteractive(objects[hit.index].type))return false;
 if(!game.towerObjectReachable(hit.index)){game.say("Podejdź bliżej wyposażenia (2,3 pola).");return false;}
 int type=objects[hit.index].type;
 if(type==4)return game.restAtTower(hit.index);if(type==9)return game.toggleTowerRune(hit.index);
 clearStoryMenus();towerPanelObject=hit.index;towerPage=0;towerPanelOpen=true;game.messageTime=0;game.sound(Cue::Ui,game.player);return true;
}
bool nearestTowerInteraction(){
 if(!gameplayInputAllowed()||!game.inTower())return false;
 WorldHover target;float best=999;
 if(game.mageReachable()){best=distance(game.player,MagePosition);target.kind=HoverKind::Mage;}
 for(int direction:{-1,1})if(game.stairReachable(direction)){float d=distance(game.player,direction==1?TowerStairUp:TowerStairDown);if(d<best){best=d;target.kind=HoverKind::TowerStair;target.index=direction;}}
 const auto& objects=towerFurnishings(game.towerFloor);
 for(int i=0;i<int(objects.size());i++)if(game.towerObjectReachable(i)){float d=distance(game.player,objects[i].pos);if(d<best){best=d;target.kind=HoverKind::TowerObject;target.index=i;}}
 if(target.kind==HoverKind::Mage)return talkToMage();
 if(target.kind==HoverKind::None){game.say("Podejdź do schodów lub wyposażenia i użyj E / PPM.");return false;}
 return useTowerTarget(target);
}
void closeTowerPanel(){towerPanelOpen=false;towerPanelObject=-1;game.sound(Cue::Ui,game.player);}
bool clickTowerPanel(){
 if(!towerPanelOpen)return false;
 if(inside(towerCloseButton())){closeTowerPanel();return true;}
 int type=towerFurnishings(game.towerFloor)[towerPanelObject].type;
 if(type==7){for(int i=0;i<2;i++)if(inside(towerRecipeButton(i)))game.brewTowerPotion(towerPanelObject,i==0?Kind::Health:Kind::Mana);}
 else for(bool next:{false,true})if(inside(towerPageButton(next)))towerPage=(towerPage+(next?1:2))%3;
 return true;
}
bool towerPanelInput(){
 if(!towerPanelOpen)return false;
 if(!game.inTower()||towerPanelObject<0||towerPanelObject>=int(towerFurnishings(game.towerFloor).size())||game.allocationRequired||game.dead()){closeTowerPanel();return false;}
 if(!IsWindowFocused())return true;
 if(IsKeyPressed(KEY_ESCAPE)||IsKeyPressed(KEY_ENTER)){closeTowerPanel();return true;}
 if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))clickTowerPanel();return true;
}
void towerParagraph(const char* text,float x,float y,float width,Color color=Muted){
 std::istringstream words(text);std::string word,row;float yy=y;
 while(words>>word){std::string candidate=row.empty()?word:row+" "+word;if(!row.empty()&&textWidth(candidate,19)>width){txt(row,x,yy,19,color);yy+=30;row=word;}else row=candidate;}
 if(!row.empty())txt(row,x,yy,19,color);
}
void drawTowerPanel(){
 if(!towerPanelOpen)return;const auto& objects=towerFurnishings(game.towerFloor);if(towerPanelObject<0||towerPanelObject>=int(objects.size()))return;
 int type=objects[towerPanelObject].type;Color accent=towerAccent(game.towerFloor);
 rect(0,0,W,H,rgb(7,12,22,208));panel({290,148,860,622},rgb(22,29,38));line(325,171,1039,171,accent);
 center(type==7?"PRACOWNIA ALCHEMICZNA":type==10?"NIEBO NAD WIEŻĄ":type==12?"PIĘĆ CZĘŚCI SERCA ŚWITU":"KRONIKI ELDRINA",720,196,28,Text,true);
 center(TowerFloors[game.towerFloor].name,720,239,17,accent);panel(towerCloseButton(),inside(towerCloseButton())?Card:Panel);center("X",1095,173,20,Gold);
 if(type==7){
  for(int i=0;i<2;i++){
   float x=329+i*405.f;panel({x,291,377,258},Panel);icon(item(i==0?Kind::Health:Kind::Mana),x+48,341,1.1f);
   txt(i==0?"Mikstura zdrowia":"Mikstura many",x+83,310,22,Text);txt(i==0?"Odnawia 50 HP":"Odnawia 35 many",x+83,346,17,i==0?Green:Blue);
   txt(i==0?"Składniki: 3 borowiki":"Składniki: 3 esencje lasu",x+22,392,18,Gold);
   txt(TextFormat("W plecaku: %d",game.bagCount(i==0?Kind::Mushroom:Kind::ForestEssence)),x+22,425,18,Muted);
   Rectangle button=towerRecipeButton(i);panel(button,inside(button)?Card:Panel);center("UWARZ MIKSTURĘ",button.x+160,button.y+12,17,accent);
  }
  towerParagraph("Gotowa mikstura trafia do plecaka. Przy braku składników lub miejsca nic nie znika. Receptury nie kosztują many.",335,577,770);
  if(game.messageTime>0)center(game.message,720,681,16,Gold);
 }else if(type==10){
  const char* maps[]={"Gwiazdozbiór Wędrowca","Korona Świtu","Ślad drugiego strażnika"};center(maps[towerPage],720,284,24,Gold,true);
  Rectangle map{353,332,734,251};panel(map,rgb(10,20,38));
  for(int i=0;i<65;i++){unsigned h=Game::hash(i,towerPage+41);intro_art::star(369+h%704,347+(h>>11)%220,i%8?1.1f:2.2f,rgb(110+h%92,150+h%73,219));}
  std::array<Vector2,7> points{};for(int i=0;i<7;i++){float a=(i*1.8f+towerPage*.5f);points[i]={720+std::cos(a)*(125+i*10),451+std::sin(a)*(70+i*3)};}
  for(int i=0;i<6;i++)DrawLineEx(points[i],points[i+1],1.5f,Fade(accent,.55f));for(auto p:points){intro_art::glow(p.x,p.y,14,14,Fade(accent,.18f));intro_art::star(p.x,p.y,4,Gold);}
  center(towerPage==2?"Eldrin bada drogę do drugiej krainy.":"Luneta odkrywa dawne znaki na niebie.",720,611,18,Muted);
 }else{
  static const char* titles[]={"Pęknięta Pieczęć Popiołu","Kamienny Strażnik","Wieża pośród gwiazd"};
  static const char* texts[]={"Pieczęć Popiołu chroniła świat przez stulecia. Gdy pękła, Serce Świtu rozpadło się na pięć części. Tylko zebranie wszystkich fragmentów pozwoli zamknąć bramę demonów. Eldrin strzeże kronik, które mogą wskazać pozostałe miejsca.","Pierwszy fragment spoczywał w ruinach Lasu Mchu. Kamienny Strażnik czerpał moc z ziemi, lecz Wędrowiec przełamał jego obronę. Zdobyta część pozostaje w plecaku bohatera; w sali pieczęci oglądasz jedynie jej magiczne odbicie.","Pięć pięter wieży służy przygotowaniu do podróży. Na dole odpoczywają goście, wyżej przechowywane są księgi i powstają mikstury. Runy podtrzymują ochronę wieży. Na szczycie Eldrin obserwuje niebo w poszukiwaniu następnego strażnika."};
  center(type==12?TextFormat("Odzyskane części: %d / 5",game.dawnFragments()):titles[towerPage],720,307,24,accent,true);
  icon(item(type==12?Kind::DawnShard:Kind::RuneStone),720,382,1.7f);towerParagraph(texts[towerPage],348,448,743,Text);
 }
 if(type!=7){for(bool next:{false,true}){Rectangle button=towerPageButton(next);panel(button,inside(button)?Card:Panel);center(next?"DALEJ":"WSTECZ",button.x+75,button.y+12,17,accent);}center(TextFormat("%d / 3",towerPage+1),720,681,17,Muted);}
 center("Gra jest zatrzymana · ENTER / ESC — zamknij",720,735,15,Muted);
}
