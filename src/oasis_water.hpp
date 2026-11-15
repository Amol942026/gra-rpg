#pragma once
// Included by desert_art. The same radius defines the shallows and collision.
namespace oasis_water {
inline Shader material{};
inline int clockLocation=-1;
inline bool ready(){return IsShaderValid(material)&&material.id!=rlGetShaderIdDefault();}
inline void init(){
 material=LoadShaderFromMemory(nullptr,R"GLSL(#version 330
in vec2 fragTexCoord;
out vec4 finalColor;
uniform float age;
float bank(vec2 p){
 float a=atan(p.y,p.x);
 return length(p)-.023*sin(a*5.+.9)-.013*sin(a*9.-.4)-.009*cos(a*13.);
}
void main(){
 vec2 p=fragTexCoord;
 float r=bank(p);
 float edge=1.-smoothstep(.97,1.01,r);
 if(edge<.002)discard;
 float shallows=smoothstep(.45,1.,r);
 float wave=p.x*17.+p.y*24.+sin(p.y*9.+p.x*3.)*.9-age*1.7;
 float crossing=p.y*37.-p.x*13.+sin(p.x*11.+age*.32)*1.6-age*.84;
 float grain=.5+.5*sin(p.x*72.+p.y*63.+sin(p.x*42.-p.y*51.)*2.);
 vec3 sand=mix(vec3(.59,.44,.255),vec3(.76,.62,.39),grain);
 vec3 water=mix(vec3(.082,.235,.244),vec3(.26,.43,.35),shallows);
 water=mix(water,sand,shallows*shallows*.63);
 float wavePeak=pow(.5+.5*cos(wave+.58*cos(crossing)),11.);
 float glint=wavePeak*(.44+.56*sin(crossing)*sin(crossing))*.075;
 float caustic=pow(.5+.5*cos(wave+crossing),8.)*.043*shallows;
 float shoreline=smoothstep(.895,.967,r)*(1.-smoothstep(.967,1.,r));
 float wash=shoreline*(.64+.36*sin(wave-age*.33));
 vec3 color=water+vec3(.78,.96,.85)*(glint+caustic)+vec3(.07,.10,.069)*wash;
 finalColor=vec4(color,edge);
}
)GLSL");
 if(ready())clockLocation=GetShaderLocation(material,"age");
}
inline void draw(){
 if(!ready())return;
 const Vector2 p=screen(DesertOasis);
 constexpr float width=142,height=71;
 if(p.x+width<0||p.x-width>WorldW||p.y+height<0||p.y-height>H)return;
 BeginShaderMode(material);
 SetShaderValue(material,clockLocation,&game.time,SHADER_UNIFORM_FLOAT);
 rlSetTexture(rlGetTextureIdDefault());rlBegin(RL_QUADS);rlColor4ub(255,255,255,255);
 for(Vector2 corner:{Vector2{-1.06f,-1.06f},Vector2{-1.06f,1.06f},Vector2{1.06f,1.06f},Vector2{1.06f,-1.06f}}){
  rlTexCoord2f(corner.x,corner.y);rlVertex2f(p.x+corner.x*width,p.y+corner.y*height);
 }
 rlEnd();rlSetTexture(0);EndShaderMode();
}
inline void unload(){if(ready())UnloadShader(material);material={};clockLocation=-1;}
}
