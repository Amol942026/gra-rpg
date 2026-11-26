#version 330
in vec2 fragTexCoord;
out vec4 finalColor;
uniform sampler2D texture0,peat;
uniform vec2 origin;
uniform float clockTime,mapSize;
float seed(vec2 tile){return fract(sin(dot(tile,vec2(37.13,91.7)))*4375.73);}
void main(){
 vec2 s=fragTexCoord*vec2(1050.,900.)-origin;
 vec2 p=vec2(s.x/80.+s.y/40.,s.y/40.-s.x/80.);
 vec2 m=texture(texture0,p/mapSize).rg;
 vec2 paint=vec2(s.x*.0018,s.y*.0029);
 float grit=dot(texture(peat,paint).rgb,vec3(.25,.58,.17));
 float grain=dot(texture(peat,paint*3.37+.27).rgb,vec3(.27,.55,.18));
 float mottling=clamp((grit-.23)*2.9+(grain-.26)*.7+.5,0.,1.);
 vec3 mud=mix(vec3(.15,.17,.11),vec3(.35,.36,.225),mottling);
 vec3 trail=mix(vec3(.24,.207,.146),vec3(.46,.40,.27),mottling);
 float road=smoothstep(.36,.74,m.y+(grit-.28)*.67);
 vec3 land=mix(mud,trail,road);
 float ripple=sin(s.y*.115+2.7*sin(s.x*.022+s.y*.043)+clockTime*1.68+grit*8.);
 float glint=smoothstep(.70,1.,ripple)*smoothstep(.24,.48,grain);
 vec3 water=mix(vec3(.071,.145,.14),vec3(.18,.26,.205),mottling)+vec3(.085,.096,.08)*glint;
 // Small rafts of duckweed only grow where there is shallow, sheltered water.
 vec2 plantCell=floor(p*.84);
 float cluster=seed(plantCell);
 if(m.x>.08 && m.x<.53 && cluster>.54){
  vec2 uv=fract(p*.84)-.5;
  for(int i=0;i<4;i++){
   float id=float(i);vec2 offset=vec2(seed(plantCell+id+2.),seed(plantCell.yx+id+4.))*.58-.29;
   vec2 leaf=(uv-offset)*vec2(1.,1.4);
   float radius=.032+.047*seed(plantCell+id+9.);
   float pad=1.-smoothstep(radius*.74,radius,length(leaf));
   water=mix(water,mix(vec3(.21,.27,.14),vec3(.36,.39,.19),cluster),pad*.88);
  }
 }
 float boundary=min(min(p.x,p.y),mapSize-1.-max(p.x,p.y));
 float bank=smoothstep(.36,.57,m.x+(grit-.30)*.24);
 bank*=smoothstep(.1,1.3,boundary+.26*sin(p.x+p.y*.41));
 vec3 shore=mix(water,land,bank);
 shore+=vec3(.060,.057,.023)*(1.-abs(2.*bank-1.));
 float mist=.017*(.5+.5*sin((p.x+p.y)*.51+sin(p.y*.25)+clockTime*.18));
 finalColor=vec4(shore+vec3(.65,.92,.80)*mist,1.);
}