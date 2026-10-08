#include "raylib.h"
#include "rlgl.h"
#include "game.hpp"
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <sstream>
#include "typography.hpp"
#include "audio.hpp"
using namespace ember;
namespace fs=std::filesystem;
constexpr int W=1440,H=900,WorldW=1050;
const Color Ink{15,23,25,255},Panel{20,29,31,255},Card{27,38,40,255},Line{52,65,63,255},Gold{224,192,128,255},Text{239,241,230,255},Muted{175,195,188,255},Green{105,176,136,255},Blue{104,167,202,255};
Typography typography;float renderScale=1;RenderTexture2D canvas{},terrain,worldCanvas{};std::array<RenderTexture2D,PropTypeCount> propArt;Game game;
constexpr int TerrainWidth=MapSize*80+200,TerrainHeight=MapSize*40+300,TerrainOriginX=TerrainWidth/2;
AudioSystem audioSystem;
#include "performance.hpp"
Vector2 mouse{};Slot dragging{};bool paused=false,help=false,welcome=true;int nearbyPage=0;float scale=1;Rectangle viewport{};
bool statsOpen=false,spellBookOpen=false;Attributes statDraft{};int draftForLevel=-1;
Color rgb(int r,int g,int b,int a=255){return{(unsigned char)r,(unsigned char)g,(unsigned char)b,(unsigned char)a};}
Color shade(Color c,float s){return rgb(std::clamp(int(c.r*s),0,255),std::clamp(int(c.g*s),0,255),std::clamp(int(c.b*s),0,255),c.a);}
void rect(float x,float y,float w,float h,Color c){DrawRectangleRec({x,y,w,h},c);}
void line(float x,float y,float x2,float y2,Color c,float width=1){DrawLineEx({x,y},{x2,y2},width,c);}
void ellipse(float x,float y,float rx,float ry,Color c){rlPushMatrix();rlTranslatef(x,y,0);DrawEllipse(0,0,rx,ry,c);rlPopMatrix();}
void ellipseOutline(float x,float y,float rx,float ry,Color c){rlPushMatrix();rlTranslatef(x,y,0);DrawEllipseLines(0,0,rx,ry,c);rlPopMatrix();}
void poly(std::initializer_list<Vector2> points,Color c){const Vector2* v=points.begin(); // raylib triangles are counterclockwise in screen space
 for(size_t i=1;i+1<points.size();i++){Vector2 a=v[0],b=v[i],d=v[i+1];float cross=(b.x-a.x)*(d.y-a.y)-(b.y-a.y)*(d.x-a.x);if(cross>0)std::swap(b,d);DrawTriangle(a,b,d,c);}}
void diamond(float x,float y,float rx,float ry,Color c){poly({{x,y-ry},{x+rx,y},{x,y+ry},{x-rx,y}},c);}
float fontSize(float size,bool title){return title?size:std::max(14.f,size);}
void txt(std::string s,float x,float y,float size,Color c=Text,bool title=false){typography.draw(s,x,y,fontSize(size,title),c,title);}
float textWidth(std::string s,float size,bool title=false){return typography.width(s,fontSize(size,title),title);}
void clip(float x,float y,float w,float h){BeginScissorMode(int(std::round(x*renderScale)),int(std::round(y*renderScale)),int(std::round(w*renderScale)),int(std::round(h*renderScale)));}
void center(std::string s,float x,float y,float size,Color c=Text,bool title=false){txt(s,x-textWidth(s,size,title)/2,y,size,c,title);}
bool inside(Rectangle r){return CheckCollisionPointRec(mouse,r);}
void panel(Rectangle r,Color c=Panel){DrawRectangleRec(r,c);DrawRectangleLinesEx(r,1,Line);}
Color rarity(int t){return t==2?Gold:t==1?Green:Muted;}
const char* rarityName(int t){return t==2?"LEGENDARNY":t==1?"NIEPOSPOLITY":"ZWYKŁY";}
Vector2 screen(V p){return{WorldW*.5f+(p.x-p.y-game.player.x+game.player.y)*40,428+(p.x+p.y-game.player.x-game.player.y)*20};}
V toWorld(Vector2 p){float x=(p.x-WorldW*.5f)/40+game.player.x-game.player.y,y=(p.y-428)/20+game.player.x+game.player.y;return{(x+y)/2,(y-x)/2};}
Rectangle bagRect(int i){return{1080.f+(i%5)*65,366.f+(i/5)*44,57,40};}
Rectangle equipRect(int i){return{1080.f+(i%3)*109,207.f+(i/3)*51,98,44};}
std::vector<int> nearby(){std::vector<int> a;for(auto&g:game.ground)if(!g.value.empty()&&distance(g.pos,game.player)<=2.5f)a.push_back(g.id);return a;}
Rectangle lootRect(int i){return{1080.f+(i%5)*65,620,57,44};}
Rectangle corpseCloseRect(){return{1375,590,27,25};}
Rectangle actionRect(int i){return{343.f+i*77,799,65,63};}
#include "audio_ui.hpp"
#include "display_ui.hpp"
enum class HoverKind { None, Enemy, Prop, Ground, Corpse };
struct WorldHover { bool tileValid=false; V tile{}; HoverKind kind=HoverKind::None; int index=-1; };
struct DrawObject { float depth; int type,index; };
WorldHover hovered{};
WorldHover pickWorld(Vector2 point);
bool worldPointerAllowed(Vector2 point);
Slot hoverSlot(){
 for(int i=0;i<BagSize;i++)if(inside(bagRect(i)))return{Place::Bag,i};for(int i=0;i<EquipSize;i++)if(inside(equipRect(i)))return{Place::Equip,i};
 if(game.findCorpse(game.openedCorpse)){for(int i=0;i<CorpseLootSize;i++)if(inside(lootRect(i)))return{Place::Corpse,i,game.openedCorpse};}
 else {auto n=nearby();for(int i=0;i<5;i++){int j=nearbyPage*5+i;if(j<(int)n.size()&&inside(lootRect(i)))return{Place::Ground,n[j]};}}
 WorldHover hit=pickWorld(mouse);if(hit.kind==HoverKind::Ground)return{Place::Ground,hit.index};
 return{};
}
void beginDrag(Slot s){Item* p=game.get(s);if(!p||p->empty())return;if(game.reachable(s))dragging=s;else game.say("Podejdź bliżej, aby podnieść przedmiot.");}
void finishDrag(){
 if(dragging.place==Place::None)return;Slot dest=hoverSlot();
 if(dest.place!=Place::None)game.transfer(dragging,dest);
 else if(worldPointerAllowed(mouse)){if(dragging.place==Place::Corpse)game.say("Przeciągnij łup z ciała do plecaka.");else if(!game.drop(dragging,toWorld(mouse)))game.say("Odłóż przedmiot bliżej postaci, na wolne pole.");}
 dragging={};
}
void icon(Item i,float x,float y,float s=1,float alpha=1){
 auto C=[alpha](Color c){return Fade(c,alpha);};Color edge=C(rgb(12,19,22)),steel=C(rgb(172,195,192)),light=C(rgb(226,233,210)),wood=C(rgb(121,80,46)),gold=C(Gold),tint=C(rarity(i.tier));
 auto R=[&](float a,float b,float w,float h,Color c){rect(x+a*s,y+b*s,w*s,h*s,c);};
 auto L=[&](float a,float b,float d,float e,Color c,float w){line(x+a*s,y+b*s,x+d*s,y+e*s,c,w*s);};
 auto P=[&](std::initializer_list<Vector2> vs,Color c){std::vector<Vector2> p;for(auto v:vs)p.push_back({x+v.x*s,y+v.y*s});for(size_t j=1;j+1<p.size();j++){Vector2 a=p[0],b=p[j],d=p[j+1];if((b.x-a.x)*(d.y-a.y)-(b.y-a.y)*(d.x-a.x)>0)std::swap(b,d);DrawTriangle(a,b,d,c);}};
 if(i.empty())return;
 switch(i.kind){
 case Kind::Sword:L(-11,15,13,-15,edge,8);L(-9,12,12,-14,steel,5);L(-9,12,12,-14,light,1);L(-13,5,-1,15,gold,4);L(-14,18,-8,11,wood,4);break;
 case Kind::Axe:L(-12,17,11,-15,edge,7);L(-12,17,11,-15,wood,4);P({{0,-13},{10,-19},{20,-9},{11,-1},{4,-5}},steel);L(10,-18,19,-9,light,2);break;
 case Kind::Helmet:P({{-16,6},{-14,-9},{0,-18},{14,-9},{16,6},{7,16},{-7,16}},edge);P({{-13,5},{-11,-8},{0,-14},{11,-8},{13,5},{6,12},{-6,12}},steel);P({{0,-14},{11,-8},{13,5},{6,12},{0,10}},C(rgb(90,116,119)));R(-12,0,24,5,edge);R(-2,-14,4,29,gold);break;
 case Kind::Armor:P({{-10,-16},{-23,-7},{-15,4},{-10,0},{-11,18},{11,18},{10,0},{15,4},{23,-7},{10,-16},{5,-10},{-5,-10}},edge);P({{-9,-13},{-19,-6},{-14,0},{-8,-4},{-8,15},{8,15},{8,-4},{14,0},{19,-6},{9,-13},{4,-7},{-4,-7}},C(rgb(87,125,120)));R(-9,7,18,4,wood);R(-2,6,5,6,gold);L(-5,-5,-5,5,steel,2);break;
 case Kind::Shield:P({{-17,-14},{0,-18},{17,-14},{15,6},{0,20},{-15,6}},edge);P({{-13,-11},{0,-14},{13,-11},{11,4},{0,15},{-11,4}},gold);P({{-10,-8},{0,-11},{10,-8},{8,3},{0,11},{-8,3}},wood);L(0,-11,0,11,steel,3);L(-9,-2,9,-2,steel,3);break;
 case Kind::Boots:R(-15,-14,11,24,edge);R(2,-12,11,24,edge);R(-17,6,16,10,edge);R(0,8,17,10,edge);R(-12,-11,7,21,wood);R(5,-9,7,21,wood);R(-14,8,12,5,wood);R(3,10,12,5,wood);L(-12,-5,-5,-5,gold,2);L(5,-3,12,-3,gold,2);break;
 case Kind::Ring:DrawRing({x,y+2*s},9*s,14*s,0,360,32,gold);diamond(x,y-11*s,7*s,9*s,C(i.tier>0?Green:Blue));break;
 case Kind::Health:case Kind::Mana:{Color liquid=C(i.kind==Kind::Health?rgb(195,71,83):rgb(69,134,197));R(-7,-20,14,7,edge);R(-5,-19,10,6,wood);R(-6,-13,12,10,steel);ellipse(x,y+5*s,14*s,17*s,edge);ellipse(x,y+5*s,11*s,14*s,C(rgb(144,181,182)));ellipse(x,y+8*s,9*s,10*s,liquid);R(-9,0,3,9,light);L(-4,-9,3,-9,light,2);break;}
 case Kind::Crystal:P({{0,-21},{12,-6},{8,14},{-1,21},{-12,5},{-9,-13}},C(rgb(66,142,156)));P({{0,-21},{2,-4},{-1,21},{-12,5},{-9,-13}},C(rgb(137,224,212)));L(0,-18,8,-6,light,2);break;
 case Kind::Gold:ellipse(x-5*s,y+8*s,12*s,6*s,wood);ellipse(x-5*s,y+4*s,12*s,6*s,gold);ellipse(x+6*s,y-2*s,11*s,6*s,wood);ellipse(x+6*s,y-5*s,11*s,6*s,gold);L(3,-5,9,-5,light,2);break;
 case Kind::WolfFang:case Kind::BoarTusk:P({{-9,-17},{5,-13},{10,-2},{6,12},{-5,21},{0,5},{-2,-6}},light);L(-8,-14,-2,-7,wood,3);L(5,-8,5,7,steel,1);if(i.kind==Kind::BoarTusk)L(-4,17,10,4,light,4);break;
 case Kind::WolfFur:case Kind::BoarHide:{Color fur=C(i.kind==Kind::WolfFur?rgb(153,165,157):rgb(139,100,66));P({{-16,-14},{-5,-18},{6,-14},{18,-18},{14,-4},{20,8},{10,17},{-4,13},{-16,20},{-19,6},{-13,-3}},fur);for(int n=0;n<6;n++)L(-11+n*5,-9,-15+n*5,9,shade(fur,.7f),2);break;}
 case Kind::SpiderSilk:ellipse(x,y,16*s,18*s,steel);ellipse(x,y,12*s,15*s,light);for(int n=0;n<6;n++)L(-11,-10+n*4,11,-14+n*5,steel,1);L(7,12,17,19,light,2);break;
 case Kind::VenomSac:ellipse(x,y+2*s,12*s,16*s,C(rgb(76,123,67)));ellipse(x,y+4*s,8*s,11*s,C(rgb(151,195,82)));L(-8,-16,8,-16,wood,4);L(-4,-13,-5,6,light,2);break;
 case Kind::BoneShard:L(-9,12,9,-12,edge,10);L(-9,12,9,-12,light,6);for(float a:{-1.f,1.f}){ellipse(x+(9*a-3)*s,y+(-12*a-2)*s,4*s,4*s,light);ellipse(x+(9*a+3)*s,y+(-12*a+2)*s,4*s,4*s,light);}break;
 case Kind::SlimeGel:ellipse(x,y+6*s,16*s,11*s,C(rgb(68,112,65)));ellipse(x-3*s,y,11*s,14*s,C(rgb(123,174,99)));ellipse(x-6*s,y-5*s,3*s,6*s,light);break;
 case Kind::AncientBark:P({{-14,-16},{4,-19},{17,-10},{12,17},{-5,20},{-18,9}},wood);for(int n=0;n<4;n++)L(-10+n*6,-13,-13+n*6,14,shade(wood,.62f),3);L(-8,-13,-4,11,gold,1);break;
 case Kind::ForestEssence:DrawRing({x,y},13*s,16*s,0,360,24,C(Green));P({{0,-21},{10,-6},{5,15},{0,21},{-7,11},{-10,-4}},C(rgb(145,206,115)));L(0,-13,0,12,light,2);break;
 case Kind::BanditBadge:P({{-13,-15},{13,-15},{16,5},{0,19},{-16,5}},wood);P({{-9,-11},{9,-11},{11,3},{0,13},{-11,3}},gold);L(-5,7,5,-6,edge,3);L(-5,-6,5,7,edge,3);break;
 case Kind::RuneStone:P({{-13,-14},{0,-20},{15,-7},{11,16},{-3,20},{-16,7}},steel);L(-5,12,-5,-9,C(Blue),3);L(-5,-9,7,-3,C(Blue),3);L(7,-3,-5,4,C(Blue),3);break;
 case Kind::Dagger:L(-9,14,8,-10,edge,7);P({{4,-7},{16,-19},{10,-2}},steel);L(7,-9,15,-17,light,2);L(-9,12,-3,3,wood,4);L(-9,0,4,10,gold,3);break;
 case Kind::Cloak:P({{-9,-15},{8,-15},{13,-3},{20,18},{0,13},{-20,18},{-13,-4}},C(rgb(100,130,119)));P({{-8,-14},{0,-20},{9,-14},{7,-6},{-6,-6}},steel);L(-2,-2,-2,12,edge,2);ellipse(x,y-7*s,3*s,3*s,gold);break;
 case Kind::Amulet:DrawRing({x,y-3*s},12*s,14*s,200,520,24,gold);P({{0,-6},{10,4},{5,17},{-5,17},{-10,4}},gold);diamond(x,y+6*s,6*s,9*s,C(rgb(226,151,65)));L(-3,1,-2,9,light,2);break;
 default:break;
 }
 if(i.tier>0){DrawCircleV({x+18*s,y-17*s},2.5f*s,tint);}
}
void tree(float x,float y,float s){
 ellipse(x,y,40*s,15*s,rgb(8,18,18,95));
 poly({{x-8*s,y},{x-5*s,y-94*s},{x+8*s,y-94*s},{x+11*s,y+3*s}},rgb(62,54,42));line(x+4*s,y-3*s,x+3*s,y-94*s,rgb(102,83,53),4*s);
 for(int j=0;j<4;j++){float yy=y-(42+j*29)*s,w=(57-j*10)*s;
  poly({{x,y-(119+j*18)*s},{x+w,yy},{x+16*s,yy-2*s},{x+31*s,yy+10*s},{x-5*s,yy+18*s},{x-w,yy+3*s}},rgb(23+j*5,51+j*7,45+j*4));
  poly({{x,y-(119+j*18)*s},{x-5*s,yy+10*s},{x-w,yy+3*s}},rgb(45+j*5,79+j*6,62+j*5));
  line(x-23*s,yy,x-5*s,yy-12*s,rgb(72,103,75),1.5f*s);
 }
}
void rock(float x,float y,float s){ellipse(x,y+1,39*s,14*s,rgb(7,17,17,90));poly({{x-36*s,y-7*s},{x-25*s,y-34*s},{x+5*s,y-45*s},{x+29*s,y-22*s},{x+34*s,y-2*s},{x+7*s,y+9*s}},rgb(68,85,82));poly({{x-36*s,y-7*s},{x-25*s,y-34*s},{x+5*s,y-45*s},{x+8*s,y-17*s}},rgb(106,120,104));poly({{x+8*s,y-17*s},{x+5*s,y-45*s},{x+29*s,y-22*s},{x+34*s,y-2*s}},rgb(85,101,93));line(x-22*s,y-31*s,x+2*s,y-38*s,rgb(148,151,118),2*s);ellipse(x-13*s,y-3*s,14*s,4*s,rgb(78,103,61));}
void bush(float x,float y,float s){ellipse(x,y,35*s,12*s,rgb(9,22,20,80));for(int i=0;i<7;i++){float dx=(i%4-1.5f)*15*s,dy=(i/4)*12*s;DrawCircle(int(x+dx),int(y-14*s-dy),(11+(i%3))*s,rgb(48+i*3,84+i*3,60+i));}for(int i=0;i<5;i++)DrawCircle(int(x+(i*13%41-20)*s),int(y-(i*17%24+9)*s),2*s,rgb(173,127,81));}
void column(float x,float y,float s){ellipse(x,y,34*s,12*s,rgb(8,18,19,100));diamond(x,y,35*s,17*s,rgb(96,105,93));poly({{x-21*s,y-8*s},{x-21*s,y-105*s},{x,y-94*s},{x,y+4*s}},rgb(93,108,98));poly({{x,y+4*s},{x,y-94*s},{x+21*s,y-105*s},{x+21*s,y-8*s}},rgb(56,77,75));diamond(x,y-105*s,25*s,12*s,rgb(149,152,127));line(x-12*s,y-20*s,x-12*s,y-90*s,rgb(120,130,110),3*s);line(x+8*s,y-14*s,x+8*s,y-78*s,rgb(40,59,59),2*s);line(x-21*s,y-57*s,x-3*s,y-48*s,rgb(43,63,60),2*s);ellipse(x-15*s,y-4*s,17*s,5*s,rgb(60,98,61));}
void deciduousTree(float x,float y,float s,bool birch){
 ellipse(x,y,46*s,16*s,rgb(8,18,18,95));Color trunk=birch?rgb(188,196,170):rgb(99,76,46);
 poly({{x-7*s,y},{x-4*s,y-145*s},{x+5*s,y-145*s},{x+10*s,y}},trunk);
 for(int n=0;n<6;n++)line(x-5*s,y-(17+n*19)*s,x+4*s,y-(14+n*19)*s,birch?rgb(65,75,60):rgb(65,52,35),3*s);
 for(float side:{-1.f,1.f})line(x,y-89*s,x+side*34*s,y-129*s,trunk,5*s);
 Color leaf=birch?rgb(103,137,73):rgb(53,100,64);
 for(int n=0;n<10;n++){float a=n*2.4f,xx=x+std::cos(a)*(24+n%3*9)*s,yy=y-(120+n%4*15)*s;
  ellipse(xx,yy,(birch?23:29)*s,(20+n%3*4)*s,shade(leaf,.82f+n*.035f));
  line(xx-10*s,yy-8*s,xx+3*s,yy-14*s,shade(leaf,1.3f),2*s);
 }
}
void mushrooms(float x,float y,float s){ellipse(x,y,31*s,10*s,rgb(9,22,20,80));for(int n=0;n<5;n++){float xx=x+(n*17%43-21)*s,yy=y-(n%2)*8*s;line(xx,yy,xx,yy-(9+n%3*3)*s,rgb(192,185,136),4*s);ellipse(xx,yy-(10+n%3*3)*s,(7+n%3*2)*s,5*s,rgb(151+n*8,78+n*3,57));DrawCircleV({xx-2*s,yy-(11+n%3*3)*s},1.5f*s,Text);}}
#include "actor_art.hpp"
void hero(float x,float y,float s,bool portrait=false);
void monster(Enemy&e,float x,float y);
void makeArt(){
 terrain=LoadRenderTexture(TerrainWidth,TerrainHeight);BeginTextureMode(terrain);ClearBackground(BLANK);
 for(int sum=0;sum<MapSize*2;sum++)for(int x=0;x<MapSize;x++){int y=sum-x;if(y<0||y>=MapSize)continue;float sx=TerrainOriginX+(x-y)*40,sy=90+(x+y)*20;unsigned h=Game::hash(x,y);bool land=x>=1&&y>=1&&x<=MapSize-2&&y<=MapSize-2;
  if(!land)continue; // The animated water is drawn underneath the land.
  if(x==MapSize-2||y==MapSize-2){poly({{sx-40,sy},{sx,sy+20},{sx+40,sy},{sx+40,sy+29},{sx,sy+49},{sx-40,sy+29}},rgb(43,53,45));line(sx,sy+21,sx,sy+48,rgb(30,44,40));}
  bool path=Game::onPath(x,y);Color c=path?rgb(91+h%8,105+h%9,89+h%8):
   y>=30?(x>=30?rgb(38+h%10,67+h%11,49+h%8):rgb(44+h%10,68+h%11,59+h%8)):
   y>=26&&x<20?rgb(66+h%10,88+h%11,54+h%8):rgb(49+h%10,76+h%11,57+h%8);diamond(sx,sy,40,20,c);
  line(sx-40,sy,sx,sy-20,shade(c,1.11f));line(sx,sy+20,sx+40,sy,shade(c,.85f));
  if(path){line(sx-21,sy-7,sx+12,sy+9,shade(c,.7f));line(sx-3,sy+1,sx+11,sy-6,shade(c,.7f));if(h%3==0)line(sx-18,sy+9,sx-10,sy+5,rgb(138,146,112));}
  else for(int k=0;k<5;k++){float a=float((h>>(k*3))%47)-23,b=float((h>>(k+2))%17)-8;line(sx+a,sy+b,sx+a-2,sy+b-4,rgb(76+h%13,103+h%15,69+h%12));if((h+k)%11==0)DrawCircle(int(sx+a),int(sy+b-4),1.4f,rgb(168,152,91));}
 }
 // A broken ritual circle in the northern courtyard.
 float cx=TerrainOriginX+(21-5)*40,cy=90+(21+5)*20;DrawEllipseLines(int(cx),int(cy),91,45,rgb(159,154,111));DrawEllipseLines(int(cx),int(cy),77,38,rgb(141,140,103));
 for(int i=0;i<8;i++){float a=i*PI/4;diamond(cx+std::cos(a)*84,cy+std::sin(a)*42,4,3,Gold);}
 EndTextureMode();
 SetTextureFilter(terrain.texture,TEXTURE_FILTER_BILINEAR);
 for(int i=0;i<PropTypeCount;i++){propArt[i]=LoadRenderTexture(180,230);BeginTextureMode(propArt[i]);ClearBackground(BLANK);if(i==0)tree(90,208,1);if(i==1)rock(90,208,1);if(i==2)bush(90,208,1);if(i==3)column(90,208,1);if(i==4)deciduousTree(90,208,1,true);if(i==5)deciduousTree(90,208,1,false);if(i==6)mushrooms(90,208,1);EndTextureMode();}
 for(auto& art:propArt)SetTextureFilter(art.texture,TEXTURE_FILTER_BILINEAR);
 fs::create_directories("assets/generated");
 if(!FileExists("assets/generated/terrain-v20.png")){Image im=LoadImageFromTexture(terrain.texture);ImageFlipVertical(&im);ExportImage(im,"assets/generated/terrain-v20.png");UnloadImage(im);}
 for(int i=0;i<PropTypeCount;i++){const char* path=TextFormat("assets/generated/prop-%d.png",i);if(!FileExists(path)){Image im=LoadImageFromTexture(propArt[i].texture);ImageFlipVertical(&im);ExportImage(im,path);UnloadImage(im);}}
 if(!FileExists("assets/generated/items-v20.png")){RenderTexture2D icons=LoadRenderTexture(832,128);BeginTextureMode(icons);ClearBackground(BLANK);for(int i=1;i<ItemKindCount;i++)icon(item(static_cast<Kind>(i)),((i-1)%13)*64.f+32,((i-1)/13)*64.f+32);EndTextureMode();Image im=LoadImageFromTexture(icons.texture);ImageFlipVertical(&im);ExportImage(im,"assets/generated/items-v20.png");UnloadImage(im);UnloadRenderTexture(icons);}
 if(!FileExists("assets/generated/characters-v20.png")){RenderTexture2D actors=LoadRenderTexture((EnemyTypeCount+1)*144,170);BeginTextureMode(actors);ClearBackground(BLANK);hero(72,153,1);for(int i=0;i<EnemyTypeCount;i++){Enemy e{i,i,{},{},100,100};monster(e,216.f+i*144,153);}EndTextureMode();Image im=LoadImageFromTexture(actors.texture);ImageFlipVertical(&im);ExportImage(im,"assets/generated/characters-v20.png");UnloadImage(im);UnloadRenderTexture(actors);}
}
#include "actor_view.hpp"
#include "corpse_art.hpp"
#include "environment.hpp"
#include "world_hover.hpp"
#include "magic_art.hpp"
void drawWorld(){
 clip(0,0,WorldW,H);ClearBackground(rgb(22,44,47));drawWindWater();performance.end(RenderStage::Water,true);Vector2 o=screen({0,0});DrawTextureRec(terrain.texture,{0,0,float(TerrainWidth),-float(TerrainHeight)},{o.x-TerrainOriginX,o.y-90},WHITE);
 for(int i=0;i<32;i++){float x=std::fmod(i*167.3f+game.time*(3+i%3),float(WorldW)),y=std::fmod(i*92.7f+std::sin(game.time*.6f+i)*13,740.f)+90;DrawCircle(int(x),int(y),i%3==0?2:1,Fade(Gold,.15f+.2f*std::sin(game.time+i)));}
 Vector2 pp=screen(game.player);DrawEllipseLines(int(pp.x),int(pp.y+2),25,12,Fade(Gold,.7f));
 drawHoveredTile();
 for(auto&g:game.ground){if(g.value.empty())continue;Vector2 p=screen(g.pos);if(p.x<-50||p.x>WorldW+50||p.y<0||p.y>H)continue;float pulse=.55f+std::sin(game.time*3+g.id)*.2f;if(hovered.kind==HoverKind::Ground&&hovered.index==g.id){DrawEllipseLines(int(p.x),int(p.y),21,10,Gold);diamond(p.x,p.y,20,10,Fade(Gold,.25f));}ellipse(p.x,p.y,18,8,Fade(rarity(g.value.tier),.17f));line(p.x,p.y-9,p.x,p.y-40,Fade(rarity(g.value.tier),pulse*.3f),2);if(dragging.place!=Place::Ground||dragging.index!=g.id)icon(g.value,p.x,p.y-14,.58f);}
 auto objects=worldDrawOrder();
 for(auto obj:objects){if(obj.type==0){auto&p=game.props[obj.index];Vector2 q=screen(p.pos);if(q.x<-130||q.x>WorldW+130||q.y<-50||q.y>H+210)continue;float width=(p.type==4||p.type==5?88:66)*p.size,height=(treeProp(p.type)?205:160)*p.size;float alpha=(std::abs(q.x-pp.x)<width&&q.y>pp.y+4&&q.y-pp.y<height)? .38f:1.f;if(hovered.kind==HoverKind::Prop&&hovered.index==obj.index)drawPropTexture(p,propOutlines[p.type],true,Gold);drawPropTexture(p,propArt[p.type].texture,false,Fade(WHITE,alpha));}
  else if(obj.type==1){auto&e=game.enemies[obj.index];Vector2 q=screen(e.pos);if(q.x<-80||q.x>WorldW+80||q.y<0||q.y>H+100)continue;monster(e,q.x,q.y);}
  else if(obj.type==4){auto&e=game.enemies[obj.index];Vector2 q=screen(e.pos);if(q.x<-80||q.x>WorldW+80||q.y<-50||q.y>H+60)continue;corpseArt(e,q.x,q.y,hovered.kind==HoverKind::Corpse&&hovered.index==e.id?Gold:game.openedCorpse==e.id?Green:BLANK);}
  else if(obj.type==3)drawFireball(game.fireballs[obj.index]);
  else if(!game.dead()||int(game.time*5)%2==0)hero(pp.x,pp.y,1);
 }
 for(const auto& blast:game.explosions)drawExplosion(blast);
 for(auto&e:game.effects){Vector2 q=screen(e.pos);Color c=e.color==1?Green:e.color==2?Blue:e.color==4?rgb(240,130,113):Gold;center(e.text,q.x,q.y-75-(1-e.life)*30,18,Fade(c,std::min(1.f,e.life)));}
 DrawRectangleGradientV(0,0,WorldW,170,rgb(9,20,23,238),rgb(9,20,23,0));DrawRectangleGradientV(0,680,WorldW,220,rgb(9,20,23,0),rgb(9,20,23,250));
 if(game.hurt>0)rect(0,0,WorldW,H,Fade(rgb(154,47,40),game.hurt*.3f));EndScissorMode();
}
void bar(float x,float y,float w,float h,float value,float max,Color c){rect(x,y,w,h,rgb(9,19,21));rect(x+1,y+1,(w-2)*std::clamp(value/max,0.f,1.f),h-2,c);line(x+1,y+1,x+std::max(1.f,(w-2)*std::clamp(value/max,0.f,1.f)),y+1,shade(c,1.2f));}
#include "inspection_hud.hpp"
#include "progression_ui.hpp"
#include "input_actions.hpp"
void slotDraw(Slot s,Rectangle r,std::string label=""){
 Item* p=game.get(s);bool hot=inside(r);Color border=hot?Gold:Line;bool same=dragging.place==s.place&&dragging.index==s.index&&dragging.owner==s.owner;Color bg=Card;
 if(dragging.place!=Place::None&&hot){
  Item*a=game.get(dragging);bool allowed=a&&Game::accepts(s,*a)&&s.place!=Place::Corpse;
  if(a&&dragging.place==Place::Corpse)allowed=s.place==Place::Bag&&p&&(p->empty()||(stackable(a->kind)&&a->kind==p->kind&&a->tier==p->tier&&p->count<99));
  border=allowed?Green:rgb(186,83,76);
 }
 DrawRectangleRec(r,bg);DrawRectangleLinesEx(r,1,border);line(r.x+4,r.y+4,r.x+12,r.y+4,Fade(border,.6f));line(r.x+4,r.y+4,r.x+4,r.y+12,Fade(border,.6f));
 if(p&&!p->empty()&&!same){rect(r.x+1,r.y+r.height-3,r.width-2,2,Fade(rarity(p->tier),p->tier?.9f:.3f));icon(*p,r.x+r.width*.5f,r.y+r.height*.48f,.78f);if(p->count>1)txt(std::to_string(p->count),r.x+r.width-20,r.y+r.height-18,12,Text);}
 else if(!label.empty())center(label,r.x+r.width/2,r.y+r.height/2-7,11,Muted);
}
void minimap(){Rectangle r{842,133,180,155};panel(r,Fade(Panel,.92f));txt("WIELKA KNIEJA",856,144,11,Gold);
 const float sx=78.f/(MapSize-2),sy=48.f/(MapSize-2);
 auto point=[&](V p){return Vector2{932+(p.x-p.y)*sx,176+(p.x+p.y-2)*sy};};
 for(int x=1;x<MapSize-1;x++)for(int y=1;y<MapSize-1;y++){Vector2 p=point({float(x),float(y)});DrawPixel(int(p.x),int(p.y),Game::onPath(x,y)?rgb(126,132,100):rgb(61,87,69));}
 for(auto&e:game.enemies)if(e.alive){Vector2 p=point(e.pos);DrawCircleV(p,e.type==3?3:1.6f,e.type==3?Gold:rgb(166,90,75));}
 DrawCircleV(point(game.player),3,Text);txt("N",925,158,9,Muted);}
void drawUI(){
 txt("EMBERFALL",28,22,34,Text,true);txt("ECHA ZAPOMNIANYCH RUIN",31,66,10,Gold);line(29,92,308,92,Fade(Gold,.35f));
 txt("01",366,29,24,Gold,true);txt(Game::region(game.player),410,29,20,Text);txt("POGRANICZE  /  STREFA DZIKICH",411,56,10,Muted);
 DrawCircle(903,39,3,Green);txt(TextFormat("%d FPS",GetFPS()),914,30,13,Green);txt("F1  POMOC",917,56,10,Muted);
 minimap();
 txt("DZIENNIK WYPRAWY",29,680,10,Gold);line(29,701,242,701,Fade(Gold,.3f));
 diamond(36,727,6,6,game.bossDead?Green:Gold);txt(game.bossDead?"Ruiny odzyskane":"Echo w kamieniu",53,714,20,Text,true);
 txt(game.bossDead?"Strażnik upadł. Zbierz jego skarby.":"Pokonaj strażnika północnych ruin.",29,748,13,Muted);
 txt(TextFormat("Pokonani przeciwnicy: %d / %d",game.kills,int(game.enemies.size())),29,771,12,Muted);
 txt("WSAD",29,835,10,Gold);txt("ruch",102,833,13,Muted);txt("ESC",166,835,10,Gold);txt("pauza",197,833,13,Muted);
 txt("SHIFT",29,862,10,Gold);txt("bieg (przytrzymaj)",102,860,13,Muted);
 if(game.messageTime>0){float tw=textWidth(game.message,13);float width=std::min(760.f,tw+32);rect(WorldW/2.f-width/2,744,width,29,Fade(Panel,.94f));center(game.message,WorldW/2.f,750,13,Text);}
 for(int i=0;i<5;i++){
  Rectangle r=actionRect(i);panel(r,i>0&&inside(r)?rgb(40,52,50):Panel);
  if(i==0)icon(game.equipment[0],r.x+33,r.y+26,.8f);
  if(i==1)fireIcon(r.x+33,r.y+26,.7f,game.time*3);
  if(i==2||i==3)icon(item(i==2?Kind::Health:Kind::Mana),r.x+33,r.y+26,.7f);
  if(i==4){DrawRing({r.x+33,r.y+25},11,14,40,320,24,Gold);poly({{r.x+42,r.y+10},{r.x+52,r.y+12},{r.x+45,r.y+21}},Gold);}
  float cd=i==0?game.attackCd/.46f:i==1?game.spellCd/FireballCooldown:i==4?game.rollCd/RollCooldown:game.potionCd/.65f;
  if(cd>0)rect(r.x+1,r.y+1,r.width-2,(r.height-2)*std::clamp(cd,0.f,1.f),Fade(Ink,.65f));
  const char* keys[]={"LPM","C","1","2","SPACJA"};center(keys[i],r.x+32,r.y+48,9,Gold);
 }
 txt("ATAK",360,870,9,Muted);center("KSIĘGA · K",452,870,9,Muted);txt("ZDROWIE",505,870,9,Muted);txt("MANA",593,870,9,Muted);center("UNIK",683,870,9,Muted);
 rect(WorldW,0,W-WorldW,H,Panel);line(WorldW,0,WorldW,H,Gold);rect(WorldW+1,0,4,H,rgb(32,42,41));
 txt("TWÓJ BOHATER",1080,25,11,Gold);panel(statsButton(),inside(statsButton())?Card:Panel);center("STATYSTYKI · P",1322,23,13,Gold);line(1080,50,1400,50,Line);
 panel({1080,68,63,68},rgb(37,54,51));clip(1081,69,61,66);hero(1110,137,.8f,true);EndScissorMode();
 txt("Wędrowiec",1158,70,25,Text,true);txt(TextFormat("POZIOM %02d  /  STRAŻNIK ŻARU",game.level),1159,105,10,Gold);
 txt("ŻYCIE",1080,151,10,Muted);txt(TextFormat("%d / %d",int(game.hp),game.maxHp()),1328,148,12,Text);bar(1139,153,173,9,game.hp,float(game.maxHp()),rgb(153,73,73));
 txt("MANA",1080,175,10,Muted);txt(TextFormat("%d / %d",int(game.mana),game.maxMana()),1328,172,12,Text);bar(1139,177,173,9,game.mana,float(game.maxMana()),rgb(66,127,161));
 const char* names[]={"BROŃ","HEŁM","TARCZA","ZBROJA","BUTY","PIERŚCIEŃ"};for(int i=0;i<EquipSize;i++)slotDraw({Place::Equip,i},equipRect(i),names[i]);
 txt(TextFormat("ATAK  %d",game.damage()),1080,314,11,Gold);txt(TextFormat("PANCERZ  %d",game.defense()),1192,314,11,Muted);txt(TextFormat("PD  %d/%d",game.xp,game.level*80),1302,314,11,Muted);
 line(1080,338,1400,338,Line);txt("PLECAK",1080,346,11,Gold);int occupied=0;for(auto&i:game.bag)if(!i.empty())occupied++;txt(TextFormat("%02d / 25",occupied),1348,346,11,Muted);
 for(int i=0;i<BagSize;i++)slotDraw({Place::Bag,i},bagRect(i));
 line(1080,612,1400,612,Line);
 if(Enemy* corpse=game.findCorpse(game.openedCorpse)){
  panel({1076,584,328,116},rgb(17,29,30));DrawRectangleLinesEx({1076,584,328,116},1,Fade(Green,.6f));
  txt(std::string("ŁUP: ")+enemyInfo(corpse->type).name,1080,594,14,Gold);
  panel(corpseCloseRect(),inside(corpseCloseRect())?Card:Panel);center("X",1388,594,14,Gold);
  int count=0;for(int i=0;i<CorpseLootSize;i++){slotDraw({Place::Corpse,i,corpse->id},lootRect(i));if(!corpse->loot[i].empty())count++;}
  txt(count?"Przeciągnij łup do plecaka.":"Ciało jest puste.",1080,679,14,count?Muted:Green);
 }else{
  txt("NA ZIEMI",1080,595,10,Gold);auto n=nearby();int pages=std::max(1,(int(n.size())+4)/5);nearbyPage=std::clamp(nearbyPage,0,pages-1);txt(TextFormat("%d  W ZASIĘGU",int(n.size())),1270,595,10,Muted);
  for(int i=0;i<5;i++){int j=nearbyPage*5+i;if(j<(int)n.size())slotDraw({Place::Ground,n[j]},lootRect(i));else panel(lootRect(i),rgb(19,29,31));}
  txt("LPM: przeciągnij · PPM: wypij",1080,679,11,Muted);
  if(pages>1){rect(1286,668,117,25,Panel);txt(TextFormat("<  %d / %d  >",nearbyPage+1,pages),1300,673,11,Gold);}
 }
 drawInspectionHud();
 line(1080,880,1400,880,Line);DrawCircle(1085,894,3,Green);txt(game.bossDead?"WYPRAWA UKOŃCZONA":"WYPRAWA W TOKU",1096,885,10,Muted);txt("v0.20",1370,885,10,Muted);
}
void tooltip(){if(dragging.place!=Place::None||welcome||help||paused)return;Slot s=hoverSlot();Item*p=game.get(s);if(!p||p->empty())return;float x=std::min(mouse.x+20,1106.f),y=std::min(mouse.y+20,740.f);panel({x,y,312,139},rgb(12,22,25,250));rect(x,y,3,139,rarity(p->tier));txt(rarityName(p->tier),x+17,y+13,10,rarity(p->tier));txt(itemName(*p),x+17,y+34,18,Text);std::string stat;
 if(gearSlot(p->kind)>=0)stat=(gearSlot(p->kind)==0||gearSlot(p->kind)==5?"Atak +":"Pancerz +")+std::to_string(power(*p));else if(p->kind==Kind::Health)stat="Odnawia 50 punktów zdrowia";else if(p->kind==Kind::Mana)stat="Odnawia 35 punktów many";else if(material(p->kind))stat=materialDescription(p->kind);else stat="Skarb znaleziony w ruinach";
 txt(stat,x+17,y+65,13,Gold);txt((s.place==Place::Ground||s.place==Place::Corpse)&&!game.reachable(s)?"Podejdź bliżej, aby podnieść.":s.place==Place::Corpse?"LPM: przeciągnij do plecaka":"LPM: przeciągnij przedmiot",x+17,y+92,12,Muted);
 if(s.place==Place::Corpse){const Enemy* body=game.findCorpse(s.owner);int chance=body?lootChance(body->type,*p):0;txt(TextFormat("Szansa z tego gatunku: %d%%",chance),x+17,y+112,12,Muted);}
 else txt(material(p->kind)?"Materiał / trofeum · stos do 99":p->kind==Kind::Health||p->kind==Kind::Mana?"PPM: wypij miksturę":"Upuść na mapę, aby odłożyć.",x+17,y+112,12,Muted);
}
void overlay(){
 if(!welcome&&!help&&!paused&&!game.dead())return;rect(0,0,W,H,rgb(6,15,18,205));Rectangle r{340,172,760,548};panel(r,rgb(19,31,33));line(370,195,1070,195,Gold);diamond(720,222,6,9,Gold);
 if(game.dead()){center("ŻAR PRZYGASŁ",720,274,39,Text,true);center("Ruiny zatrzymały twoją wyprawę.",720,347,18,Muted);center("Naciśnij ENTER, aby rozpocząć od nowa.",720,411,17,Gold);center("Spróbuj zakładać lepszy sprzęt i unikać zamachów.",720,462,15,Muted);return;}
 center(welcome?"WITAJ W EMBERFALL":help?"PRZEWODNIK WĘDROWCA":"CHWILA ODDECHU",720,258,32,Text,true);
 center("Echa zapomnianych ruin",720,307,19,Gold);line(420,348,1020,348,Line);
 const char* keys[]={"W S A D","SHIFT","SPACJA","LPM","C","PPM","1 / 2","PRZECIĄGNIJ"};const char* desc[]={"W — góra, S — dół, A — lewo, D — prawo.","Przytrzymaj z WSAD, aby biec 50% szybciej.","Przeturlaj się w kierunku ruchu lub ustawienia.","Jedno kliknięcie na mapie — zamach obszarowy.","Rzuć wybrany czar w stronę kursora (10 many).","Otwórz ciało potwora i jego łup w HUD.","Wypij miksturę zdrowia / many z plecaka.","Przenieś łup z okna ciała lub ziemi do plecaka."};
 for(int i=0;i<8;i++){txt(keys[i],384,363+i*29,15,Gold);txt(desc[i],554,360+i*29,19,Text);}
 Rectangle b{540,613,360,57};panel(b,inside(b)?rgb(73,82,61):rgb(49,62,51));center(welcome?"WYRUSZ":"WRÓĆ DO GRY",720,629,21,Text);center("ENTER — powrót  /  K — księga  /  P — statystyki  /  F2 — dźwięk  /  ESC — pauza",720,687,14,Muted);
}
void input(float dt){
 if(audioInput())return;
 if(progressionInput())return;
 if(IsKeyPressed(KEY_F1)){help=!help;paused=false;welcome=false;dragging={};}
 if(IsKeyPressed(KEY_ESCAPE)){if(help||welcome){help=false;welcome=false;}else paused=!paused;dragging={};}
 if(welcome||help||paused){if(IsKeyPressed(KEY_ENTER)||(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&inside({540,613,360,57}))){welcome=help=paused=false;}return;}
 if(game.dead()){if(IsKeyPressed(KEY_ENTER)){game=Game(std::random_device{}());dragging={};nearbyPage=0;}return;}
 if(!IsWindowFocused()){paused=true;dragging={};return;}
 V move{};if(IsKeyDown(KEY_D))move=move+V{1,-1};if(IsKeyDown(KEY_A))move=move+V{-1,1};if(IsKeyDown(KEY_S))move=move+V{1,1};if(IsKeyDown(KEY_W))move=move+V{-1,-1};
 if(IsKeyPressed(KEY_SPACE))game.startRoll(move);
 const bool running=IsKeyDown(KEY_LEFT_SHIFT)||IsKeyDown(KEY_RIGHT_SHIFT);
 if(IsKeyPressed(KEY_ONE))game.potion(Kind::Health);if(IsKeyPressed(KEY_TWO))game.potion(Kind::Mana);
 if(IsKeyPressed(KEY_C))castGameplaySpell();
 if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))rightClickWorldOrInventory();
 if(game.openedCorpse<0&&mouse.x>WorldW&&mouse.y>=594&&mouse.y<702&&GetMouseWheelMove()!=0){nearbyPage+=GetMouseWheelMove()<0?1:-1;}
 if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))leftClickGameplay(move);
 if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT))finishDrag();
 if(!statsOpen&&!spellBookOpen)game.update(dt,move,false,running);
 if(dragging.place==Place::Corpse&&game.openedCorpse!=dragging.owner)dragging={};
}
void integrationTests(){
 std::ofstream out("interaction-test-v20.txt");int count=0;
 auto check=[&](bool result,const char* label){out<<(result?"PASS: ":"FAIL: ")<<label<<"\n";out.flush();count++;if(!result)std::exit(2);};
 auto midpoint=[](Rectangle r){return Vector2{r.x+r.width/2,r.y+r.height/2};};
 check(game.ground.empty(),"normal startup contains no ground loot");
 game.addGround(item(Kind::Sword,1),game.player+V{-1,.8f});game.addGround(item(Kind::Health,0,2),game.player+V{0,-1});game.addGround(item(Kind::Helmet),game.player+V{1,0});
 Vector2 start=screen(game.ground[0].pos);mouse={start.x,start.y-14};Slot s=hoverSlot();check(s.place==Place::Ground&&s.index==game.ground[0].id,"map hit testing finds sword");
 clickWorldOrInventory();check(dragging.place==Place::Ground,"drag begins on nearby ground item through the click handler");mouse=midpoint(bagRect(7));finishDrag();check(game.bag[7].kind==Kind::Sword&&game.bag[7].tier==1,"map to backpack through pointer handlers");
 mouse=midpoint(bagRect(7));beginDrag(hoverSlot());mouse=midpoint(equipRect(0));finishDrag();check(game.equipment[0].tier==1&&game.bag[7].tier==0,"backpack to equipment swaps old weapon");
 mouse=midpoint(bagRect(0));beginDrag(hoverSlot());mouse=midpoint(equipRect(1));finishDrag();check(game.bag[0].count==5&&game.equipment[1].empty(),"invalid equipment drop keeps potion");
 mouse=midpoint(bagRect(7));beginDrag(hoverSlot());mouse=screen(game.player+V{-1,1});finishDrag();check(game.bag[7].empty(),"backpack to ground through pointer handlers");
 auto n=nearby();int helmet=-1;for(int j=0;j<int(n.size());j++)if(game.get({Place::Ground,n[j]})->kind==Kind::Helmet)helmet=j;
 check(helmet>=0&&helmet<5,"nearby loot tray contains helmet");mouse=midpoint(lootRect(helmet));beginDrag(hoverSlot());mouse=midpoint(equipRect(1));finishDrag();check(game.equipment[1].kind==Kind::Helmet,"loot tray directly to equipment");
 mouse=midpoint(bagRect(2));beginDrag(hoverSlot());mouse=midpoint(equipRect(2));finishDrag();check(game.equipment[2].kind==Kind::Shield,"shield equipped from backpack");
 mouse=midpoint(bagRect(0));game.hp=30;check(game.use(hoverSlot()),"consume pointed potion");check(game.hp==80&&game.bag[0].count==4,"potion updates health and stack");
 mouse=midpoint(bagRect(3));beginDrag(hoverSlot());mouse=screen({0,0});finishDrag();check(game.bag[3].kind==Kind::Boots,"out-of-range ground drop is cancelled");
 game.hp=game.maxHp();mouse={0,0};out<<"PASS: "<<count<<" interaction checks\n";
}
#include "visual_qa.hpp"
#include "hover_qa.hpp"
#include "roll_qa.hpp"
#include "audio_qa.hpp"
#include "inspection_qa.hpp"
#include "swing_qa.hpp"
#include "crowd_qa.hpp"
#include "progression_qa.hpp"
#include "controls_qa.hpp"
#include "corpse_qa.hpp"
#include "patrol_qa.hpp"
#include "environment_qa.hpp"
#include "presentation_qa.hpp"
#include "motion_qa.hpp"
#include "pointer_qa.hpp"
#include "performance_qa.hpp"
#include "refresh_qa.hpp"
#include "forest_qa.hpp"
int main(int argc,char**argv) {
    bool smoke=false,benchmark=false,integration=false,visualQA=false,hoverQA=false,rollQA=false,large=false;
    bool audioQA=false,noAudio=false,audioPassed=true;
    bool inspectionQA=false,inspectionPassed=true;
    bool swingQA=false;
    bool crowdQA=false,crowdPassed=true;
    bool progressionQA=false,progressionPassed=true;
    bool controlsQA=false,controlsPassed=true;
    bool corpseQA=false,corpsePassed=true;
    bool patrolQA=false,patrolPassed=true;
    bool environmentQA=false,environmentPassed=true,startFullScreen=true;
    bool presentationQA=false,presentationPassed=true;
    bool motionQA=false,motionPassed=true;
    bool pointerQA=false,pointerPassed=true;
    bool performanceQA=false,performancePassed=true;
    bool refreshQA=false,refreshPassed=true;
    bool forestQA=false,forestPassed=true;
    int frameLimit=0;
    for(int i=1;i<argc;i++){
        std::string arg=argv[i];
        if(arg=="--smoke"){smoke=true;frameLimit=180;}
        if(arg=="--benchmark"){benchmark=true;frameLimit=900;}
        if(arg=="--benchmark-long"){benchmark=true;frameLimit=1800;}
        if(arg=="--profile"){benchmark=true;frameLimit=600;performance.enabled=true;}
        if(arg=="--integration"){integration=true;smoke=true;frameLimit=180;}
        if(arg=="--visual-qa")visualQA=true;
        if(arg=="--hover-qa")hoverQA=true;
        if(arg=="--roll-qa")rollQA=true;
        if(arg=="--audio-qa")audioQA=true;
        if(arg=="--inspection-qa")inspectionQA=true;
        if(arg=="--swing-qa")swingQA=true;
        if(arg=="--crowd-qa")crowdQA=true;
        if(arg=="--progression-qa")progressionQA=true;
        if(arg=="--controls-qa")controlsQA=true;
        if(arg=="--corpse-qa")corpseQA=true;
        if(arg=="--patrol-qa")patrolQA=true;
        if(arg=="--environment-qa")environmentQA=true;
        if(arg=="--presentation-qa")presentationQA=true;
        if(arg=="--motion-qa")motionQA=true;
        if(arg=="--pointer-qa")pointerQA=true;
        if(arg=="--performance-qa")performanceQA=true;
        if(arg=="--refresh-qa")refreshQA=true;
        if(arg=="--forest-qa")forestQA=true;
        if(arg=="--fullscreen")startFullScreen=true;
        if(arg=="--windowed")startFullScreen=false;
        if(arg=="--no-audio")noAudio=true;
        if(arg=="--large")large=true;
    }
    fs::path exe=fs::absolute(argv[0]).parent_path();
    if(fs::exists(exe/"assets"))fs::current_path(exe);
    else if(fs::exists(exe.parent_path()/"assets"))fs::current_path(exe.parent_path());
    SetTraceLogLevel(LOG_WARNING);
    // The offscreen world is antialiased already. Multisampling the final
    // pixel-aligned fullscreen texture only adds an unnecessary resolve pass.
    SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_WINDOW_HIGHDPI|FLAG_VSYNC_HINT);
    InitWindow(large?1920:W,large?1080:H,"Emberfall - Echa zapomnianych ruin | v0.20");
    SetWindowMinSize(1152,720);SetExitKey(KEY_NULL);updateFramePacing(true);
    if(startFullScreen)toggleFullScreen();
    bool diagnostic=smoke||benchmark||integration||visualQA||hoverQA||rollQA||audioQA||inspectionQA||swingQA||crowdQA||progressionQA||controlsQA||corpseQA||patrolQA||environmentQA||presentationQA||motionQA||pointerQA||performanceQA||refreshQA||forestQA;
    if(!diagnostic)game.rng.seed(std::random_device{}());
    makeArt();initHoverArt();
    audioSystem.init(!noAudio&&!visualQA&&!hoverQA&&!rollQA&&!inspectionQA&&!swingQA&&!crowdQA&&!controlsQA&&!corpseQA&&!patrolQA&&!environmentQA&&!presentationQA&&!motionQA&&!pointerQA&&!performanceQA&&!refreshQA&&!forestQA,!(smoke||benchmark||visualQA||hoverQA||rollQA||audioQA||inspectionQA||swingQA||crowdQA||progressionQA||controlsQA||corpseQA||patrolQA||environmentQA||presentationQA||motionQA||pointerQA||performanceQA||refreshQA||forestQA));
    if(smoke||benchmark)welcome=false;
    if(forestQA)forestPassed=runForestQA();
    if(refreshQA)refreshPassed=runRefreshQA();
    if(pointerQA)pointerPassed=runPointerQA();
    if(performanceQA)performancePassed=runPerformanceQA();
    if(motionQA)motionPassed=runMotionQA(startFullScreen);
    if(integration)integrationTests();
    if(visualQA)exportVisualQA();
    if(hoverQA)runHoverQA();
    if(rollQA)exportRollQA();
    if(audioQA)audioPassed=runAudioQA(noAudio);
    if(inspectionQA)inspectionPassed=runInspectionQA();
    if(swingQA)exportSwingQA();
    if(crowdQA)crowdPassed=runCrowdQA();
    if(progressionQA)progressionPassed=runProgressionQA();
    if(controlsQA)controlsPassed=runControlsQA();
    if(corpseQA)corpsePassed=runCorpseQA();
    if(patrolQA)patrolPassed=runPatrolQA();
    if(environmentQA)environmentPassed=runEnvironmentQA();
    if(presentationQA)presentationPassed=runPresentationQA();
    int frame=0;
    std::vector<double> cpuTimes,frameTimes,endTimes,presentTimes;
    double previous=GetTime();
    while(!visualQA&&!hoverQA&&!rollQA&&!audioQA&&!inspectionQA&&!swingQA&&!crowdQA&&!progressionQA&&!controlsQA&&!corpseQA&&!patrolQA&&!environmentQA&&!presentationQA&&!motionQA&&!pointerQA&&!performanceQA&&!refreshQA&&!forestQA&&!WindowShouldClose()){
        double begin=GetTime();double elapsed=begin-previous;previous=begin;
        performance.begin();
        float dt=float(std::clamp(elapsed,0.0,.05));
        updateFramePacing();
        refreshDisplayLayout();
        bool displayChanged=displayInput();
        if(displayChanged)refreshDisplayLayout();
        if(smoke||benchmark){
            if(benchmark&&game.allocationRequired){Attributes points{};points[int(Attribute::MagicPower)]=game.attributePoints;game.distribute(points);statsOpen=false;}
            game.update(1.f/60,{},false);game.hp=game.maxHp();
            if(benchmark){
                V next{24+std::sin(frame*.004f)*17,24+std::cos(frame*.004f)*17};
                if(!game.rolling())game.animation.advance(next.x-game.player.x,next.y-game.player.y,1.f/60);
                game.player=next;
                if(frame%95==0)game.startRoll({std::cos(frame*.004f),-std::sin(frame*.004f)});
                if(frame%32==0)game.attack();
                if(frame%90==0){game.mana=float(game.maxMana());V aim{};for(const auto& e:game.enemies)if(e.alive&&distance(e.pos,game.player)<8){aim=e.pos-game.player;break;}game.spell(aim);}
                // Include hover picking and the outline pass in the performance test.
                for(const Enemy& enemy:game.enemies)if(enemy.alive){
                    Vector2 p=screen(enemy.pos);p.y-=enemy.type==0?18:enemy.type==1?36:enemy.type==2?66:92;
                    if(worldPointerAllowed(p)){mouse=p;break;}
                }
            }
        }else if(!displayChanged)input(dt);
        audioSystem.update(dt,game,welcome||help||paused||audioOpen||statsOpen||spellBookOpen||game.allocationRequired,smoke||benchmark||IsWindowFocused());
        performance.end(RenderStage::Update);
        renderScene();
        BeginDrawing();
        presentCanvas();
        if((smoke&&frame==119)||(!smoke&&!benchmark&&IsKeyPressed(KEY_F12))){
            fs::create_directories("screenshots");Image image=presentedImage();
            ExportImage(image,"screenshots/emberfall-v20.png");UnloadImage(image);
        }
        double submitted=GetTime();double cpu=submitted-begin;EndDrawing();double ended=GetTime();performance.end(RenderStage::Present,true);finishFramePacing();double shown=GetTime();
        if(frameLimit&&frame>60){cpuTimes.push_back(cpu*1000);frameTimes.push_back(elapsed*1000);endTimes.push_back((ended-submitted)*1000);presentTimes.push_back((shown-ended)*1000);}
        frame++;
        if(frameLimit&&frame>=frameLimit)break;
    }
    if(frameLimit&&!cpuTimes.empty()){
        std::sort(cpuTimes.begin(),cpuTimes.end());std::sort(frameTimes.begin(),frameTimes.end());
        std::ofstream f(benchmark?(startFullScreen?"benchmark-v20-fullscreen.txt":large?"benchmark-v20-large.txt":"benchmark-v20.txt"):"smoke-test-v20.txt");
        f<<"Frames: "<<frame<<"\nWindow: "<<GetScreenWidth()<<"x"<<GetScreenHeight()
         <<"\nFullscreen: "<<(fullScreen()?"yes":"no")
         <<"\nRefresh rate (Hz): "<<refreshHz<<"\nV-sync interval: "<<swapInterval
         <<"\nPresentation clock: "<<pacingName()
         <<"\nDXGI synchronized presents: "<<sync_present::presentCount()
         <<"\nOccluded presentations: "<<sync_present::occludedCount()
         <<"\nSuppressed native swaps: "<<sync_present::suppressedSwaps()
         <<"\nActual presentation interval: "<<sync_present::lastSyncInterval()
         <<"\nPresentation fallback: "<<sync_present::error()
         <<"\nAudio device opened: "<<(audioSystem.available?"yes":"no")
         <<"\nNative canvas: "<<canvas.texture.width<<"x"<<canvas.texture.height
         <<"\nAverage FPS: "<<1000/(std::accumulate(frameTimes.begin(),frameTimes.end(),0.0)/frameTimes.size())
         <<"\nCPU frame mean (ms): "<<std::accumulate(cpuTimes.begin(),cpuTimes.end(),0.0)/cpuTimes.size()
         <<"\nEndDrawing mean (ms): "<<std::accumulate(endTimes.begin(),endTimes.end(),0.0)/endTimes.size()
         <<"\nPresentation mean (ms): "<<std::accumulate(presentTimes.begin(),presentTimes.end(),0.0)/presentTimes.size()
         <<"\nCPU frame p95 (ms): "<<cpuTimes[size_t(cpuTimes.size()*.95)]
         <<"\nCPU frame p99 (ms): "<<cpuTimes[size_t(cpuTimes.size()*.99)]
         <<"\nFrame p95 (ms): "<<frameTimes[size_t(frameTimes.size()*.95)]
         <<"\nFrame p99 (ms): "<<frameTimes[size_t(frameTimes.size()*.99)]
         <<"\nFrame max (ms): "<<frameTimes.back()
         <<"\nFrames over 25 ms: "<<std::count_if(frameTimes.begin(),frameTimes.end(),[](double ms){return ms>25;})<<"\n";
    }
    performance.save();
    audioSystem.shutdown();
    sync_present::shutdown();
    unloadHoverArt();
    for(auto& t:propArt)UnloadRenderTexture(t);
    UnloadRenderTexture(terrain);if(canvas.id)UnloadRenderTexture(canvas);if(worldCanvas.id)UnloadRenderTexture(worldCanvas);
    typography.clear();CloseWindow();return audioPassed&&inspectionPassed&&crowdPassed&&progressionPassed&&controlsPassed&&corpsePassed&&patrolPassed&&environmentPassed&&presentationPassed&&motionPassed&&pointerPassed&&performancePassed&&refreshPassed&&forestPassed?0:2;
}




