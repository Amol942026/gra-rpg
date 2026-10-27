#pragma once
namespace guardian_art {
inline Texture2D stone{},fragments{};
inline void load(){stone=LoadTexture("assets/sprites/guardian-stone.png");fragments=LoadTexture("assets/sprites/guardian-fragments.png");if(stone.id)SetTextureFilter(stone,TEXTURE_FILTER_BILINEAR);if(fragments.id)SetTextureFilter(fragments,TEXTURE_FILTER_BILINEAR);}
inline void unload(){if(stone.id)UnloadTexture(stone);if(fragments.id)UnloadTexture(fragments);stone={};fragments={};}
inline void rock(Vector2 p,float angle=0,float size=BoulderSpriteSize,Color tint=WHITE,float groundY=1.e9f){
 if(!stone.id)return;
 if(p.y+size*.5f>groundY){
  float h=std::clamp(groundY-p.y+size*.5f,0.f,size);if(h<=0)return;
  DrawTexturePro(stone,{0,0,float(stone.width),stone.height*h/size},{p.x-size*.5f,p.y-size*.5f,size,h},{0,0},0,tint);return;
 }
 DrawTexturePro(stone,{0,0,float(stone.width),float(stone.height)},{p.x,p.y,size,size},{size*.5f,size*.5f},angle,tint);
}
inline void held(const Enemy& e){
 if(e.type!=3||e.boulderWindup<=0)return;
 auto socket=Game::boulderHand(e,e.boulderProgress());Vector2 p=screen(socket.ground);float floor=p.y;p.y-=socket.height;rock(p,0,BoulderSpriteSize,WHITE,floor);
}
inline void projectile(const Boulder& b){Vector2 p=screen(b.pos);p.y-=b.height;rock(p,b.age*35);}
inline Vector2 fragmentGround(const BoulderImpact& impact,int piece){auto q=guardian_break::sample(piece,impact.progress());Vector2 p=screen(impact.pos);return {p.x+q.x,p.y+q.y};}
inline void fragment(const BoulderImpact& impact,int piece){
 if(!fragments.id)return;auto q=guardian_break::sample(piece,impact.progress());if(q.alpha<=0)return;
 auto pivot=guardian_break::pivots[piece];Vector2 p=fragmentGround(impact,piece);p.y-=q.height;
 DrawTexturePro(fragments,{float(piece%3*96),float(piece/3*96),96,96},{p.x,p.y,BoulderSpriteSize,BoulderSpriteSize},{pivot.x*BoulderSpriteSize/96,pivot.y*BoulderSpriteSize/96},q.angle,Fade(WHITE,q.alpha));
}
inline void uproot(const Enemy& e){
 // Stable pickup footprint; it does not slide along with the lifted stone.
 auto socket=Game::guardianSocket(e,0,36,0);Vector2 p=screen(socket.ground);
 float seconds=e.boulderWindup>0?BoulderWindup-e.boulderWindup:BoulderWindup+BoulderRecovery-e.boulderRecovery;
 seconds*=168.f/guardian_motion::Frames; // Ground fracture follows the retimed lift.
 float opening=std::clamp((seconds-.35f)/.4f,0.f,1.f),opacity=e.boulderWindup>0?1.f:std::clamp(e.boulderRecovery/BoulderRecovery,0.f,1.f);
 ellipse(p.x,p.y,26,12,Fade(rgb(82,72,44),.65f*opacity));
 ellipse(p.x,p.y+1,23*opening,9*opening,Fade(rgb(34,39,28),.85f*opacity));
 for(int i=0;i<7;i++){
  float a=i*2*PI/7,dx=std::cos(a),dy=std::sin(a)*.5f;
  line(p.x+dx*20,p.y+dy*20,p.x+dx*(27+5*opening),p.y+dy*(27+5*opening),Fade(rgb(62,57,38),.8f*opacity),1.5f);
  float burst=std::clamp((seconds-.55f)/.65f,0.f,1.f);
  if(burst>0&&burst<1){Vector2 at{p.x+dx*(16+burst*22),p.y+dy*(16+burst*22)-std::sin(burst*PI)*(11+i%3*3)};rock(at,i*39+burst*60,6+i%3,Fade(rgb(173,153,111),1-burst));}
 }
}
inline void target(V position,float progress){
 Vector2 p=screen(position);float w=BoulderImpactRadius*56.568543f,h=w*.5f;
 Color amber=rgb(237,150,83);ellipse(p.x,p.y,w,h,Fade(amber,.10f));
 ellipseOutline(p.x,p.y,w,h,Fade(amber,.65f));ellipseOutline(p.x,p.y,w*(.36f+.62f*progress),h*(.36f+.62f*progress),Fade(Gold,.8f));
 line(p.x-5,p.y,p.x+5,p.y,amber,1);line(p.x,p.y-3,p.x,p.y+3,amber,1);
}
inline void ground(){
 if(game.location!=WorldLocation::Forest)return;
 for(const auto& e:game.enemies)if(e.alive&&e.type==3){if(e.boulderWindup>0||e.boulderRecovery>0)uproot(e);if(e.boulderWindup>0)target(e.boulderTarget,1-e.boulderWindup/BoulderWindup);}
 for(const auto& b:game.boulders)target(b.target,1);
 for(const auto& b:game.boulderImpacts){
  Vector2 p=screen(b.pos);float age=b.progress(),puff=std::clamp(age/.56f,0.f,1.f);
  float opacity=(1-puff)*.24f;ellipse(p.x,p.y+2,12+puff*43,6+puff*21,Fade(rgb(153,145,110),opacity));
  ellipseOutline(p.x,p.y,12+puff*49,6+puff*24,Fade(rgb(192,175,127),(1-puff)*.65f));
  for(int i=0;i<8;i++){float a=i*2*PI/8;Vector2 at{p.x+std::cos(a)*(5+puff*42),p.y+std::sin(a)*(3+puff*20)-std::sin(puff*PI)*14};rock(at,i*39+puff*90,4+(i%3),Fade(WHITE,1-puff));}
 }
}
}
