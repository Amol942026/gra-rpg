#pragma once
// Detailed furniture shares the staircase's 40:20 perspective and fixed light.
// Physical meshes are baked once at 2x; only flame, water and magic move in play.
namespace tower_furniture {
using P=tower_stairs::P;
constexpr int ObjectCount=13,AlchemyTable=13,Window=14,Orb=15,Crystal=16,Heart=17,Herb=18,Flame=21,SpriteCount=22;
const Color Stone{151,160,163,255},Wood{129,83,51,255},Brass{193,151,81,255},Paper{232,216,176,255};
Vector2 offset(P p){return {(p.x-p.y)*40,(p.x+p.y)*20-p.z};}
struct Model: tower_stairs::Mesh {
 Model(float px,float py):Mesh{px,py,{},{},{}}{}
 void box(float a,float b,float l,float r,float low,float high,Color color,bool detail=false){
  size_t n=faces.size();Mesh::box(a,b,l,r,low,high,color,false,false);for(size_t i=n;i<faces.size();i++)faces[i].detail=detail?1:0;
 }
 void stroke(P a,P b,Color color,float width=1){
  Vector2 pa=project(a),pb=project(b);float len=std::hypot(pb.x-pa.x,pb.y-pa.y);if(len<.001f)return;
  float dx=-(pb.y-pa.y)*width*.5f/len,dy=(pb.x-pa.x)*width*.5f/len;
  faces.push_back({{{pa.x+dx,pa.y+dy},{pb.x+dx,pb.y+dy},{pb.x-dx,pb.y-dy},{pa.x-dx,pa.y-dy}},((a.x+a.y+b.x+b.y)*34.6f+(a.z+b.z)*.5f)*.5f+.03f,color,false,0});
 }
 void frustum(float u,float v,float r0,float r1,float z0,float z1,Color color,bool cap=true,int sides=16){
  for(int i=0;i<sides;i++){
   float a=i*2*PI/sides,b=(i+1)*2*PI/sides,mid=(a+b)*.5f;
   if(std::cos(mid)+std::sin(mid)>-.04f)face({{u+r0*std::cos(a),v+r0*std::sin(a),z0},{u+r0*std::cos(b),v+r0*std::sin(b),z0},{u+r1*std::cos(b),v+r1*std::sin(b),z1},{u+r1*std::cos(a),v+r1*std::sin(a),z1}},shade(color,.69f+.22f*std::cos(mid+.7f)));
   if(cap)face({{u,v,z1},{u+r1*std::cos(a),v+r1*std::sin(a),z1},{u+r1*std::cos(b),v+r1*std::sin(b),z1}},shade(color,1.1f));
  }
 }
 void ring(float u,float v,float outer,float inner,float z,Color color,int sides=28){
  for(int i=0;i<sides;i++){float a=i*2*PI/sides,b=(i+1)*2*PI/sides;
   face({{u+outer*std::cos(a),v+outer*std::sin(a),z},{u+outer*std::cos(b),v+outer*std::sin(b),z},{u+inner*std::cos(b),v+inner*std::sin(b),z},{u+inner*std::cos(a),v+inner*std::sin(a),z}},shade(color,.98f+.1f*std::cos(a+.6f)));
  }
 }
 void tube(P a,P b,float radius,Color color,Color cap=BLANK){
  float dx=(b.x-a.x)*40,dy=(b.y-a.y)*40,dz=b.z-a.z,len=std::sqrt(dx*dx+dy*dy+dz*dz);if(len<.001f)return;
  dx/=len;dy/=len;dz/=len;float flat=std::hypot(dx,dy);float nx=flat>.001f?-dy/flat:1,ny=flat>.001f?dx/flat:0;
  float bx=-dz*ny,by=dz*nx,bz=dx*ny-dy*nx;
  auto point=[&](P p,float angle){float c=std::cos(angle)*radius,s=std::sin(angle)*radius;return P{p.x+(nx*c+bx*s)/40,p.y+(ny*c+by*s)/40,p.z+bz*s};};
  for(int i=0;i<12;i++){float aa=i*2*PI/12,bb=(i+1)*2*PI/12;P p=point(a,aa),q=point(a,bb),t=point(b,bb),s=point(b,aa);
   face({p,q,t,s},shade(color,.76f+.25f*std::cos(aa-.7f)));if(cap.a)face({b,s,t},shade(cap,.94f+.15f*std::sin(aa)));
  }
 }
 void pedestal(float height,float radius=.56f){
  frustum(0,0,radius*1.22f,radius*1.18f,0,7,shade(Stone,.82f),true,12);
  frustum(0,0,radius*1.18f,radius*.75f,7,14,Stone,true,12);
  frustum(0,0,radius*.69f,radius*.61f,14,height-10,shade(Stone,.9f),true,12);
  frustum(0,0,radius*.65f,radius*1.06f,height-10,height-4,Stone,true,12);
  frustum(0,0,radius*1.06f,radius*1.08f,height-4,height,rgb(192,198,185),true,12);
  for(int i=0;i<5;i++){float z=18+(height-30)*i/5;ring(0,0,radius*.66f,radius*.61f,z,shade(Stone,.68f),12);}
  for(int i=-1;i<=1;i++){float u=i*radius*.25f;stroke({u,radius*.64f,20},{u,radius*.64f,height-14},shade(Stone,.64f),1);stroke({u+.018f,radius*.64f,20},{u+.018f,radius*.64f,height-14},shade(Stone,1.18f),.65f);}
 }
 void bottle(float u,float v,float z,Color liquid,float size=1){
  frustum(u,v,.12f*size,.15f*size,z,z+9*size,rgb(113,150,158),true,10);
  frustum(u,v,.15f*size,.07f*size,z+9*size,z+13*size,shade(liquid,1.12f),true,10);
  frustum(u,v,.066f*size,.066f*size,z+13*size,z+18*size,rgb(165,207,209),true,8);
  frustum(u,v,.075f*size,.075f*size,z+18*size,z+20*size,rgb(127,93,58),true,8);
  stroke({u-.045f,v+.12f,z+2*size},{u-.045f,v+.12f,z+10*size},rgb(224,248,232),1.2f);
 }
 void openBook(float u,float v,float z,float size=1,float slope=0){
  // Keep the two pages and lettering above their cover and tabletop.
  // Average-depth sorting of a broad cover can otherwise erase the rear page.
  size_t first=faces.size();float layer=0;for(const auto& f:faces)layer=std::max(layer,f.depth);
  auto pt=[&](float a,float b,float h){return P{u+a*size,v+b*size,z+h*size-b*slope};};
  face({pt(-.65f,-.45f,0),pt(.65f,-.45f,0),pt(.65f,.43f,0),pt(-.65f,.43f,0)},rgb(94,43,32));
  face({pt(-.6f,-.41f,2),pt(0,-.41f,8),pt(0,.39f,7),pt(-.6f,.39f,1)},Paper);
  face({pt(0,-.41f,8),pt(.6f,-.41f,3),pt(.6f,.39f,1),pt(0,.39f,7)},shade(Paper,.93f));
  for(int i=0;i<6;i++){float b=-.3f+i*.105f;
   stroke(pt(-.52f,b,3),pt(-.1f,b,7),rgb(125,106,81),.65f);stroke(pt(.09f,b,7),pt(.5f-(i%3)*.06f,b,3),rgb(121,104,80),.65f);
  }
  for(int side:{-1,1})for(int i=0;i<3;i++)stroke(pt(side*.61f,-.4f,i*.65f),pt(side*.61f,.38f,i*.65f),rgb(173,156,124),.6f);
  stroke(pt(0,-.4f,8.5f),pt(0,.56f,7),rgb(159,42,48),2.3f);
  for(size_t i=first;i<faces.size();i++)faces[i].depth=layer+.25f+(i-first)*.001f;
 }
};
void column(Model& m){
 m.frustum(0,0,.57f,.57f,0,7,shade(Stone,.72f),true,12);m.frustum(0,0,.54f,.40f,7,17,Stone,true,12);
 for(int row=0;row<8;row++){float z=17+row*17;Color stone=shade(Stone,.9f+(Game::hash(row,52)%10)*.013f);m.frustum(0,0,.345f,.34f,z,z+16,stone,true,12);}
 for(float u:{-.16f,0.f,.16f}){m.stroke({u,.33f,25},{u,.33f,147},shade(Stone,.61f),1.2f);m.stroke({u+.025f,.33f,25},{u+.025f,.33f,147},shade(Stone,1.15f),.8f);}
 m.frustum(0,0,.35f,.5f,153,160,Stone,true,12);m.frustum(0,0,.52f,.52f,160,167,rgb(193,201,193),true,12);m.frustum(0,0,.39f,.3f,167,171,shade(Brass,.77f),true,12);
 m.stroke({-.1f,.352f,77},{.1f,.352f,92},rgb(193,163,105),1.4f);m.stroke({.1f,.352f,92},{0,.352f,67},rgb(193,163,105),1.4f);
}
void table(Model& m,bool alchemy){
 for(float u:{-.62f,.62f})for(float v:{-.5f,.5f}){m.box(u-.1f,u+.1f,v-.1f,v+.1f,0,38,shade(Wood,.8f));m.box(u-.14f,u+.14f,v-.14f,v+.14f,0,5,shade(Wood,.58f));m.frustum(u,v,.12f,.13f,28,33,shade(Wood,1.15f),true,8);}
 for(float v:{-.5f,.5f})m.box(-.67f,.67f,v-.055f,v+.055f,14,19,shade(Wood,.65f));
 m.frustum(0,0,1.03f,1.07f,36,41,shade(Wood,.72f),true,24);m.frustum(0,0,1.07f,1.07f,41,45,Wood,true,24);m.ring(0,0,1.06f,1.025f,45.3f,shade(Brass,.8f),40);
 for(int i=0;i<8;i++){float a=-.72f+i*.19f,span=std::sqrt(1-a*a)*.95f;m.stroke({-span,a,45.4f},{span,a,45.4f},shade(Wood,.72f),.65f);}
 if(alchemy){m.frustum(-.39f,.08f,.28f,.27f,46,56,Stone,true,12);m.ring(-.39f,.08f,.27f,.19f,56.2f,rgb(207,209,191));m.frustum(-.39f,.08f,.19f,.19f,55,55.4f,rgb(44,62,42),true,12);m.tube({-.41f,.07f,54},{-.09f,-.04f,70},3.2f,rgb(177,182,158),Stone);
  m.bottle(.49f,-.27f,45,rgb(72,166,133),1.1f);m.bottle(.72f,.16f,45,rgb(186,71,84),.85f);m.openBook(-.32f,-.57f,46,.55f);m.box(.18f,.62f,.46f,.7f,45.5f,47,rgb(203,185,140));
 }else{
  m.openBook(-.19f,.17f,46,.8f);m.box(.28f,.85f,-.6f,-.24f,46,51,rgb(57,76,74));m.box(.3f,.83f,-.58f,-.26f,48,51.5f,Paper);m.box(.27f,.86f,-.61f,-.23f,51.5f,53,rgb(92,45,41));
  m.frustum(.72f,-.32f,.14f,.14f,45,49,Brass,true,12);m.frustum(.72f,-.32f,.07f,.07f,49,67,rgb(220,213,178),true,10);m.stroke({.72f,-.25f,54},{.72f,-.25f,66},rgb(251,238,197),1.1f);
  m.tube({.18f,.53f,46},{.64f,.56f,47},1.1f,rgb(224,200,142),Brass);
 }
}
void fireplace(Model& m){
 m.box(-1.32f,1.32f,-.56f,.86f,0,8,shade(Stone,.82f),true);
 m.box(-1.13f,1.13f,-.52f,-.36f,8,140,shade(Stone,.65f),true);
 m.face({{-.88f,-.25f,8},{.88f,-.25f,8},{.88f,-.25f,105},{-.88f,-.25f,105}},rgb(20,20,24));
 for(int side:{-1,1})for(int row=0;row<5;row++){float a=side<0?-1.13f:.83f,b=side<0?-.83f:1.13f;m.box(a,b,-.4f,.48f,8+row*18,25+row*18,shade(Stone,.91f+(row%2)*.05f),true);}
 for(int i=0;i<11;i++){float a=i*PI/11,b=(i+1)*PI/11;auto p=[](float angle,bool outer){return P{std::cos(angle)*(outer?1.13f:.82f),.495f,88+std::sin(angle)*(outer?48.f:34.f)};};
  m.face({p(a,true),p(b,true),p(b,false),p(a,false)},shade(Stone,1.02f+(i%2)*.09f));m.stroke(p(a,true),p(a,false),shade(Stone,.56f),.8f);
 }
 m.box(-1.24f,1.24f,-.52f,.6f,135,145,Stone,true);m.box(-1.31f,1.31f,-.57f,.67f,145,150,rgb(195,200,184));
 m.box(-.96f,.96f,-.44f,.4f,150,173,shade(Stone,.84f),true);
 m.box(-.15f,.15f,.48f,.59f,127,140,rgb(185,190,178));
 m.tube({-.52f,.03f,14},{.55f,.32f,17},5,rgb(90,54,30),rgb(158,107,62));m.tube({-.48f,.37f,17},{.56f,-.02f,22},4.5f,rgb(95,60,34),rgb(179,129,73));
 for(int i=-1;i<=1;i++)m.bottle(i*.56f,-.02f,174,i==0?rgb(66,140,155):rgb(126,77,151),.64f);
}
void shelf(Model& m){
 Color wood=rgb(112,71,47);m.box(-1.12f,1.12f,-.35f,-.27f,5,150,shade(wood,.59f));m.box(-1.23f,1.23f,-.4f,.44f,0,7,shade(wood,.8f));
 for(int side:{-1,1}){float u=side*1.08f;m.box(u-.11f,u+.11f,-.33f,.43f,7,145,wood);m.box(u-.14f,u+.14f,.38f,.46f,7,145,shade(wood,1.22f));
  for(float z:{14.f,48.f,84.f,121.f,140.f})m.box(u-.15f,u+.15f,.36f,.48f,z,z+3,Brass);
  for(float delta:{-.04f,.035f})m.stroke({u+delta,.467f,16},{u+delta,.467f,138},shade(wood,.68f),.8f);
 }
 const Color bindings[]={rgb(129,49,44),rgb(51,96,77),rgb(51,76,111),rgb(169,124,63),rgb(121,86,124),rgb(87,82,75)};
 for(int row=0;row<5;row++){
  float z=8+row*27;m.box(-1.05f,1.05f,-.3f,.43f,z,z+3,shade(wood,1.02f));m.stroke({-1.05f,.45f,z+3.1f},{1.05f,.45f,z+3.1f},Brass,.9f);
  for(int b=0;b<10;b++){unsigned h=Game::hash(b+13,row+47);float a=-.99f+b*.197f,w=.135f+(h%5)*.011f,height=17+h%7;Color c=bindings[h%6];
   m.box(a,a+w,-.23f,.3f,z+3,z+3+height,shade(c,.84f));m.box(a,a+w,.295f,.35f,z+3,z+3+height,c);
   m.box(a+.012f,a+w-.009f,-.21f,.27f,z+height+2.6f,z+height+3.05f,Paper);
   for(float band:{z+6,z+height})m.stroke({a+.015f,.365f,band},{a+w-.015f,.365f,band},rgb(206,175,104),.9f);
   m.stroke({a+w*.5f,.367f,z+11},{a+w*.5f,.367f,z+15},shade(Brass,1.1f),1.2f);
  }
 }
 m.box(-1.23f,1.23f,-.41f,.47f,145,151,shade(wood,1.14f));m.box(-1.29f,1.29f,-.45f,.52f,151,156,shade(wood,1.34f));
 for(int i=-2;i<=2;i++)m.stroke({i*.28f-.07f,.535f,149},{i*.28f+.07f,.535f,149},Brass,1.5f);
}
void lectern(Model& m){
 m.box(-.5f,.5f,-.15f,.15f,0,6,shade(Wood,.7f));m.box(-.15f,.15f,-.5f,.5f,0,6,shade(Wood,.7f));
 m.frustum(0,0,.2f,.17f,6,56,Wood,true,8);for(float z:{12.f,47.f,55.f})m.frustum(0,0,.23f,.23f,z,z+4,shade(Wood,1.15f),true,8);
 m.face({{-.74f,-.54f,79},{.74f,-.54f,79},{.74f,.55f,56},{-.74f,.55f,56}},Wood);
 m.face({{-.74f,.55f,51},{.74f,.55f,51},{.74f,.55f,56},{-.74f,.55f,56}},shade(Wood,.67f));
 m.openBook(0,0,69,1.04f,20);m.box(-.76f,.76f,.51f,.61f,51,57,shade(Brass,.76f));
}
void cauldron(Model& m){
 for(int i=0;i<3;i++){float a=i*2*PI/3;m.tube({std::cos(a)*.54f,std::sin(a)*.54f,24},{std::cos(a)*.77f,std::sin(a)*.77f,1},5,rgb(70,77,80),rgb(96,107,110));}
 m.frustum(0,0,.48f,.83f,12,23,rgb(64,75,79),false,20);m.frustum(0,0,.83f,.9f,23,43,rgb(74,88,95),false,20);m.frustum(0,0,.9f,.74f,43,65,rgb(76,89,95),false,20);
 m.frustum(0,0,.765f,.78f,65,68,rgb(169,177,166),false,24);m.ring(0,0,.79f,.68f,68,rgb(183,192,178));m.frustum(0,0,.68f,.68f,64,64.5f,rgb(38,95,69),true,24);
 m.ring(0,0,.87f,.835f,36,Brass,20);for(int i=0;i<12;i++){float a=i*2*PI/12;if(std::cos(a)+std::sin(a)>0)m.frustum(std::cos(a)*.89f,std::sin(a)*.89f,.026f,.026f,36,38,Brass,true,6);}
 for(int side:{-1,1})for(int i=0;i<20;i++){float a=i*2*PI/20,b=(i+1)*2*PI/20;m.stroke({side*.92f,.17f+std::cos(a)*.09f,54+std::sin(a)*12},{side*.92f,.17f+std::cos(b)*.09f,54+std::sin(b)*12},rgb(147,158,155),3);}
}
void garden(Model& m){
 m.box(-1.f,1.f,-.45f,.45f,0,15,rgb(108,78,47));m.box(-.91f,.91f,-.36f,.36f,15,16,rgb(43,41,29));
 for(float side:{-.48f,.48f})m.box(-1.06f,1.06f,side-.07f,side+.07f,15,19,rgb(175,136,78));
 for(float side:{-1.02f,1.02f})m.box(side-.07f,side+.07f,-.49f,.49f,15,19,rgb(166,128,72));
 for(int i=-4;i<=4;i++)m.stroke({i*.2f,.465f,1},{i*.2f,.465f,15},rgb(66,49,36),.8f);
 for(int side:{-1,1})m.box(side*.9f-.055f,side*.9f+.055f,.456f,.475f,2,14,shade(Brass,.8f));
}
void rune(Model& m){
 m.pedestal(47,.57f);
 m.face({{-.34f,-.13f,48},{.34f,-.13f,48},{.42f,-.13f,89},{0,-.13f,115},{-.42f,-.13f,89}},rgb(105,116,123));
 m.face({{-.34f,.19f,48},{.34f,.19f,48},{.42f,.19f,89},{0,.19f,115},{-.42f,.19f,89}},rgb(139,150,154));
 m.face({{.34f,-.13f,48},{.34f,.19f,48},{.42f,.19f,89},{0,.19f,115},{0,-.13f,115},{.42f,-.13f,89}},rgb(113,124,131));
 m.stroke({-.19f,.2f,63},{-.28f,.2f,73},rgb(81,97,108),.8f);m.stroke({.22f,.2f,92},{.14f,.2f,88},rgb(191,204,203),.9f);
}
void telescope(Model& m){
 for(int i=0;i<3;i++){float a=i*2*PI/3;P foot{std::cos(a)*.82f,std::sin(a)*.82f,2};m.tube({0,0,63},foot,4.8f,Wood,Brass);m.frustum(foot.x,foot.y,.11f,.09f,0,6,shade(Brass,.66f),true,8);}
 m.frustum(0,0,.13f,.13f,55,72,Brass,true,12);m.tube({-.12f,0,70},{.16f,0,70},6,shade(Brass,.86f),Brass);
 P a{-.71f,.17f,78},b{.91f,-.23f,126};auto t=[&](float f){return P{a.x+(b.x-a.x)*f,a.y+(b.y-a.y)*f,a.z+(b.z-a.z)*f};};
 m.tube(t(0),t(.2f),6,rgb(88,81,61),rgb(34,47,57));m.tube(t(.17f),t(.84f),10,rgb(166,130,69));m.tube(t(.83f),t(1),14,rgb(210,175,93),rgb(100,178,214));
 for(float f:{.25f,.51f,.77f})m.tube(t(f),t(f+.045f),10.8f,rgb(225,201,138));
 m.tube(t(.84f),t(.99f),14.5f,shade(Brass,.88f),rgb(109,182,214));m.tube(t(.988f),t(1.007f),12.5f,rgb(233,205,128),rgb(110,190,221));
 m.box(.18f,.65f,.51f,.78f,1,2,Paper);m.stroke({.22f,.62f,2.1f},{.58f,.64f,2.1f},rgb(89,117,139),.65f);
}
void astrolabe(Model& m){
 m.pedestal(57,.49f);m.frustum(0,0,.16f,.14f,57,82,Brass,true,12);
 for(int ring=0;ring<3;ring++)for(int i=0;i<64;i++){
  auto p=[&](float a){if(ring==0)return P{.75f*std::cos(a),.75f*std::sin(a),100};if(ring==1)return P{.55f*std::cos(a),0,100+30*std::sin(a)};return P{.47f*std::cos(a),.31f*std::cos(a),100+27*std::sin(a)};};
  float a=i*2*PI/64,b=(i+1)*2*PI/64;m.stroke(p(a),p(b),shade(Brass,ring==0?1.06f:.89f),2);if(i%4==0){P q=p(a);m.stroke(q,{q.x*.93f,q.y*.93f,100+(q.z-100)*.93f},rgb(231,211,153),1);}
 }
 m.frustum(0,0,.055f,.055f,71,132,shade(Brass,.73f),true,8);
}
void window(float x,float y){
 rect(x-62,y-183,124,183,rgb(73,83,94));DrawRectangleRounded({x-62,y-252,124,100},.5f,24,rgb(145,155,157));
 rect(x-49,y-180,98,176,rgb(15,27,43));DrawRectangleRounded({x-49,y-241,98,115},.5f,24,rgb(20,34,53));
 for(int i=0;i<8;i++){float yy=y-176+i*23;line(x-61,yy,x-50,yy,rgb(61,73,84),1.2f);line(x+49,yy,x+62,yy,rgb(61,73,84),1.2f);line(x-59,yy+1,x-51,yy+1,rgb(199,204,190));line(x+51,yy+1,x+60,yy+1,rgb(199,204,190));}
 for(int i=0;i<9;i++){float a=PI+i*PI/9,b=PI+(i+1)*PI/9;poly({{x+std::cos(a)*62,y-190+std::sin(a)*62},{x+std::cos(b)*62,y-190+std::sin(b)*62},{x+std::cos(b)*49,y-190+std::sin(b)*49},{x+std::cos(a)*49,y-190+std::sin(a)*49}},rgb(159+i%2*13,169+i%2*13,168+i%2*10));line(x+std::cos(a)*62,y-190+std::sin(a)*62,x+std::cos(a)*49,y-190+std::sin(a)*49,rgb(85,98,107));}
 for(int row=0;row<7;row++)for(int col=0;col<4;col++){float xx=x-38+col*25,yy=y-24-row*26;diamond(xx,yy,12,13,rgb(38+col*4,56+row*2,73+col*5));line(xx-12,yy,xx,yy-13,rgb(112,127,134,105));line(xx,yy-13,xx+12,yy,rgb(112,127,134,105));}
 rect(x-3,y-230,6,228,rgb(125,133,135));rect(x-50,y-118,100,5,rgb(122,132,134));
 rect(x-69,y-2,138,11,rgb(159,169,164));rect(x-73,y+8,146,7,rgb(107,121,127));line(x-67,y-1,x+67,y-1,rgb(221,222,200),1.5f);
}
void overlay(int id,float x,float y){
 if(id==Orb){ellipse(x,y,25,26,rgb(42,88,132));ellipse(x-4,y-4,22,23,rgb(82,146,194));ellipse(x-9,y-10,13,13,rgb(140,197,221));ellipse(x-12,y-14,6,4,rgb(231,249,237));ellipseOutline(x,y,25,26,rgb(155,203,211));intro_art::arc(x,y+3,22,9,.15f,PI*.9f,rgb(181,224,229),1);}
 else if(id==Crystal||id==Heart){float w=id==Heart?14:9,h=id==Heart?27:22;Color main=id==Heart?rgb(235,181,72):rgb(153,165,218);poly({{x,y-h},{x+w,y-3},{x+5,y+h},{x-5,y+h},{x-w,y-3}},shade(main,.8f));poly({{x,y-h},{x+3,y+2},{x,y+h},{x-w,y-3}},shade(main,1.1f));poly({{x,y-h},{x+w,y-3},{x+3,y+2}},shade(main,1.22f));poly({{x+3,y+2},{x+w,y-3},{x+5,y+h},{x,y+h}},shade(main,.68f));line(x,y-h,x+3,y+2,rgb(251,247,214),1.2f);line(x-w+3,y-3,x-3,y+9,rgb(236,236,217),1);}
 else if(id>=Herb&&id<Flame){int variant=id-Herb;Color leaves=variant==0?rgb(98,156,98):variant==1?rgb(109,157,135):rgb(133,153,93);line(x,y,x-2,y-51,rgb(116,161,98),1.4f);
  for(int i=0;i<7;i++){float side=i%2?1.f:-1.f,yy=y-8-i*6;poly({{x-1,yy+2},{x+side*15,yy-9},{x+side*18,yy-4},{x+side*8,yy+3}},shade(leaves,1+i%3*.06f));line(x,yy,x+side*15,yy-5,shade(leaves,1.42f),.7f);}
  if(variant==1){for(int i=0;i<5;i++)ellipse(x-2+(i%2?3:-3),y-51-i*2,3,4,rgb(171,123,181));}else ellipse(x-2,y-53,3,4,rgb(202,194,111));
 }else if(id==Flame){poly({{x-7,y},{x-9,y-7},{x-5,y-15},{x+2,y-27},{x+3,y-12},{x+8,y-6},{x+6,y}},rgb(234,113,40));poly({{x-4,y-1},{x-5,y-8},{x,y-18},{x+4,y-6},{x+3,y-1}},rgb(255,185,66));poly({{x-2,y-2},{x,y-11},{x+2,y-2}},rgb(255,240,158));}
}
struct Sprite {RenderTexture2D art{};Texture2D outline{};std::vector<unsigned char> mask;int w=0,h=0;float ox=0,oy=0;};
std::array<Sprite,SpriteCount> sprites{};
std::array<shadow_geometry::Sprite,SpriteCount> groundShadows;
void make(){
 for(int id=0;id<SpriteCount;id++){
  auto& s=sprites[id];bool small=id>=Orb;s.w=small?192:512;s.h=small?256:640;s.ox=small?48:128;s.oy=small?80:270;s.art=LoadRenderTexture(s.w,s.h);
  shadow_geometry::Polygons shadow;
  BeginTextureMode(s.art);ClearBackground(BLANK);BeginMode2D({{0,0},{0,0},0,2});
  if(id>=Orb)overlay(id,s.ox,s.oy);else if(id==Window)window(s.ox,s.oy);else{
   Model m(s.ox,s.oy);switch(id){case 0:column(m);break;case 1:table(m,false);break;case 2:m.pedestal(69,.55f);break;case 3:fireplace(m);break;case 4:m.pedestal(33,.55f);m.frustum(0,0,.53f,.72f,33,51,Stone,false,16);m.ring(0,0,.73f,.58f,52,rgb(206,211,191));m.frustum(0,0,.58f,.58f,49,49.4f,rgb(46,117,141),true,24);break;case 5:shelf(m);break;case 6:lectern(m);break;case 7:cauldron(m);break;case 8:garden(m);break;case 9:rune(m);break;case 10:telescope(m);break;case 11:astrolabe(m);break;case 12:m.pedestal(62,.57f);m.ring(0,0,.46f,.43f,62.2f,Brass,28);break;case AlchemyTable:table(m,true);break;}
   shadow=std::move(m.shadowFaces);m.draw();
  }EndMode2D();EndTextureMode();groundShadows[id]=shadow_geometry::bake(shadow);SetTextureFilter(s.art.texture,TEXTURE_FILTER_BILINEAR);
  Image source=LoadImageFromTexture(s.art.texture);ImageFlipVertical(&source);Color* pixels=LoadImageColors(source);s.mask.resize(s.w*s.h);int left=s.w,right=0,top=s.h,bottom=0;
  for(int y=0;y<s.h;y++)for(int x=0;x<s.w;x++){bool solid=pixels[y*s.w+x].a>=210;s.mask[y*s.w+x]=solid?255:0;if(solid){left=std::min(left,x);right=std::max(right,x);top=std::min(top,y);bottom=std::max(bottom,y);}}
  UnloadImageColors(pixels);UnloadImage(source);Image border=GenImageColor(s.w,s.h,BLANK);auto* out=static_cast<Color*>(border.data);
  for(int y=std::max(4,top-4);y<=std::min(s.h-5,bottom+4);y++)for(int x=std::max(4,left-4);x<=std::min(s.w-5,right+4);x++)if(!s.mask[y*s.w+x]){bool edge=false;for(int dy=-4;dy<=4&&!edge;dy++)for(int dx=-4;dx<=4;dx++)if(dx*dx+dy*dy<=16&&s.mask[(y+dy)*s.w+x+dx]){edge=true;break;}if(edge)out[y*s.w+x]=WHITE;}
  s.outline=LoadTextureFromImage(border);SetTextureFilter(s.outline,TEXTURE_FILTER_BILINEAR);UnloadImage(border);
 }
}
void unload(){for(auto& s:groundShadows)shadow_geometry::unload(s);for(auto& s:sprites){if(s.art.id)UnloadRenderTexture(s.art);if(s.outline.id)UnloadTexture(s.outline);s={};}}
struct Part {int id;Vector2 shift{};float scale=1;Color tint=WHITE;};
struct Parts {std::array<Part,8> values{};int count=0;void add(int id,Vector2 shift={},float scale=1,Color tint=WHITE){if(count<int(values.size()))values[count++]={id,shift,scale,tint};}};
Parts parts(int type,float time,int floor){
 Parts p;p.add(type==1&&floor==2?AlchemyTable:type);
 if(type==0)p.add(Crystal,{0,-193+std::sin(time*1.4f)*2},.85f);
 if(type==1&&floor!=2){Vector2 o=offset({.72f,-.32f,68});p.add(Flame,o,.64f+.04f*std::sin(time*6));}
 if(type==2)p.add(Orb,{0,-104+std::sin(time*1.4f)*2});
 if(type==3)for(int i=-2;i<=2;i++){Vector2 o=offset({i*.19f,.08f,21});p.add(Flame,o,1.17f+.14f*std::sin(time*4.3f+i*1.7f));}
 if(type==8)for(int i=0;i<3;i++){Vector2 o=offset({-.56f+i*.56f,.07f,19});o.x+=std::sin(time*1.3f+i)*1.2f;p.add(Herb+i,o,.8f);}
 if(type==11){p.add(Orb,{0,-100},.36f);for(int i=0;i<3;i++){float a=time*.4f+i*2*PI/3;p.add(Orb,{std::cos(a)*41,-100+std::sin(a)*18},.14f,i%2?rgb(244,205,120):rgb(162,214,222));}}
 if(type==12)p.add(Heart,{0,-104+std::sin(time*1.4f)*3});return p;
}
void drawSprite(int id,Vector2 base,float scale=1,Color tint=WHITE,bool hover=false){
 const auto& s=sprites[id];if(!s.art.id)return;Rectangle target{base.x-s.ox*scale,base.y-s.oy*scale,s.w*.5f*scale,s.h*.5f*scale};
 if(hover)DrawTexturePro(s.outline,{0,0,float(s.w),float(s.h)},target,{0,0},0,Gold);
 DrawTexturePro(s.art.texture,{0,0,float(s.w),-float(s.h)},target,{0,0},0,tint);
}
bool spriteContains(const Part& p,Vector2 base,Vector2 point){
 const auto& s=sprites[p.id];int x=int(std::floor(((point.x-base.x-p.shift.x)/p.scale+s.ox)*2)),y=int(std::floor(((point.y-base.y-p.shift.y)/p.scale+s.oy)*2));
 return x>=0&&x<s.w&&y>=0&&y<s.h&&!s.mask.empty()&&s.mask[y*s.w+x];
}
bool contains(int type,Vector2 base,Vector2 point,float time,int floor){Parts p=parts(type,time,floor);for(int i=p.count-1;i>=0;i--)if(spriteContains(p.values[i],base,point))return true;return false;}
void draw(int type,Vector2 base,float time,int floor,int index,bool hover){
 float x=base.x,y=base.y;Color accent=floor==0?Gold:floor==2?Green:floor==3?rgb(184,139,248):Blue;

 Parts p=parts(type,time,floor);drawSprite(p.values[0].id,base,1,WHITE,hover);
 if(type==0||type==2||type==12){float z=type==0?193:type==2?104:104;intro_art::glow(x,y-z,50,52,Fade(type==12?Gold:Blue,.12f));}
 if(type==3)intro_art::glow(x-3,y-42,85,78,rgb(255,147,48,30));
 for(int i=1;i<p.count;i++)drawSprite(p.values[i].id,{x+p.values[i].shift.x,y+p.values[i].shift.y},p.values[i].scale,p.values[i].tint,hover);
 if(type==2){intro_art::arc(x,y-102,34,12,time*.3f,time*.3f+PI*1.2f,Fade(Brass,.8f),1);for(int i=0;i<3;i++){float a=time*.6f+i*2*PI/3;intro_art::star(x+std::cos(a)*33,y-102+std::sin(a)*12,2,Gold);}}
 if(type==4){for(int i=0;i<3;i++){float age=std::fmod(time*.28f+i*.33f,1.f);ellipseOutline(x,y-49,4+age*27,2+age*13,Fade(rgb(167,226,229),(1-age)*.55f));}intro_art::glow(x,y-49,58,34,rgb(107,205,226,27));}
 if(type==7){for(int i=0;i<5;i++){float a=i*1.9f+time;ellipse(x+std::sin(a)*26,y-64+std::cos(a)*9,2+std::sin(a)*.6f,1.7f,rgb(176,221,166,175));float age=std::fmod(time*.3f+i*.19f,1.f);ellipse(x+std::sin(age*5+i)*12,y-75-age*42,6+age*9,4+age*5,rgb(165,210,182,int(31*(1-age))));}}
 if(type==9){bool lit=index>=0&&(game.towerRuneMask&(1<<index));Color c=lit?accent:rgb(80,105,121);if(lit)intro_art::glow(x-5,y-86,50,56,Fade(accent,.12f));line(x-13,y-88,x+2,y-99,c,2);line(x+2,y-99,x-3,y-73,c,2);line(x-3,y-73,x-13,y-88,c,1.5f);line(x-13,y-81,x+5,y-87,c,1.5f);}
 if(type==12){for(int i=1;i<5;i++){float a=time*.3f+i*2*PI/5;diamond(x+std::cos(a)*37,y-101+std::sin(a)*19,4,8,rgb(128,155,179,125));}ellipseOutline(x,y-61,25,12,Fade(Gold,.45f));}
}
}
