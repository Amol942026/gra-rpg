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
   outlinedCombatText("KRYTYK!",x,y-21,20,Fade(rgb(255,175,69),alpha),true);
  }
  outlinedCombatText(effect.text,x,y,size,Fade(effect.critical?rgb(255,214,106):rgb(250,249,232),alpha),effect.critical);return;
 }
 Color c=effect.color==1?rgb(136,239,158):effect.color==2?Blue:effect.color==4?rgb(240,130,113):Gold;
 outlinedCombatText(effect.text,p.x,p.y-effect.height-(1-effect.life)*30,20,Fade(c,std::min(1.f,effect.life)));
}
void drawImpactDebris(const CombatImpact& impact,Vector2 origin,float heading,float age,float scale){
 const auto material=impact.material;
 Color body{},light{};
 switch(material){
 case ImpactMaterial::Stone:body=rgb(96,111,107);light=rgb(189,207,193);break;
 case ImpactMaterial::Bark:body=rgb(116,74,39);light=rgb(221,170,97);break;
 case ImpactMaterial::Slime:body=rgb(77,160,68);light=rgb(179,235,115);break;
 case ImpactMaterial::Bone:body=rgb(143,123,95);light=rgb(239,223,175);break;
 case ImpactMaterial::Sand:body=rgb(164,114,59);light=rgb(235,192,117);break;
 case ImpactMaterial::Shell:body=rgb(100,81,55);light=rgb(222,164,82);break;
 case ImpactMaterial::Ember:body=rgb(161,48,28);light=rgb(255,175,70);break;
 case ImpactMaterial::Fur:body=rgb(97,81,67);light=rgb(176,159,125);break;
 default:return;
 }
 const float alpha=1.f-age*age;
 for(int i=0;i<9;i++){
  const float angle=heading+(i-4)*.48f;
  const float travel=(2+age*(19+i%4*6))*scale;
  const float arc=age*((material==ImpactMaterial::Slime?11.f:19.f)+i%3*5-age*31);
  const Vector2 p{origin.x+std::cos(angle)*travel,origin.y+std::sin(angle)*travel*.64f-arc};
  const float size=(1.8f+(i%3)*.7f)*scale*(1-age*.32f);
  if(material==ImpactMaterial::Slime||material==ImpactMaterial::Sand||material==ImpactMaterial::Fur){
   const bool wet=material==ImpactMaterial::Slime;
   const float ry=size*(wet?1.12f:.55f);
   ellipse(p.x,p.y,size,ry,Fade(body,alpha*(wet?.93f:.67f)));
   ellipse(p.x-size*.29f,p.y-ry*.34f,size*.48f,ry*.35f,Fade(light,alpha*.85f));
  }else{
   const bool splinter=material==ImpactMaterial::Bark&&i%3!=0;
   const float spin=angle+age*(i%2?8:-7);
   const Vector2 u{std::cos(spin)*size,std::sin(spin)*size},v{-u.y,u.x};
   const float reach=splinter?2.15f:1.15f,thickness=splinter?.42f:.83f;
   const Vector2 a{p.x-u.x*reach,p.y-u.y*reach},b{p.x+v.x*thickness,p.y+v.y*thickness};
   const Vector2 c{p.x+u.x*reach,p.y+u.y*reach},d{p.x-v.x,p.y-v.y};
   poly({a,b,c,d},Fade(body,alpha));
   poly({a,d,c},Fade(light,alpha*.9f));
  }
 }
}
void drawArmorImpact(Vector2 p,float heading,float age,DamageType type){
 using hero_impact::data::samples;
 constexpr int last=int(std::size(samples))-1;
 const float frame=age*last;
 const int before=std::min(int(frame),last-1);
 const auto a=samples[before],b=samples[before+1];
 const float mix=frame-before;
 const float spread=a.spread+(b.spread-a.spread)*mix;
 const float glow=a.glow+(b.glow-a.glow)*mix;
 const float fade=a.fade+(b.fade-a.fade)*mix;
 const bool magic=type==DamageType::Magic;
 const Color body=magic?rgb(109,187,238):rgb(228,147,77);
 const Color light=magic?rgb(202,238,253):rgb(255,221,156);
 if(glow>0)DrawCircleV(p,2.4f,Fade(light,glow*.66f));
 // Irregular cone with separate droplets and shorter, slower fragments near contact.
 struct Droplet {float angle,start,travel,tail,radius;};
 constexpr Droplet droplets[]={
  {-1.22f,3,21,4.5f,1.55f},{-.83f,8,32,6.5f,2.1f},{-.64f,3,16,3.8f,1.5f},
  {-.47f,7,39,8.1f,2.45f},{-.17f,5,30,5.5f,1.9f},{.02f,10,41,8.6f,2.5f},
  {.23f,6,22,4.9f,2.05f},{.39f,9,36,7.1f,2.3f},{.68f,4,25,5.5f,1.85f},
  {.97f,6,19,4.4f,1.7f},{1.67f,4,12,3.4f,1.42f}
 };
 for(const auto& drop:droplets){
  const float angle=heading+drop.angle;
  const Vector2 ray{std::cos(angle),std::sin(angle)};
  const float travel=drop.start+drop.travel*spread;
  const Vector2 head{p.x+ray.x*travel,p.y+ray.y*travel-4*spread+13*age*age};
  const float radius=drop.radius*(1-age*.32f);
  const float tail=drop.tail*(.68f+.32f*glow)*(1-age*.46f);
  const Color color=Fade(body,fade*.95f);
  poly({{head.x-ray.x*tail,head.y-ray.y*tail},
        {head.x-ray.y*radius,head.y+ray.x*radius},
        {head.x+ray.y*radius,head.y-ray.x*radius}},color);
  DrawCircleV(head,radius,color);
  if(drop.radius>=1.85f){
   const Vector2 glint{head.x+ray.x*radius*.3f,head.y+ray.y*radius*.3f};
   DrawCircleV(glint,radius*.48f,Fade(light,fade*(.75f+.25f*glow)));
  }
 }
}
void drawImpactBursts(){
 for(const auto& impact:game.combatImpacts){
  Vector2 p=screen(impact.pos);p.y-=impact.height;
  const Vector2 tip=screen(impact.pos+impact.direction);
  const float heading=std::atan2(tip.y-p.y-impact.height,tip.x-p.x);
  if(impact.material==ImpactMaterial::Armor){
   const float age=std::clamp(1-impact.life/hero_impact::data::Duration,0.f,1.f);
   drawArmorImpact(p,heading,age,impact.type);
   continue;
  }
  const float age=std::clamp(1-impact.life/EnemyImpactLife,0.f,1.f);
  const float fade=(1-age)*(1-age),scale=impact.critical?1.24f:1.f;
  const Color edge=impact.type==DamageType::Magic?rgb(156,213,255):rgb(255,172,75);
  const Color core=impact.type==DamageType::Magic?rgb(220,248,255):rgb(255,241,199);
  if(impact.material!=ImpactMaterial::Spark)drawImpactDebris(impact,p,heading,age,scale);
  for(int i=0;i<(impact.material==ImpactMaterial::Spark?7:0);i++){
   const float angle=heading+(i-3)*.59f;
   const Vector2 ray{std::cos(angle),std::sin(angle)};
   const float distance=(5+age*(18+(i%3)*8))*scale,length=(8+(i%3)*3)*scale;
   const Vector2 a{p.x+ray.x*distance,p.y+ray.y*distance};
   const Vector2 b{a.x+ray.x*length,a.y+ray.y*length};
   const float width=(i%2?1.3f:2.5f)*scale*(1-age*.6f);
   poly({a,{a.x-ray.y*width,a.y+ray.x*width},b,{a.x+ray.y*width,a.y-ray.x*width}},
        Fade(edge,fade));
   if(age<.43f)DrawLineEx(a,b,1.f,Fade(core,1-age/.43f));
  }
  if(age<.30f){
   const float light=1-age/.30f;
   ellipse(p.x,p.y,6.5f*scale,4.8f*scale,Fade(edge,light*.5f));
   ellipse(p.x,p.y,3.7f*scale,2.7f*scale,Fade(core,light));
  }
 }
}
void drawCombatEffects(){
 std::vector<Rectangle> occupied;
 if(hovered.kind==HoverKind::Enemy)if(const Enemy* target=game.findEnemy(hovered.index)){
  const Vector2 p=screen(target->pos);
  const Rectangle bar=enemyHealthBar(*target,p);
  const float width=std::max(bar.width+2,textWidth(enemyInfo(target->type).name,15)+12);
  occupied.push_back({p.x-width/2,bar.y-22,width,29});
 }
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
