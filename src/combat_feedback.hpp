#pragma once
// Shared by forest and tower. Critical numbers use prewarmed title atlases.
void outlinedCombatText(const std::string& text,float x,float y,float size,Color color,bool title=false){
 float left=x-textWidth(text,size,title)/2;
 // Measure once and use four edge passes; glyph geometry stays GPU-batched.
 for(auto offset:std::array<Vector2,4>{{{0,-2},{-2,0},{2,0},{0,2}}})txt(text,left+offset.x,y+offset.y,size,Fade(BLACK,color.a/255.f*.9f),title);
 txt(text,left,y,size,color,title);
}
void drawCombatEffect(const Effect& effect,float lift=0){
 Vector2 p=screen(effect.pos);
 if(effect.damageNumber){
  float duration=effect.critical?1.25f:.95f,age=std::clamp(1-effect.life/duration,0.f,1.f),alpha=std::min(1.f,effect.life/.3f);
  float size=effect.critical?39:24,w=std::max(textWidth(effect.text,size,effect.critical),effect.critical?textWidth("KRYTYK!",20,true):0.f);
  float x=std::clamp(p.x,w/2+8,float(WorldW)-w/2-8),y=std::max(112.f,p.y-effect.height-age*34-lift);
  if(effect.critical){
   rect(x-w/2-7,y-23,w+14,69,Fade(rgb(25,12,7),alpha*.62f));
   outlinedCombatText("KRYTYK!",x,y-21,20,Fade(rgb(255,175,69),alpha),true);
  }
  outlinedCombatText(effect.text,x,y,size,Fade(effect.critical?rgb(255,214,106):rgb(250,249,232),alpha),effect.critical);return;
 }
 Color c=effect.color==1?rgb(136,239,158):effect.color==2?Blue:effect.color==4?rgb(240,130,113):Gold;
 outlinedCombatText(effect.text,p.x,p.y-effect.height-(1-effect.life)*30,20,Fade(c,std::min(1.f,effect.life)));
}
void drawCombatEffects(){
 std::vector<Rectangle> occupied;
 for(const auto& effect:game.effects){
  float lift=0;
  if(effect.damageNumber){
   Vector2 p=screen(effect.pos);float duration=effect.critical?1.25f:.95f,age=std::clamp(1-effect.life/duration,0.f,1.f);
   float w=std::max(textWidth(effect.text,effect.critical?39:24,effect.critical),effect.critical?textWidth("KRYTYK!",20,true):0.f)+16;
   float x=std::clamp(p.x,w/2+1,float(WorldW)-w/2-1),base=std::max(112.f,p.y-effect.height-age*34);
   Rectangle box{x-w/2,base-(effect.critical?23:2),w,effect.critical?69.f:32.f};
   // Keep separate AoE/piercing impact readouts legible above nearby enemies.
   for(size_t attempt=0;attempt<occupied.size();attempt++){
    float next=box.y;for(const auto& other:occupied)if(CheckCollisionRecs(box,other))next=std::min(next,other.y-box.height-6);
    if(next==box.y||next<90)break;box.y=next;
   }
   lift=base-box.y-(effect.critical?23:2);occupied.push_back(box);
   if(lift>4)line(p.x,base+20,x,box.y+box.height,Fade(effect.critical?Gold:Text,std::min(.35f,effect.life*.35f)));
  }
  drawCombatEffect(effect,lift);
 }
}
void drawHealingAura(){
 if(game.healingPulse<=0)return;
 Vector2 p=screen(game.player);float age=1-game.healingPulse/HealingDuration,alpha=std::sin(age*PI);Color green=rgb(123,246,166);
 for(int i=0;i<3;i++)ellipseOutline(p.x,p.y-6-i*18-age*28,19+age*24-i*3,9+age*11-i,Fade(green,alpha*(.9f-i*.2f)));
 for(int i=0;i<10;i++){float a=i*2*PI/10+age*2,x=p.x+std::cos(a)*(24+age*13),y=p.y-25-std::sin(a)*12-age*(35+i%3*11);
  rect(x-2,y-6,4,12,Fade(green,alpha));rect(x-6,y-2,12,4,Fade(green,alpha));}
 ellipse(p.x,p.y,30+age*18,14+age*8,Fade(green,alpha*.13f));
}
