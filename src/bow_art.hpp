#pragma once
namespace arrow_art {
// Coordinates relative to the simulated projectile, with X across the screen,
// Y along ground depth and Z up. The physics position and collisions stay in world tiles.
inline hero_quiver::CarriedArrow flight(const Arrow& arrow){
 using namespace hero_motion;
 constexpr float length=hero_quiver::ArrowLength;
 Vec along{arrow.velocity.x-arrow.velocity.y,(arrow.velocity.x+arrow.velocity.y)*.5f/ActorGroundSquash,0};
 along=along*(1/std::max(.001f,hero_motion::length(along)));
 const Vec forwardAcross{along.y,-along.x,0};
 const hero_quiver::CarriedArrow cruising{Vec{0,0,35}-along*length,along,forwardAcross,length};
 const auto& launch=arrow.launch;
 if(launch.charge<0)return cruising;
 const float range=Game::bowRange(launch.charge)*(arrow.power?powerAttack(PowerStyle::Bow).reach:1.f);
 const float age=(range-arrow.remaining)/std::max(.001f,ember::length(arrow.velocity));
 constexpr float departure=.14f;
 if(age>=departure)return cruising;
 auto pose=hero_motion::evaluate(launch.phase,launch.moving,0,launch.idleTime);
 const auto profile=weapon_profiles::bow(launch.bow);
 const auto shot=hero_bow::sampleShot(launch.charge,-1,1.f);
 hero_bow::apply(pose,shot,profile.curve-profile.recurve*.29f);
 const auto arrowInHand=hero_quiver::carried(pose,shot.arrowAxis,0,1.f);
 const auto facing=actorFacing(launch.facing);
 auto oriented=[&](Vec p){return Vec{p.x*facing.y+p.y*facing.x,-p.x*facing.x+p.y*facing.y,p.z};};
 const Vec axis=oriented(arrowInHand.axis),across=oriented(arrowInHand.across);
 const float t=ease(age/departure);
 const Quat turn=mix({},fromTo(axis,along),t);
 const Vec rotatedAcross=rotate(turn,across);
 return {oriented(arrowInHand.nock)*(1-t)+cruising.nock*t,rotate(turn,axis),
         rotatedAcross*(1-t)+forwardAcross*t,length};
}

inline void draw(const Arrow& shot,bool shadow){
 if(ember::length(shot.velocity)<.001f)return;
 const auto p=screen(shot.pos);
 const auto arrow=flight(shot);
 actor_art::Rig rig(p.x,p.y,1.f,Direction::S,shadow);
 rig.setSmoothSurfaces();
 auto local=[](hero_motion::Vec v){return actor_art::P3{v.x,v.y,v.z};};
 const auto shaft=local(arrow.axis),nock=local(arrow.nock);
 actor_art::hero_weapon_art::arrow(rig,nock,shaft,local(arrow.across),rgb(174,187,184),arrow.exposed);
 if(shadow){rig.drawShadow();return;}
 const auto tip=rig.project(nock+shaft*hero_quiver::ArrowLength),tail=rig.project(nock);
 const float pixels=std::max(.001f,std::hypot(tip.x-tail.x,tip.y-tail.y));
 const Vector2 direction{(tip.x-tail.x)/pixels,(tip.y-tail.y)/pixels};
 if(shot.power)power_fx::arrow(tip,direction);
 if(shot.piercing){
  for(int i=1;i<=5;i++)line(tail.x-direction.x*i*5,tail.y-direction.y*i*5,tip.x-direction.x*i*5,tip.y-direction.y*i*5,Fade(rgb(107,206,239),.38f-i*.055f),2);
  DrawCircleLines(int(tip.x),int(tip.y),6,rgb(134,232,251));
 }
 rig.draw();
}
}
void drawArrow(const Arrow& arrow){arrow_art::draw(arrow,false);}
