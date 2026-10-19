#pragma once
constexpr Vector2 WheelCenter{WorldW/2.f,422};
int weaponWheelHit(Vector2 point){
 float dx=point.x-WheelCenter.x,dy=point.y-WheelCenter.y,r=std::hypot(dx,dy);if(r<76||r>218)return -1;
 float angle=std::atan2(dy,dx)+PI/2+PI/8;if(angle<0)angle+=2*PI;return int(angle/(PI/4))%WeaponWheelSize;
}
void closeWeaponWheel(){weaponWheelOpen=false;dragging={};game.cancelBow();}
void openWeaponWheel(){if(welcome||help||paused||audioOpen||statsOpen||skillsOpen||spellBookOpen||game.allocationRequired||game.dead())return;game.cancelBow();weaponWheelOpen=true;game.say("Przeciągnij broń z plecaka do koła.");audioSystem.play(Cue::Ui);}
void clickWeaponWheel(){
 int sector=weaponWheelHit(mouse);
 if(sector>=0){if(game.weaponWheel[sector].kind==Kind::None){game.say("Przeciągnij broń z plecaka na puste pole.");return;}
  if(game.selectWeapon(sector))closeWeaponWheel();return;}
 Slot slot=hoverSlot();if(slot.place==Place::Bag||slot.place==Place::Equip)beginDrag(slot);
}
bool weaponWheelInput(){
 if(weaponWheelOpen){
  if(!IsWindowFocused()||game.dead()||game.allocationRequired){closeWeaponWheel();if(!IsWindowFocused())paused=true;return true;}
  if(IsKeyPressed(KEY_Q)||IsKeyPressed(KEY_ESCAPE)){closeWeaponWheel();return true;}
  if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&dragging.place==Place::None)clickWeaponWheel();
  if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){int index=weaponWheelHit(mouse);if(index>=0)game.clearWeaponBinding(index);}
  if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT))finishDrag();return true;
 }
 if(IsKeyPressed(KEY_Q)||(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&inside(weaponWheelButton()))){openWeaponWheel();return weaponWheelOpen;}
 return false;
}
void drawWeaponWheelButton(){Rectangle r=weaponWheelButton();panel(r,inside(r)||weaponWheelOpen?Card:Panel);icon(game.equipment[0],r.x+23,r.y+16,.43f);center("BROŃ · Q",r.x+101,r.y+10,14,Gold);}
void drawWeaponWheel(){
 if(!weaponWheelOpen)return;
 rect(0,0,WorldW,H,rgb(5,14,17,185));center("KOŁO BRONI",WheelCenter.x,142,30,Text,true);
 center("Przeciągnij broń z plecaka na pole koła",WheelCenter.x,187,17,Muted);
 int hot=weaponWheelHit(mouse);Item* dragged=game.get(dragging);bool assigning=dragging.place!=Place::None;
 for(int i=0;i<WeaponWheelSize;i++){
  const auto& ref=game.weaponWheel[i];Slot source=game.weaponSource(i);bool available=source.place!=Place::None,active=source.place==Place::Equip;
  Color color=i==hot?Gold:active?Green:Line;
  if(assigning&&i==hot)color=dragged&&gearSlot(dragged->kind)==0&&(dragging.place==Place::Bag||dragging.place==Place::Equip)?Green:rgb(205,99,80);
  float start=-90+i*45-21.5f,end=start+43;
  DrawRing(WheelCenter,76,216,start,end,24,i==hot?rgb(50,63,54):active?rgb(30,53,45):rgb(19,32,35));
  DrawRingLines(WheelCenter,76,216,start,end,24,color);
  float angle=(-90+i*45)*DEG2RAD;Vector2 p{WheelCenter.x+std::cos(angle)*147,WheelCenter.y+std::sin(angle)*147};
  if(ref.kind!=Kind::None){icon(item(ref.kind,ref.tier),p.x,p.y-8,.82f,available?1.f:.28f);center(available?itemShortName(ref.kind):"BRAK",p.x,p.y+23,14,available?rarity(item(ref.kind,ref.tier)):Muted);}
  else{line(p.x-6,p.y-8,p.x+6,p.y-8,Muted);line(p.x,p.y-14,p.x,p.y-2,Muted);center("dodaj",p.x,p.y+16,14,Muted);}
 }
 DrawCircleV(WheelCenter,71,Panel);DrawCircleLines(int(WheelCenter.x),int(WheelCenter.y),71,Gold);
 icon(game.equipment[0],WheelCenter.x,WheelCenter.y-18,.85f);center("W RĘKU",WheelCenter.x,WheelCenter.y+17,14,Gold);
 center("LPM: wybierz broń · PPM: usuń skrót",WheelCenter.x,674,17,Gold);
 center("Q / ESC: zamknij · Gra jest wstrzymana",WheelCenter.x,708,15,Muted);
 center(game.message,WheelCenter.x,743,14,Text);
}
void drawBowHud(){
 if(archetype(game.equipment[0].kind)!=Kind::Bow||!gameplayInputAllowed())return;
 txt(TextFormat("STRZAŁY  %d",game.ammoCount()),207,119,16,game.ammoCount()>0?Gold:rgb(236,132,105));
 if(game.bowDrawing){bar(335,124,170,8,game.bowCharge(),1,Gold);txt(TextFormat("%.0f%%  ·  ZASIĘG %.1f",game.bowCharge()*100,Game::bowRange(game.bowCharge())),520,119,14,Gold);}
 if(!worldPointerAllowed(mouse)||dragging.place!=Place::None||game.dead())return;
 float charge=game.bowDrawing?game.bowCharge():0;V direction=game.bowDrawing?game.bowAim:normalized(combatAimFromPointer());
 Color color=game.ammoCount()>0?(game.piercingArrowArmed?rgb(126,215,232):Gold):rgb(236,132,105);clip(0,116,WorldW,678);
 if(game.bowDrawing){float range=Game::bowRange(charge);int dots=std::max(2,int(range*3));
  for(int i=1;i<=dots;i++){Vector2 p=screen(game.player+direction*(range*i/dots));DrawCircleV({p.x,p.y-35},i==dots?4:1.6f,Fade(color,i==dots?.9f:.5f));}
  Vector2 end=screen(game.player+direction*range);DrawCircleLines(int(end.x),int(end.y-35),8,color);
 }
 float radius=14-5*charge;DrawCircleLines(int(mouse.x),int(mouse.y),radius,Fade(color,.65f));
 for(float sign:{-1.f,1.f}){line(mouse.x+sign*(radius+4),mouse.y,mouse.x+sign*(radius+10),mouse.y,color,1.6f);line(mouse.x,mouse.y+sign*(radius+4),mouse.x,mouse.y+sign*(radius+10),color,1.6f);}
 DrawCircleV(mouse,1.6f,Text);if(game.bowDrawing)DrawRing(mouse,radius+2,radius+4,-90,-90+360*charge,48,color);EndScissorMode();
}
