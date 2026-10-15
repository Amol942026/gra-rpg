#pragma once
namespace sand_prison {
std::array<RenderTexture2D,2> mound{};
void make(){
 for(int front=0;front<2;front++){
  auto& rt=mound[front];rt=LoadRenderTexture(256,192);BeginTextureMode(rt);ClearBackground(BLANK);BeginMode2D({{0,0},{0,0},0,2});
  desert_art::beginTransparentLayer();rlSetTexture(rlGetTextureIdDefault());
  auto surface=[](float radius,float angle){
   float ridge=std::pow(std::max(0.f,1-radius),.72f)*39;
   float crease=std::sin(angle*9+radius*5)*1.3f*std::sin(radius*PI);
   Vector2 p{64+std::cos(angle)*radius*53,68+std::sin(angle)*radius*22-ridge-crease};
   float light=1-.17f*std::cos(angle)-.12f*std::sin(angle)+.1f*std::sin(radius*13+angle*3);
   return std::pair<Vector2,Color>{p,shade(rgb(217,173,110),light)};
  };
  for(int radial=0;radial<24;radial++)for(int step=0;step<96;step++){
   float a=(front?0:PI)+step*PI/96,b=(front?0:PI)+(step+1)*PI/96;
   float r=.32f+radial*.68f/24,s=.32f+(radial+1)*.68f/24;
   auto p=surface(r,a),q=surface(r,b),u=surface(s,b),v=surface(s,a);
   desert_art::roadTriangle(p.first,q.first,u.first,p.second,q.second,u.second);
   desert_art::roadTriangle(p.first,u.first,v.first,p.second,u.second,v.second);
  }rlSetTexture(0);
  // Fine wind ripples and individual sunlit grains follow the curved mound.
  for(int i=0;i<320;i++){
   unsigned h=Game::hash(i,front+834);float radius=.36f+(h%610)*.001f,angle=(front?0:PI)+((h>>12)%1000)*PI*.001f;
   auto p=surface(radius,angle).first;Color color=i%3?rgb(245,205,141,145):rgb(149,105,66,100);
   line(p.x,p.y,p.x+.8f+(h%3)*.3f,p.y-.25f,color,.5f);
  }
  native_world::endBlendMode();EndMode2D();EndTextureMode();SetTextureFilter(rt.texture,TEXTURE_FILTER_BILINEAR);
 }
}
void drawMound(Vector2 position,bool front){
 if(!game.sandTrapped())return;
 float age=SandPrisonDuration-game.sandPrisonRemaining;
 float emergence=std::min(1.f,age/.18f),fade=std::min(1.f,game.sandPrisonRemaining/.28f);
 auto& rt=mound[front?1:0];float height=.55f+.45f*emergence;
 DrawTexturePro(rt.texture,{0,0,256,-192},{position.x-64,position.y-68*height,128,96*height},{0,0},0,Fade(WHITE,fade*(.35f+.65f*emergence)));
 if(!front){
  float t=game.time;desert_art::beginTransparentLayer();
  for(int i=0;i<30;i++){float a=i*2.39996f+t*3,r=27+std::sin(t*4+i)*10;float z=std::fmod(t*23+i*3,38.f);
   float x=position.x+std::cos(a)*r,y=position.y+std::sin(a)*r*.38f-z;float alpha=fade*(1-z/42)*.65f;
   line(x,y,x+2.3f,y-.8f,Fade(rgb(251,215,154),alpha),1);
  }native_world::endBlendMode();
 }
}
void drawCaster(const Enemy& e,float x,float y){
 if(e.sandCastPulse<=0||!e.alive)return;
 float phase=1-e.sandCastPulse/SandCastDuration;
 desert_art::beginTransparentLayer();
 for(int ring=0;ring<3;ring++)ellipseOutline(x,y,35+phase*44+ring*8,16+phase*20+ring*4,Fade(rgb(255,215,145),(1-phase)*(.7f-ring*.16f)));
 for(int i=0;i<22;i++){float angle=i*2.39996f+phase*7,r=26+phase*43;float xx=x+std::cos(angle)*r,yy=y+std::sin(angle)*r*.45f-phase*45;
  line(xx,yy,xx+2,yy-3,Fade(Gold,1-phase),1.2f);
 }native_world::endBlendMode();
}
void drawStatus(){
 if(!game.sandTrapped())return;
 panel({355,85,390,57},rgb(41,30,24,238));
 txt("PIASKOWE WIĘZIENIE",369,93,16,rgb(248,210,148));
 txt(TextFormat("%.1f s",game.sandPrisonRemaining),680,92,20,rgb(255,228,181));
 txt("Ruch i ataki zablokowane",369,115,12,Text);
 rect(369,134,362*game.sandPrisonRemaining/SandPrisonDuration,2,Gold);
}
void unload(){for(auto& rt:mound){if(rt.id)UnloadRenderTexture(rt);rt={};}}
}
