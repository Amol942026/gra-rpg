#pragma once

// Included inside caldera_puppets. The shader samples one complete pose and
// writes one colour and one coverage value for the character at each pixel.
namespace imp_render {
inline Shader program{};
inline bool sharpen=true;
struct Locations {int axisU,axisV,tiles,paint,core,count,mode;};
inline Locations locations{};

inline bool ready(){return impCore.texture.id&&program.id&&program.id!=rlGetShaderIdDefault();}

inline void load(){
 if(program.id||!impCore.texture.id)return;
 constexpr const char* fragment=R"GLSL(#version 330
 in vec2 fragTexCoord;
 in vec4 fragColor;
 out vec4 finalColor;
 uniform sampler2D texture0, coreArt;
 uniform vec3 axisU[22],axisV[22];
 uniform vec4 tiles[22];
 uniform vec2 paint[22];
 uniform int count,mode;

 vec4 ink(int i,vec2 p){
    if(paint[i].x>.5)return textureLod(coreArt,p,0.);
    return textureLod(texture0,p,0.);
 }
 void main(){
    vec3 point=vec3(fragTexCoord,1.);
    vec2 d=abs(dFdx(fragTexCoord))+abs(dFdy(fragTexCoord));
    bool fill=mode==0||mode==3;
    vec4 pose=vec4(0.);
    for(int i=0;i<22;++i){
        if(i>=count)break;
        vec2 uv=vec2(dot(axisU[i],point),dot(axisV[i],point));
        if(any(lessThan(uv,vec2(0.)))||any(greaterThan(uv,vec2(1.))))continue;
        vec4 tile=tiles[i];
        vec2 p=tile.xy+uv*tile.zw;
        vec4 base=ink(i,p);
        if(mode==0&&base.a>.90){
            // Area sampling tames single-texel glints on the scaled limbs.
            // Keep the original coverage so the body, shadow and picking agree.
            vec2 radius=vec2(dot(abs(axisU[i].xy),d),dot(abs(axisV[i].xy),d))*.54*tile.zw;
            vec2 br=tile.xy+tile.zw;
            vec4 l=ink(i,clamp(p-vec2(radius.x,0),tile.xy,br));
            vec4 r=ink(i,clamp(p+vec2(radius.x,0),tile.xy,br));
            vec4 t=ink(i,clamp(p-vec2(0,radius.y),tile.xy,br));
            vec4 b=ink(i,clamp(p+vec2(0,radius.y),tile.xy,br));
            float support=l.a+r.a+t.a+b.a;
            vec3 average=(l.rgb*l.a+r.rgb*r.a+t.rgb*t.a+b.rgb*b.a)/max(support,.01);
            float edge=dot(base.rgb-average,vec3(.2126,.7152,.0722));
            float interior=smoothstep(3.25,3.96,support);
            base.rgb=clamp(mix(base.rgb,average,.68*interior)+interior*clamp(.12*edge,-.014,.014),0.,1.);
        }
        base.rgb=fill?base.rgb*base.a*paint[i].y:vec3(0.);
        pose=base+pose*(1.-base.a);
    }
    if(mode==2){
        if(pose.a<.62)discard;
        finalColor=vec4(fragColor.a);
        return;
    }
    if(pose.a<=.0039)discard;
    finalColor=vec4(fill?pose.rgb/pose.a:fragColor.rgb,pose.a*fragColor.a);
 }
 )GLSL";
 program=LoadShaderFromMemory(nullptr,fragment);
 if(!ready())return;
 const auto at=[](const char* name){return GetShaderLocation(program,name);};
 locations={at("axisU[0]"),at("axisV[0]"),at("tiles[0]"),at("paint[0]"),at("coreArt"),at("count"),at("mode")};
}

inline void unload(){
 if(program.id&&program.id!=rlGetShaderIdDefault())UnloadShader(program);
 program={};locations={};
}

inline void draw(const Pose& pose,Vector2 anchor,float size,Color outline,bool shadow,bool flash,unsigned char alpha,float opacity){
 std::array<Vector3,22> u{},v{};
 std::array<Vector4,22> uv{};
 std::array<Vector2,22> shades{};
 Vector2 lo{1.e6f,1.e6f},hi{-1.e6f,-1.e6f};
 const auto& body=artwork[Imp];
 for(size_t i=0;i<pose.count;++i){
  const auto& part=pose.pieces[i];
  const auto sprite=spriteFor(body,part.tile);
  const float c=std::cos(part.angle),s=std::sin(part.angle);
  const float across=1/(2*part.half.x),down=1/(2*part.half.y);
  const float sign=part.mirror?-1.f:1.f;
  u[i]={sign*c*across,sign*s*across,.5f-sign*(c*part.center.x+s*part.center.y)*across};
  v[i]={-s*down,c*down,.5f-(-s*part.center.x+c*part.center.y)*down};
  const auto rect=sprite.source;
  const float w=float(sprite.art.texture.width),h=float(sprite.art.texture.height);
  uv[i]={rect.x/w,rect.y/h,rect.width/w,rect.height/h};
  shades[i]={part.tile>=FirstCore?1.f:0.f,part.light};
  const float dx=std::abs(c)*part.half.x+std::abs(s)*part.half.y;
  const float dy=std::abs(s)*part.half.x+std::abs(c)*part.half.y;
  lo={std::min(lo.x,part.center.x-dx),std::min(lo.y,part.center.y-dy)};
  hi={std::max(hi.x,part.center.x+dx),std::max(hi.y,part.center.y+dy)};
 }
 const auto count=int(pose.count);
 BeginShaderMode(program);
 SetShaderValueV(program,locations.axisU,u.data(),SHADER_UNIFORM_VEC3,count);
 SetShaderValueV(program,locations.axisV,v.data(),SHADER_UNIFORM_VEC3,count);
 SetShaderValueV(program,locations.tiles,uv.data(),SHADER_UNIFORM_VEC4,count);
 SetShaderValueV(program,locations.paint,shades.data(),SHADER_UNIFORM_VEC2,count);
 SetShaderValue(program,locations.count,&count,SHADER_UNIFORM_INT);
 const auto batch=[&](int mode,Color color,Vector2 offset){
  // The mode uniform and the auxiliary texture apply to precisely this quad.
  rlDrawRenderBatchActive();
  SetShaderValue(program,locations.mode,&mode,SHADER_UNIFORM_INT);
  SetShaderValueTexture(program,locations.core,impCore.texture);
  const Vector2 points[]{{lo.x,lo.y},{lo.x,hi.y},{hi.x,hi.y},{hi.x,lo.y}};
  rlSetTexture(body.texture.id);rlBegin(RL_QUADS);
  rlColor4ub(color.r,color.g,color.b,color.a);
  for(int i=0;i<4;i++){
   const auto local=points[shadow?(4-i)%4:i];
   rlTexCoord2f(local.x,local.y);
   const auto p=shadow?Vector2{local.x-shadow_math::LightX*local.y,-shadow_math::LightY*local.y}:local;
   rlVertex2f(anchor.x+p.x*size+offset.x,anchor.y+p.y*size+offset.y);
  }
  rlEnd();rlSetTexture(0);
 };
 if(shadow)batch(2,{255,255,255,alpha},{});
 else{
  if(outline.a){
   outline.a=static_cast<unsigned char>(outline.a*opacity);
   for(const auto p:{Vector2{-1.5f,0},Vector2{1.5f,0},Vector2{0,-1.5f},Vector2{0,1.5f}})batch(1,outline,p);
  }
  batch(sharpen?0:3,{255,255,255,alpha},{});
  if(flash)batch(1,{255,255,255,static_cast<unsigned char>(alpha*.43f)},{});
 }
 EndShaderMode();
 if(shadow&&world_shadows::maskShader.id)BeginShaderMode(world_shadows::maskShader);
}
}
