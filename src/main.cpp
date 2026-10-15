#include "raylib.h"
#include "rlgl.h"
#include "native_world.hpp"
#include "game.hpp"
#include "intro_story.hpp"
#include "tower_story.hpp"
#ifndef EMBERFALL_PLAYER_BUILD
#include "developer_mode.hpp"
#endif
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <sstream>
#include "typography.hpp"
#include "startup_log.hpp"
#include "audio.hpp"
using namespace ember;
#ifdef EMBERFALL_PLAYER_BUILD
constexpr bool PlayerBuild=true;
#else
constexpr bool PlayerBuild=false;
#endif
namespace fs=std::filesystem;
constexpr int W=1440,H=900,WorldW=1050;
const Color Ink{15,23,25,255},Panel{20,29,31,255},Card{27,38,40,255},Line{52,65,63,255},Gold{224,192,128,255},Text{239,241,230,255},Muted{175,195,188,255},Green{105,176,136,255},Blue{104,167,202,255};
Typography typography;float renderScale=1;RenderTexture2D canvas{},terrain,worldCanvas{};Texture2D minimapTerrain{};std::array<RenderTexture2D,PropTypeCount> propArt;Game game;
constexpr int TerrainWidth=MapSize*80+200,TerrainHeight=MapSize*40+300,TerrainOriginX=TerrainWidth/2;
AudioSystem audioSystem;
IntroSequence intro;
TowerSequence towerStory;
#include "performance.hpp"
bool developerActive=false,developerMenuOpen=false,developerKit=true;int developerSelection=0;std::string developerError;
Rectangle developerButton();void drawDeveloperUi();
Vector2 mouse{};Slot dragging{};bool paused=false,help=false,welcome=true;int nearbyPage=0,corpsePage=0,corpsePageOwner=-1;float scale=1;Rectangle viewport{};
bool weaponWheelOpen=false;
bool statsOpen=false,spellBookOpen=false;Attributes statDraft{};int draftForLevel=-1;
 bool skillsOpen=false;SkillRanks skillDraft{};int skillDraftLevel=-1;
 void openSkills();bool clickSkills();bool confirmSkills();void drawSkillsMenu();
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
Color rarity(Rarity r){switch(r){case Rarity::Common:return WHITE;case Rarity::Rare:return rgb(88,170,255);case Rarity::VeryRare:return rgb(255,220,76);case Rarity::ExtremelyRare:return rgb(194,112,255);}return WHITE;}
Color rarity(Item value){return rarity(itemRarity(value));}
Vector2 cameraPixels(){
 return {float(std::round((double(game.player.x)-game.player.y)*40*renderScale)),
         float(std::round((double(game.player.x)+game.player.y)*20*renderScale))};
}
Vector2 screen(V p){
 // Quantize anchors in physical pixels before subtracting the camera. Static
 // geometry keeps exactly the same sampling phase as the camera moves.
 Vector2 camera=cameraPixels();
 return {float((std::round(WorldW*.5*renderScale)+std::round((double(p.x)-p.y)*40*renderScale)-camera.x)/renderScale),
         float((std::round(428.*renderScale)+std::round((double(p.x)+p.y)*20*renderScale)-camera.y)/renderScale)};
}
V toWorld(Vector2 p){
 Vector2 camera=cameraPixels();
 double x=(double(p.x)*renderScale-std::round(WorldW*.5*renderScale)+camera.x)/(40*renderScale),
        y=(double(p.y)*renderScale-std::round(428.*renderScale)+camera.y)/(20*renderScale);
 return {float((x+y)/2),float((y-x)/2)};
}
Rectangle bagRect(int i){return{1080.f+(i%5)*65,366.f+(i/5)*44,57,40};}
Rectangle equipRect(int i){return{1080.f+(i%3)*109,207.f+(i/3)*51,98,44};}
std::vector<int> nearby(){std::vector<int> a;for(auto&g:game.ground)if(game.groundVisible(g)&&!g.value.empty()&&distance(g.pos,game.player)<=2.5f)a.push_back(g.id);return a;}
std::vector<Slot> nearbySlots(){
 std::vector<Slot> slots;for(int id:nearby())slots.push_back({Place::Ground,id});
 if(!game.inTower())for(const auto& plant:game.forage)if(!plant.value.empty()&&distance(plant.pos,game.player)<=2.5f)slots.push_back({Place::Forage,plant.id});return slots;
}
Rectangle lootRect(int i){return{1080.f+(i%5)*65,620,57,44};}
int corpsePages(){const Enemy* body=game.findCorpse(game.openedCorpse);int last=0;if(body)for(int i=0;i<CorpseLootSize;i++)if(!body->loot[i].empty())last=i;return last/5+1;}
void syncCorpsePage(){if(corpsePageOwner!=game.openedCorpse){corpsePageOwner=game.openedCorpse;corpsePage=0;}corpsePage=std::clamp(corpsePage,0,corpsePages()-1);}
Rectangle corpseCloseRect(){return{1375,590,27,25};}
Rectangle skillActionRect(int i){return {167.f+i*72,799,65,63};}
Rectangle skillsButton(){return {1080,16,146,29};}
Rectangle actionRect(int i){return{392.f+i*72,799,65,63};}
#include "audio_ui.hpp"
#include "display_ui.hpp"
Rectangle weaponWheelButton(){return {28,112,158,34};}
int weaponWheelHit(Vector2 point);
enum class HoverKind { None, Enemy, Prop, Ground, Corpse, Forage, Mage, TowerObject, TowerStair };
struct WorldHover { bool tileValid=false; V tile{}; HoverKind kind=HoverKind::None; int index=-1; };
struct DrawObject { float depth; int type,index; };
WorldHover hovered{};
WorldHover pickWorld(Vector2 point);
bool worldPointerAllowed(Vector2 point);
bool talkToMage();
bool useTowerTarget(WorldHover hit);bool nearestTowerInteraction();bool towerPanelInput();void drawTowerPanel();
bool towerPanelOpen=false;int towerPanelObject=-1,towerPage=0;
void openMerchantSelection();bool clickMerchantPanel();void drawMerchantPanel();
Slot hoverSlot(){
 if(weaponWheelOpen){int sector=weaponWheelHit(mouse);if(sector>=0)return game.weaponSource(sector);}
 for(int i=0;i<BagSize;i++)if(inside(bagRect(i)))return{Place::Bag,i};for(int i=0;i<EquipSize;i++)if(inside(equipRect(i)))return{Place::Equip,i};
 if(game.findCorpse(game.openedCorpse)){syncCorpsePage();for(int i=0;i<5;i++)if(inside(lootRect(i)))return{Place::Corpse,corpsePage*5+i,game.openedCorpse};}
 else {auto n=nearbySlots();for(int i=0;i<5;i++){int j=nearbyPage*5+i;if(j<(int)n.size()&&inside(lootRect(i)))return n[j];}}
 WorldHover hit=pickWorld(mouse);if(hit.kind==HoverKind::Ground)return{Place::Ground,hit.index};
 if(hit.kind==HoverKind::Forage)return{Place::Forage,hit.index};
 return{};
}
void beginDrag(Slot s){Item* p=game.get(s);if(!p||p->empty())return;game.cancelBow();if(game.reachable(s))dragging=s;else game.say("Podejdź bliżej, aby podnieść przedmiot.");}
void finishDrag(){
 if(dragging.place==Place::None)return;
 if(weaponWheelOpen&&weaponWheelHit(mouse)>=0){game.bindWeapon(dragging,weaponWheelHit(mouse));dragging={};return;}
 Slot dest=hoverSlot();
 if(dest.place!=Place::None)game.transfer(dragging,dest);
 else if(worldPointerAllowed(mouse)){if(dragging.place==Place::Corpse||dragging.place==Place::Forage)game.say("Przeciągnij łup z ciała do plecaka.");else if(!game.drop(dragging,toWorld(mouse)))game.say("Odłóż przedmiot bliżej postaci, na wolne pole.");}
 dragging={};
}
void icon(Item i,float x,float y,float s=1,float alpha=1){
 auto C=[alpha](Color c){return Fade(c,alpha*c.a/255.f);};Color edge=C(rgb(12,19,22)),steel=C(rgb(172,195,192)),light=C(rgb(226,233,210)),wood=C(rgb(121,80,46)),gold=C(Gold),tint=C(rarity(i));
 auto R=[&](float a,float b,float w,float h,Color c){rect(x+a*s,y+b*s,w*s,h*s,c);};
 auto L=[&](float a,float b,float d,float e,Color c,float w){line(x+a*s,y+b*s,x+d*s,y+e*s,c,w*s);};
 auto P=[&](std::initializer_list<Vector2> vs,Color c){std::vector<Vector2> p;for(auto v:vs)p.push_back({x+v.x*s,y+v.y*s});for(size_t j=1;j+1<p.size();j++){Vector2 a=p[0],b=p[j],d=p[j+1];if((b.x-a.x)*(d.y-a.y)-(b.y-a.y)*(d.x-a.x)>0)std::swap(b,d);DrawTriangle(a,b,d,c);}};
 if(i.empty())return;
 if(beastGear(i.kind)){
  int theme=int(i.kind)-int(Kind::WolfKnife);
  Color palette[]={rgb(147,174,175),rgb(128,163,120),rgb(183,136,83),rgb(163,178,199),rgb(90,152,151),rgb(218,173,106)};
  Color base=C(palette[(theme/4)%6]);wood=shade(base,.62f);steel=shade(base,1.08f);gold=C(rarity(i));
 }
 if(itemRarity(i)!=Rarity::Common)ellipse(x,y+1*s,18*s,18*s,C(Fade(tint,.09f)));
 switch(archetype(i.kind)){
 case Kind::ScorpionSting:P({{-12,14},{-5,11},{-8,3},{-7,-8},{2,-17},{10,-15},{3,-8},{0,2},{4,10},{-4,16}},C(rgb(203,147,71)));L(-6,4,-5,-7,light,1.3f);P({{3,-12},{16,-7},{6,-19}},C(rgb(89,58,37)));break;
 case Kind::ScarabShell:ellipse(x,y,15*s,18*s,C(rgb(53,113,113)));ellipse(x-4*s,y-3*s,9*s,14*s,C(rgb(79,155,143)));L(0,-16,0,17,gold,2);for(int n=0;n<4;n++){L(-12,-10+n*6,0,-7+n*6,gold,.8f);L(0,-7+n*6,12,-10+n*6,gold,.8f);}break;
 case Kind::CobraScale:P({{0,-20},{13,-9},{15,4},{0,19},{-15,4},{-13,-9}},C(rgb(202,157,76)));P({{0,-18},{0,14},{-12,2},{-10,-8}},C(rgb(237,200,119)));for(int n=0;n<4;n++){L(-9,-8+n*6,0,-3+n*6,wood,.8f);L(0,-3+n*6,9,-8+n*6,wood,.8f);}break;
 case Kind::SandEssence:ellipse(x,y,15*s,15*s,C(rgb(228,181,95,60)));P({{-8,16},{-13,5},{-3,-3},{-7,-16},{5,-9},{11,0},{6,11}},C(rgb(220,167,81)));L(-6,8,7,-2,light,2);L(-9,3,1,-6,gold,1);for(int n=0;n<5;n++)ellipse(x+(n*7%23-11)*s,y+(n*11%31-15)*s,1.5f*s,1.3f*s,light);break;
 case Kind::Sword:L(-11,15,13,-15,edge,8);L(-9,12,12,-14,steel,5);L(-9,12,12,-14,light,1);L(-13,5,-1,15,gold,4);L(-14,18,-8,11,wood,4);break;
 case Kind::Axe:L(-12,17,11,-15,edge,7);L(-12,17,11,-15,wood,4);P({{0,-13},{10,-19},{20,-9},{11,-1},{4,-5}},steel);L(10,-18,19,-9,light,2);break;
 case Kind::Helmet:P({{-16,6},{-14,-9},{0,-18},{14,-9},{16,6},{7,16},{-7,16}},edge);P({{-13,5},{-11,-8},{0,-14},{11,-8},{13,5},{6,12},{-6,12}},steel);P({{0,-14},{11,-8},{13,5},{6,12},{0,10}},C(rgb(90,116,119)));R(-12,0,24,5,edge);R(-2,-14,4,29,gold);break;
 case Kind::Armor:P({{-10,-16},{-23,-7},{-15,4},{-10,0},{-11,18},{11,18},{10,0},{15,4},{23,-7},{10,-16},{5,-10},{-5,-10}},edge);P({{-9,-13},{-19,-6},{-14,0},{-8,-4},{-8,15},{8,15},{8,-4},{14,0},{19,-6},{9,-13},{4,-7},{-4,-7}},C(rgb(87,125,120)));R(-9,7,18,4,wood);R(-2,6,5,6,gold);L(-5,-5,-5,5,steel,2);break;
 case Kind::Shield:P({{-17,-14},{0,-18},{17,-14},{15,6},{0,20},{-15,6}},edge);P({{-13,-11},{0,-14},{13,-11},{11,4},{0,15},{-11,4}},gold);P({{-10,-8},{0,-11},{10,-8},{8,3},{0,11},{-8,3}},wood);L(0,-11,0,11,steel,3);L(-9,-2,9,-2,steel,3);break;
 case Kind::Boots:R(-15,-14,11,24,edge);R(2,-12,11,24,edge);R(-17,6,16,10,edge);R(0,8,17,10,edge);R(-12,-11,7,21,wood);R(5,-9,7,21,wood);R(-14,8,12,5,wood);R(3,10,12,5,wood);L(-12,-5,-5,-5,gold,2);L(5,-3,12,-3,gold,2);break;
 case Kind::Ring:DrawRing({x,y+2*s},9*s,14*s,0,360,32,gold);diamond(x,y-11*s,7*s,9*s,C(i.tier>0?Green:Blue));break;
 case Kind::Health:case Kind::Mana:{Color liquid=C(i.kind==Kind::Health?rgb(195,71,83):rgb(69,134,197));R(-7,-20,14,7,edge);R(-5,-19,10,6,wood);R(-6,-13,12,10,steel);ellipse(x,y+5*s,14*s,17*s,edge);ellipse(x,y+5*s,11*s,14*s,C(rgb(144,181,182)));ellipse(x,y+8*s,9*s,10*s,liquid);R(-9,0,3,9,light);L(-4,-9,3,-9,light,2);break;}
 case Kind::Crystal:P({{0,-21},{12,-6},{8,14},{-1,21},{-12,5},{-9,-13}},C(rgb(66,142,156)));P({{0,-21},{2,-4},{-1,21},{-12,5},{-9,-13}},C(rgb(137,224,212)));L(0,-18,8,-6,light,2);break;
 case Kind::DawnShard:
  ellipse(x,y,20*s,20*s,C(rgb(225,177,87,30)));DrawRing({x,y},15*s,17*s,20,315,32,gold);
  P({{-2,-21},{12,-8},{8,12},{-1,20},{-13,4},{-9,-13}},C(rgb(221,156,62)));
  P({{-2,-21},{1,-4},{-1,20},{-13,4},{-9,-13}},C(rgb(255,230,140)));
  P({{1,-4},{12,-8},{8,12},{-1,20}},C(rgb(252,194,84)));L(-2,-18,1,-4,light,1.5f);L(-6,-7,5,3,light,1.1f);L(5,3,-2,9,light,1.1f);break;
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
 case Kind::Bow:{
  Vector2 previous{x-9*s,y-22*s};for(int n=1;n<=14;n++){float t=n/14.f;Vector2 next{x+(-9+24*std::sin(t*PI))*s,y+(-22+44*t)*s};DrawLineEx(previous,next,4*s,wood);DrawLineEx({previous.x-s,previous.y},{next.x-s,next.y},s,gold);previous=next;}
  L(-9,-22,-9,22,light,1);L(-9,0,23,-5,wood,2);P({{23,-5},{16,-8},{18,-1}},steel);R(8,-2,3,4,gold);break;}
 case Kind::Arrow:for(int n=0;n<3;n++){float dx=(n-1)*6;L(-8+dx,16,7+dx,-15,wood,2);P({{7+dx,-19},{12+dx,-10},{4+dx,-12}},steel);L(-8+dx,16,-13+dx,11,light,2);L(-8+dx,16,-4+dx,19,light,2);}break;
 case Kind::Mushroom:
  ellipse(x,y+10*s,9*s,10*s,C(rgb(221,201,156)));
  P({{-5,1},{5,1},{8,17},{-7,17}},C(rgb(229,211,167)));L(-3,5,-2,14,wood,.8f);L(3,7,4,15,gold,.7f);
  ellipse(x,y-1*s,19*s,8*s,C(rgb(83,50,28)));
  P({{-19,-2},{-16,-11},{-8,-17},{3,-19},{13,-14},{19,-5},{20,0},{12,3},{-10,3}},C(rgb(142,82,39)));
  ellipse(x-4*s,y-9*s,11*s,6*s,C(rgb(183,119,60)));L(-13,-8,-8,-12,C(rgb(222,161,88)),1.4f);
  ellipse(x+10*s,y-3*s,2*s,s,C(rgb(111,60,29)));break;
 case Kind::Amulet:DrawRing({x,y-3*s},12*s,14*s,200,520,24,gold);P({{0,-6},{10,4},{5,17},{-5,17},{-10,4}},gold);diamond(x,y+6*s,6*s,9*s,C(rgb(226,151,65)));L(-3,1,-2,9,light,2);break;
 default:break;
 }
 if(beastGear(i.kind)){
  int shape=gearSlot(i.kind);float a=shape==0?-2.f:0.f,b=shape==0?2.f:-2.f;
  diamond(x+a*s,y+b*s,2.8f*s,4*s,tint);L(a-5,b-5,a+5,b+5,C(Fade(light,.7f)),.7f);
 }
 // Material-specific fine details stay legible at the actual backpack scale.
 switch(archetype(i.kind)){
 case Kind::Sword:case Kind::Dagger:L(-6,9,9,-10,light,1);L(-11,13,-8,15,gold,1);ellipse(x-12*s,y+16*s,2*s,2*s,gold);break;
 case Kind::Axe:L(11,-16,17,-10,light,1);L(-8,10,-5,13,gold,1);L(-4,5,-1,8,gold,1);break;
 case Kind::Helmet:L(-9,-7,-6,-10,light,1);L(6,7,8,4,gold,1);R(-8,7,2,2,gold);break;
 case Kind::Armor:for(float side:{-1.f,1.f}){L(side*6,-3,side*6,5,light,1);ellipse(x+side*12*s,y-5*s,2*s,2*s,gold);}L(-5,12,5,12,light,1);break;
 case Kind::Shield:for(float side:{-1.f,1.f}){L(side*8,-6,side*6,3,light,1);ellipse(x+side*9*s,y-7*s,1.3f*s,1.3f*s,steel);}ellipse(x,y-2*s,3*s,3*s,gold);ellipse(x-s,y-3*s,s,s,light);break;
 case Kind::Boots:L(-12,-1,-5,0,light,1);L(5,1,12,2,light,1);L(-11,2,-6,4,gold,1);L(6,4,11,6,gold,1);break;
 case Kind::Health:case Kind::Mana:ellipse(x+4*s,y+7*s,2*s,2*s,C(rgb(238,226,189,145)));ellipse(x-2*s,y+12*s,s,s,light);L(-4,-16,4,-16,gold,1);L(4,-5,8,0,light,1);break;
 case Kind::Gold:ellipseOutline(x-5*s,y+4*s,8*s,3*s,light);ellipseOutline(x+6*s,y-5*s,7*s,3*s,light);L(5,-7,5,-3,wood,1);break;
 case Kind::Ring:case Kind::Amulet:ellipse(x-9*s,y+2*s,1.6f*s,2.5f*s,light);L(-1,-11,2,-8,light,1);break;
 case Kind::WolfFang:case Kind::BoarTusk:L(-5,-10,-1,-5,gold,1);L(-4,-6,0,-2,wood,1);L(1,3,-2,13,C(rgb(253,242,198)),1.4f);break;
 case Kind::WolfFur:case Kind::BoarHide:for(int n=0;n<8;n++){float a=-12+n*3.3f;L(a,-5,a+1,4,light,.7f);L(a+2,6,a,10,wood,.6f);}break;
 case Kind::SpiderSilk:L(-8,-7,7,-9,light,1);L(-10,1,10,2,light,1);break;
 case Kind::VenomSac:ellipse(x+2*s,y+7*s,2*s,3*s,C(rgb(191,219,122)));L(-5,-17,4,-15,gold,1);break;
 case Kind::BoneShard:L(-4,5,4,-6,steel,.8f);L(1,-5,6,-10,wood,.7f);break;
 case Kind::SlimeGel:ellipse(x+6*s,y+5*s,2*s,3*s,C(rgb(176,216,121)));ellipse(x-1*s,y+8*s,s,s,light);break;
 case Kind::AncientBark:L(-4,-5,1,-2,edge,1);L(1,-2,2,9,edge,1);L(4,-10,11,-6,gold,.8f);break;
 case Kind::ForestEssence:L(-4,3,4,-4,light,1);ellipse(x-12*s,y-11*s,s,s,light);ellipse(x+13*s,y+12*s,s,s,light);break;
 case Kind::BanditBadge:R(-7,-9,2,2,light);R(5,-9,2,2,light);L(-4,9,0,12,light,1);break;
 case Kind::RuneStone:L(-9,-7,-10,5,light,1);L(7,10,9,1,edge,1);R(-6,-10,2,2,light);break;
 case Kind::Cloak:L(-8,2,-13,14,light,1);L(8,2,13,14,steel,1);L(-5,-9,5,-9,gold,1);break;
 default:break;
 }
 if(itemRarity(i)!=Rarity::Common){DrawCircleV({x+18*s,y-17*s},2.5f*s,tint);}
}
#include "scenery_art.hpp"
#include "desert_art.hpp"
#include "actor_art.hpp"
#include "sand_prison_art.hpp"
#include "intro_art.hpp"
void hero(float x,float y,float s,bool portrait=false);
void monster(Enemy&e,float x,float y);
void makeArt(){
 terrain=LoadRenderTexture(TerrainWidth,TerrainHeight);BeginTextureMode(terrain);ClearBackground(BLANK);
 for(int sum=0;sum<MapSize*2;sum++)for(int x=0;x<MapSize;x++){int y=sum-x;if(y<0||y>=MapSize)continue;float sx=TerrainOriginX+(x-y)*40,sy=90+(x+y)*20;bool land=x>=1&&y>=1&&x<=MapSize-2&&y<=MapSize-2;
  if(!land)continue; // The animated water is drawn underneath the land.
  detailedGroundTile(x,y,sx,sy);
 }
 // A broken ritual circle in the northern courtyard.
 float cx=TerrainOriginX+(21-5)*40,cy=90+(21+5)*20;DrawEllipseLines(int(cx),int(cy),91,45,rgb(159,154,111));DrawEllipseLines(int(cx),int(cy),77,38,rgb(141,140,103));
 for(int i=0;i<12;i++){float a=i*PI/6,xx=cx+std::cos(a)*84,yy=cy+std::sin(a)*42;diamond(xx,yy,3,2,rgb(193,182,131));
  line(xx-3,yy-5,xx+2,yy+3,rgb(173,170,125),1.2f);line(xx-2,yy,xx+4,yy-2,rgb(173,170,125),1.2f);}
 // A second, larger landmark gives the deep forest a recognizable destination.
 cx=TerrainOriginX+(59-57)*40;cy=90+(59+57)*20;
 for(int ring=0;ring<3;ring++)ellipseOutline(cx,cy,164-ring*17,82-ring*8.5f,rgb(147-ring*10,151-ring*9,109-ring*6));
 for(int i=0;i<16;i++){
  float a=i*PI/8,xx=cx+std::cos(a)*152,yy=cy+std::sin(a)*76;
  diamond(xx,yy,3,5,rgb(189,184,129));line(xx-5,yy-6,xx+5,yy+6,rgb(134,151,128),1.2f);
 }
 EndTextureMode();
 SetTextureFilter(terrain.texture,TEXTURE_FILTER_BILINEAR);
 // Roads never change during a run; baking their minimap avoids thousands of
 // path-distance calculations and tiny draw calls every frame on the larger map.
 Image mini=GenImageColor(180,112,BLANK);float mx=78.f/(MapSize-2),my=48.f/(MapSize-2);
 for(bool paths:{false,true})for(int x=1;x<MapSize-1;x++)for(int y=1;y<MapSize-1;y++){
  if(Game::onPath(x,y)!=paths)continue;
  ImageDrawPixel(&mini,int(90+(x-y)*mx),int(6+(x+y-2)*my),paths?rgb(126,132,100):rgb(61,87,69));
 }
 minimapTerrain=LoadTextureFromImage(mini);UnloadImage(mini);SetTextureFilter(minimapTerrain,TEXTURE_FILTER_POINT);
 for(int i=0;i<PropTypeCount;i++){int density=i>=ForestPropTypeCount?2:1;propArt[i]=LoadRenderTexture(180*density,230*density);BeginTextureMode(propArt[i]);ClearBackground(BLANK);BeginMode2D({{0,0},{0,0},0,float(density)});if(i==0)tree(90,208,1);if(i==1)rock(90,208,1);if(i==2)bush(90,208,1);if(i==3)column(90,208,1);if(i==4)deciduousTree(90,208,1,true);if(i==5)deciduousTree(90,208,1,false);if(i==6)mushrooms(90,208,1);if(i>=ForestPropTypeCount)desert_art::prop(i,90,208);EndMode2D();EndTextureMode();}
 for(auto& art:propArt)SetTextureFilter(art.texture,TEXTURE_FILTER_BILINEAR);
 fs::create_directories("assets/generated");
 if(!FileExists("assets/generated/terrain-v21.png")){Image im=LoadImageFromTexture(terrain.texture);ImageFlipVertical(&im);ExportImage(im,"assets/generated/terrain-v21.png");UnloadImage(im);}
 for(int i=0;i<ForestPropTypeCount;i++){const char* path=TextFormat("assets/generated/prop-v21-%d.png",i);if(!FileExists(path)){Image im=LoadImageFromTexture(propArt[i].texture);ImageFlipVertical(&im);ExportImage(im,path);UnloadImage(im);}}
 if(!FileExists("assets/generated/items-v38.png")){RenderTexture2D icons=LoadRenderTexture(832,64*((ItemKindCount-1+12)/13));BeginTextureMode(icons);ClearBackground(BLANK);for(int i=1;i<ItemKindCount;i++)icon(item(static_cast<Kind>(i)),((i-1)%13)*64.f+32,((i-1)/13)*64.f+32);EndTextureMode();Image im=LoadImageFromTexture(icons.texture);ImageFlipVertical(&im);ExportImage(im,"assets/generated/items-v38.png");UnloadImage(im);UnloadRenderTexture(icons);}
 if(!FileExists("assets/generated/characters-v21.png")){RenderTexture2D actors=LoadRenderTexture((EnemyTypeCount+1)*144,170);BeginTextureMode(actors);ClearBackground(BLANK);hero(72,153,1);for(int i=0;i<EnemyTypeCount;i++){Enemy e{i,i,{},{},100,100};monster(e,216.f+i*144,153);}EndTextureMode();Image im=LoadImageFromTexture(actors.texture);ImageFlipVertical(&im);ExportImage(im,"assets/generated/characters-v21.png");UnloadImage(im);UnloadRenderTexture(actors);}
}
#include "actor_view.hpp"
#include "corpse_art.hpp"
#include "environment.hpp"
#include "tower_stairs_art.hpp"
#include "merchant_art.hpp"
#include "tower_furniture_art.hpp"
#include "world_hover.hpp"
#include "magic_art.hpp"
#include "bow_art.hpp"
#include "combat_feedback.hpp"
#include "ground_shadows.hpp"
#include "tower_world.hpp"
#include "map_labels.hpp"
void drawWorld(){
 if(game.inTower()){drawTowerWorld();return;}
 clip(0,0,WorldW,H);ClearBackground(game.inDesert()?rgb(163,121,74):rgb(22,44,47));if(game.inDesert())desert_art::drawCachedBackdrop();else drawWindWater();performance.end(RenderStage::Water,true);Vector2 o=screen({0,0});if(game.inDesert())BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
 DrawTextureRec(game.inDesert()?desert_art::terrain.texture:terrain.texture,{0,0,float(TerrainWidth),-float(TerrainHeight)},{o.x-TerrainOriginX,o.y-90},WHITE);
 if(game.inDesert()){native_world::endBlendMode();desert_art::atmosphere();}
 if(!game.inDesert())for(int i=0;i<32;i++){float x=std::fmod(i*167.3f+game.time*(3+i%3),float(WorldW)),y=std::fmod(i*92.7f+std::sin(game.time*.6f+i)*13,740.f)+90;DrawCircle(int(x),int(y),i%3==0?2:1,Fade(Gold,.15f+.2f*std::sin(game.time+i)));}
 world_shadows::draw();
 Vector2 pp=screen(game.player);DrawEllipseLines(int(pp.x),int(pp.y+2),25,12,Fade(Gold,.7f));
 drawHoveredTile();
 for(const auto& plant:game.forage){
  if(plant.value.empty())continue;Vector2 p=screen(plant.pos);if(p.x<-40||p.x>WorldW+40||p.y<0||p.y>H)continue;
  if(hovered.kind==HoverKind::Forage&&hovered.index==plant.id){ellipseOutline(p.x,p.y,18,9,Gold);diamond(p.x,p.y,18,9,Fade(Gold,.17f));}
  if(dragging.place!=Place::Forage||dragging.index!=plant.id)icon(plant.value,p.x,p.y-12,.68f);
 }
 for(auto&g:game.ground){if(!game.groundVisible(g)||g.value.empty())continue;Vector2 p=screen(g.pos);if(p.x<-50||p.x>WorldW+50||p.y<0||p.y>H)continue;float pulse=.55f+std::sin(game.time*3+g.id)*.2f;if(hovered.kind==HoverKind::Ground&&hovered.index==g.id){DrawEllipseLines(int(p.x),int(p.y),21,10,Gold);diamond(p.x,p.y,20,10,Fade(Gold,.25f));}ellipse(p.x,p.y,18,8,Fade(rarity(g.value),.17f));line(p.x,p.y-9,p.x,p.y-40,Fade(rarity(g.value),pulse*.3f),2);if(dragging.place!=Place::Ground||dragging.index!=g.id)icon(g.value,p.x,p.y-14,.58f);}
 auto objects=worldDrawOrder();
 for(auto obj:objects){if(obj.type==0){auto&p=game.props[obj.index];Vector2 q=screen(p.pos);if(q.x<-130||q.x>WorldW+130||q.y<-50||q.y>H+210)continue;float width=(p.type==4||p.type==5?88:66)*p.size,height=(treeProp(p.type)?205:160)*p.size;float alpha=(std::abs(q.x-pp.x)<width&&q.y>pp.y+4&&q.y-pp.y<height)? .38f:1.f;if(hovered.kind==HoverKind::Prop&&hovered.index==obj.index)drawPropTexture(p,propOutlines[p.type],true,Gold);drawPropTexture(p,propArt[p.type].texture,false,Fade(WHITE,alpha));}
  else if(obj.type==1){auto&e=game.enemies[obj.index];Vector2 q=screen(e.pos);if(q.x<-140||q.x>WorldW+140||q.y<-20||q.y>H+160)continue;monster(e,q.x,q.y);}
  else if(obj.type==4){auto&e=game.enemies[obj.index];Vector2 q=screen(e.pos);if(q.x<-80||q.x>WorldW+80||q.y<-50||q.y>H+60)continue;corpseArt(e,q.x,q.y,hovered.kind==HoverKind::Corpse&&hovered.index==e.id?Gold:game.openedCorpse==e.id?Green:BLANK);}
  else if(obj.type==3)drawFireball(game.fireballs[obj.index]);
  else if(obj.type==5)drawArrow(game.arrows[obj.index]);
  else if(!game.dead()||int(game.time*5)%2==0){sand_prison::drawMound(pp,false);hero(pp.x,pp.y,1);sand_prison::drawMound(pp,true);}
 }
 if(game.inDesert())desert_art::drawCachedFoothills();
 for(const auto& flash:game.pierceFlashes){Vector2 p=screen(flash.pos);float age=1-flash.life/.26f;ellipseOutline(p.x,p.y-35,8+age*22,5+age*12,Fade(rgb(137,226,245),1-age));}
 for(const auto& blast:game.explosions)drawExplosion(blast);
 drawHealingAura();
 DrawRectangleGradientV(0,0,WorldW,170,game.inDesert()?rgb(52,34,24,215):rgb(9,20,23,238),game.inDesert()?rgb(52,34,24,0):rgb(9,20,23,0));DrawRectangleGradientV(0,680,WorldW,220,game.inDesert()?rgb(52,34,24,0):rgb(9,20,23,0),game.inDesert()?rgb(52,34,24,230):rgb(9,20,23,250));
 if(game.hurt>0)rect(0,0,WorldW,H,Fade(rgb(154,47,40),game.hurt*.3f));EndScissorMode();
}
void bar(float x,float y,float w,float h,float value,float max,Color c){rect(x,y,w,h,rgb(9,19,21));rect(x+1,y+1,(w-2)*std::clamp(value/max,0.f,1.f),h-2,c);line(x+1,y+1,x+std::max(1.f,(w-2)*std::clamp(value/max,0.f,1.f)),y+1,shade(c,1.2f));}
#include "inspection_hud.hpp"
#include "progression_ui.hpp"
#include "input_actions.hpp"
#include "weapon_wheel_ui.hpp"
#include "skills_ui.hpp"
void slotDraw(Slot s,Rectangle r,std::string label=""){
 Item* p=game.get(s);bool hot=inside(r);Color border=hot?Gold:Line;bool same=dragging.place==s.place&&dragging.index==s.index&&dragging.owner==s.owner;Color bg=Card;
 if(dragging.place!=Place::None&&hot){
  Item*a=game.get(dragging);bool allowed=a&&Game::accepts(s,*a)&&s.place!=Place::Corpse&&s.place!=Place::Forage;
  if(a&&(dragging.place==Place::Corpse||dragging.place==Place::Forage))allowed=s.place==Place::Bag&&p&&(p->empty()||(stackable(a->kind)&&a->kind==p->kind&&a->tier==p->tier&&p->count<99));
  border=allowed?Green:rgb(186,83,76);
 }
 DrawRectangleRec(r,bg);DrawRectangleLinesEx(r,1,border);line(r.x+4,r.y+4,r.x+12,r.y+4,Fade(border,.6f));line(r.x+4,r.y+4,r.x+4,r.y+12,Fade(border,.6f));
 if(p&&!p->empty()&&!same){rect(r.x+1,r.y+r.height-3,r.width-2,2,Fade(rarity(*p),.85f));icon(*p,r.x+r.width*.5f,r.y+r.height*.48f,.78f);if(p->count>1)txt(std::to_string(p->count),r.x+r.width-20,r.y+r.height-18,12,Text);}
 else if(!label.empty())center(label,r.x+r.width/2,r.y+r.height/2-7,11,Muted);
}
void minimap(){Rectangle r{842,133,180,155};panel(r,Fade(Panel,.92f));txt(game.inDesert()?"MORZE PIASKÓW":"WIELKA KNIEJA",856,144,11,Gold);
 if(game.inTower()){
  rect(r.x+1,r.y+1,r.width-2,r.height-2,rgb(24,23,40));txt("WIEŻA ELDRINA",856,144,11,Gold);
  ellipse(932,220,68,42,rgb(67,58,85));ellipseOutline(932,220,68,42,Fade(Gold,.5f));
  auto point=[](V p){return Vector2{932+(p.x-p.y)*6.5f,220+(p.x+p.y-24)*3.25f};};
  for(const auto& object:towerFurnishings(game.towerFloor)){Vector2 p=point(object.pos);DrawCircleV(p,2,Muted);}
  if(game.towerFloor==0)DrawCircleV(point(MagePosition),3,rgb(198,150,250));
  if(game.stairPresent(1))DrawCircleV(point(TowerStairUp),3,Gold);if(game.stairPresent(-1))DrawCircleV(point(TowerStairDown),3,Blue);DrawCircleV(point(game.player),3,Text);txt(TextFormat("PIĘTRO %d / 5",game.towerFloor+1),887,268,14,Gold);return;
 }
 const float sx=78.f/(MapSize-2),sy=48.f/(MapSize-2);
 auto point=[&](V p){return Vector2{932+(p.x-p.y)*sx,176+(p.x+p.y-2)*sy};};
 DrawTexture(game.inDesert()?desert_art::minimap:minimapTerrain,int(r.x),170,WHITE);
 if(!game.inDesert())for(const auto& glade:ForestGlades){Vector2 p=point(glade.center);diamond(p.x,p.y,2,2,Fade(Gold,.7f));}
 for(auto&e:game.enemies)if(e.alive){Vector2 p=point(e.pos);DrawCircleV(p,enemyBoss(e.type)?3:1.6f,enemyBoss(e.type)?Gold:rgb(166,90,75));}
 if(game.inDesert()){DrawCircleV(point(DesertPortal),3,rgb(192,153,240));DrawCircleV(point(DesertOasis),2,Blue);}DrawCircleV(point(game.player),3,Text);txt("N",925,158,9,Muted);}
void drawUI(){
 sand_prison::drawStatus();
 txt("EMBERFALL",28,22,34,Text,true);txt("ECHA ZAPOMNIANYCH RUIN",31,66,10,Gold);line(29,92,308,92,Fade(Gold,.35f));
 txt(game.inTower()?TextFormat("%02d / 5",game.towerFloor+1):game.inDesert()?"02":"01",335,29,20,Gold,true);txt(game.inTower()?TowerFloors[game.towerFloor].name:game.currentRegion(),410,29,20,Text);txt(game.inTower()?TowerFloors[game.towerFloor].theme:game.inDesert()?"DRUGA KRAINA / SZLAK ŻYWEJ WYDMY":"POGRANICZE  /  STREFA DZIKICH",411,56,10,Muted);
 DrawCircle(903,39,3,Green);txt(TextFormat("%d FPS",GetFPS()),914,30,13,Green);txt("F1  POMOC",917,56,10,Muted);
 minimap();
 txt("DZIENNIK WYPRAWY",29,680,10,Gold);line(29,701,242,701,Fade(Gold,.3f));
 diamond(36,727,6,6,game.dawnFragments()?Green:Gold);txt("Serce Świtu",53,714,20,Text,true);
 txt(game.inTower()?"Schody i wyposażenie · E / PPM":game.inDesert()?(game.dawnFragments()>=2?"Dwie części bezpieczne. Wróć do maga.":game.desertBossDead?"Zbierz fragment z ciała Żywej Wydmy.":"Żywa Wydma: południowy wschód."):game.bossDead?"Zbierz część Serca Świtu z ciała.":"Odnajdź Strażnika na północy.",29,748,13,Muted);
 txt(TextFormat("Odzyskane części: %d / 5",game.dawnFragments()),29,771,12,game.dawnFragments()?Green:Muted);
 txt("WSAD · ruch",29,808,13,Muted);txt("SHIFT · bieg",29,835,13,Muted);
 txt("ESC · pauza",29,862,13,Muted);
 if(game.messageTime>0&&!weaponWheelOpen){float tw=textWidth(game.message,13);float width=std::min(760.f,tw+32);rect(WorldW/2.f-width/2,744,width,29,Fade(Panel,.94f));center(game.message,WorldW/2.f,750,13,Text);}
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
 const char* labels[]={"ATAK","CZAR · K","ZDROWIE","MANA","UNIK"};for(int i=0;i<5;i++){Rectangle button=actionRect(i);center(labels[i],button.x+button.width/2,870,14,Muted);}
 rect(WorldW,0,W-WorldW,H,Panel);line(WorldW,0,WorldW,H,Gold);rect(WorldW+1,0,4,H,rgb(32,42,41));
 panel(skillsButton(),inside(skillsButton())?Card:Panel);center("UMIEJĘTNOŚCI · U",1153,23,13,Gold);panel(statsButton(),inside(statsButton())?Card:Panel);center("STATYSTYKI · P",1322,23,13,Gold);line(1080,50,1400,50,Line);
 panel({1080,68,63,68},rgb(37,54,51));clip(1081,69,61,66);hero(1110,137,.8f,true);EndScissorMode();
 txt("Wędrowiec",1158,70,25,Text,true);txt(TextFormat("POZIOM %02d  /  STRAŻNIK ŻARU",game.level),1159,105,10,Gold);
 txt(TextFormat("ZŁOTO  %lld",static_cast<long long>(game.goldBalance())),1159,125,14,Gold);
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
  syncCorpsePage();int count=0;for(const auto& value:corpse->loot)count+=!value.empty();
  for(int i=0;i<5;i++)slotDraw({Place::Corpse,corpsePage*5+i,corpse->id},lootRect(i));
  if(corpsePages()>1){panel({1286,668,117,25},Panel);center(TextFormat("<  %d / %d  >",corpsePage+1,corpsePages()),1344,674,14,Gold);}
  txt(count?"Przeciągnij do plecaka.":"Ciało jest puste.",1080,679,14,count?Muted:Green);
 }else{
  txt("W ZASIĘGU",1080,595,10,Gold);auto n=nearbySlots();int pages=std::max(1,(int(n.size())+4)/5);nearbyPage=std::clamp(nearbyPage,0,pages-1);txt(TextFormat("%d  W ZASIĘGU",int(n.size())),1270,595,10,Muted);
  for(int i=0;i<5;i++){int j=nearbyPage*5+i;if(j<(int)n.size())slotDraw(n[j],lootRect(i));else panel(lootRect(i),rgb(19,29,31));}
  txt("LPM: zbierz · PPM w plecaku: użyj",1080,679,11,Muted);
  if(pages>1){rect(1286,668,117,25,Panel);txt(TextFormat("<  %d / %d  >",nearbyPage+1,pages),1300,673,11,Gold);}
 }
 drawInspectionHud();
 line(1080,880,1400,880,Line);DrawCircle(1085,894,3,Green);txt(game.inTower()?"SCHRONIENIE ELDRINA":game.inDesert()?"WYPRAWA NA PUSTYNIĘ":"WYPRAWA W TOKU",1096,885,10,Muted);txt("v0.47",1370,885,10,Muted);
}
struct ItemTooltipText {std::string label,name,stat,description,instruction,chance,value;Color color;};
ItemTooltipText itemTooltipText(Item value,Slot source){
 ItemTooltipText text;Kind shape=archetype(value.kind);int slot=gearSlot(value.kind);
 text.color=rarity(value);text.label=rarityName(itemRarity(value));text.name=itemName(value);
 if(value.kind==Kind::DawnShard)text.stat=value.tier>=3?"Artefakt fabularny · część 2 z 5":"Artefakt fabularny · część 1 z 5";
 else if(slot>=0)text.stat=(slot==0||slot==5?"Atak +":"Pancerz +")+std::to_string(power(value));
 else if(value.kind==Kind::Arrow)text.stat="Amunicja do łuku · jedna na strzał";
 else if(value.kind==Kind::Health)text.stat="Odnawia 50 punktów zdrowia";
 else if(value.kind==Kind::Mushroom)text.stat="Po zjedzeniu odnawia 10 HP";
 else if(value.kind==Kind::Mana)text.stat="Odnawia 35 punktów many";
 else if(material(value.kind))text.stat=materialDescription(value.kind);
 else text.stat="Skarb znaleziony w ruinach";
 if(shape==Kind::Bow)text.stat+=" · zasięg zależy od naciągu";
 text.description=gearDescription(value.kind);
 text.value="Wartość: "+std::to_string(itemSellValue(value))+" złota";
 if(value.count>1)text.value+=" / szt. · stos: "+std::to_string(itemStackValue(value))+" złota";
 bool world=source.place==Place::Ground||source.place==Place::Forage||source.place==Place::Corpse;
 text.instruction=world&&!game.reachable(source)?"Podejdź bliżej, aby podnieść.":source.place==Place::Corpse?"LPM: przeciągnij do plecaka":slot==0?"Przeciągnij na sprzęt lub koło broni (Q)":value.kind==Kind::Health||value.kind==Kind::Mana||value.kind==Kind::Mushroom?"PPM: użyj · LPM: przeciągnij":"LPM: przeciągnij przedmiot";
 if(value.kind==Kind::DawnShard)text.instruction="Artefakt fabularny · nie na sprzedaż";
 text.chance=value.kind==Kind::DawnShard?"Łup fabularny · gwarantowany (100%)":"Szansa na łup: "+std::to_string(itemDropChance(value))+"% · według rzadkości";
 return text;
}
Rectangle tooltipBounds(const ItemTooltipText& text){
 float width=std::clamp(std::max({360.f,textWidth(text.value,14)+34,textWidth(text.stat,14)+34,textWidth(text.description,14)+34,textWidth(text.chance,14)+34,textWidth(text.instruction,14)+34}),360.f,560.f),height=text.description.empty()?174.f:200.f;
 return {std::clamp(mouse.x+20,8.f,W-width-12),std::clamp(mouse.y+20,8.f,H-height-14),width,height};
}
void drawItemTooltip(Item value,Slot source,const std::string& instruction=""){
 ItemTooltipText text=itemTooltipText(value,source);if(!instruction.empty())text.instruction=instruction;Rectangle box=tooltipBounds(text);
 panel(box,rgb(12,22,25,250));rect(box.x,box.y,3,box.height,text.color);
 txt(text.label,box.x+17,box.y+12,14,text.color);
 float nameSize=20;while(nameSize>14&&textWidth(text.name,nameSize)>box.width-34)nameSize--;
 txt(text.name,box.x+17,box.y+34,nameSize,text.color);
 txt(text.stat,box.x+17,box.y+65,14,Gold);
 float row=box.y+90;
 if(!text.description.empty()){txt(text.description,box.x+17,row,14,Muted);row+=26;}
 txt(text.value,box.x+17,row,14,Gold);row+=26;
 txt(text.instruction,box.x+17,row,14,Muted);txt(text.chance,box.x+17,row+24,14,value.kind==Kind::DawnShard?Gold:Muted);
}
void tooltip(){
 if(towerPanelOpen||dragging.place!=Place::None||welcome||help||paused)return;
 Slot source=hoverSlot();Item* value=game.get(source);if(value&&!value->empty())drawItemTooltip(*value,source);
}
void overlay(){
 if(!welcome&&!help&&!paused&&!game.dead())return;rect(0,0,W,H,rgb(6,15,18,205));Rectangle r{340,172,760,548};panel(r,rgb(19,31,33));line(370,195,1070,195,Gold);diamond(720,222,6,9,Gold);
 if(game.dead()){center("ŻAR PRZYGASŁ",720,274,39,Text,true);center("Ruiny zatrzymały twoją wyprawę.",720,347,18,Muted);center("Naciśnij ENTER, aby rozpocząć od nowa.",720,411,17,Gold);center("Spróbuj zakładać lepszy sprzęt i unikać zamachów.",720,462,15,Muted);return;}
 center(welcome?"WITAJ W EMBERFALL":help?"PRZEWODNIK WĘDROWCA":"CHWILA ODDECHU",720,258,32,Text,true);
 center("Echa zapomnianych ruin",720,307,19,Gold);line(420,348,1020,348,Line);
 const char* keys[]={"W S A D","SHIFT","SPACJA","LPM","C","PPM","1 / 2","PRZECIĄGNIJ"};const char* desc[]={"W — góra, S — dół, A — lewo, D — prawo.","Przytrzymaj z WSAD, aby biec 50% szybciej.","Przeturlaj się w kierunku ruchu lub ustawienia.","Miecz: kliknij. Łuk: przytrzymaj i puść.","Rzuć wybrany czar w stronę kursora (10 many).","Otwórz ciało; użyj mikstury / grzyba w plecaku.","Wypij miksturę zdrowia / many z plecaka.","Zbierz łup lub grzyb do plecaka."};
 for(int i=0;i<8;i++){txt(keys[i],384,363+i*29,15,Gold);txt(desc[i],554,360+i*29,19,Text);}
 Rectangle b{540,613,360,57};panel(b,inside(b)?rgb(73,82,61):rgb(49,62,51));center(welcome?"WYRUSZ":"WRÓĆ DO GRY",720,629,21,Text);center("Z / X — ataki specjalne  /  V — Leczenie  /  U — umiejętności  /  Q — broń  /  F2 — dźwięk  /  ESC — pauza",720,687,14,Muted);
}
namespace world_shadows {void draw();}
#include "intro_scene.hpp"
#include "tower_scene.hpp"
#include "tower_rooms_ui.hpp"
#include "merchant_ui.hpp"
#include "developer_ui.hpp"
void input(float dt){
 if(developerInput())return;
 if(intro.active()){introInput(dt);return;}
 startTowerReturn();if(towerStory.active()){towerInput(dt);return;}
 if(audioInput()){game.cancelBow();if(audioOpen)weaponWheelOpen=false;return;}
 if(towerPanelInput())return;
 if(progressionInput()){game.cancelBow();weaponWheelOpen=false;return;}
 if(weaponWheelInput())return;
 if(IsKeyPressed(KEY_F1)){game.cancelBow();help=!help;paused=false;welcome=false;dragging={};}
 if(IsKeyPressed(KEY_ESCAPE)){game.cancelBow();if(help||welcome){help=false;welcome=false;}else paused=!paused;dragging={};}
 if(welcome||help||paused){if(IsKeyPressed(KEY_ENTER)||(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&inside({540,613,360,57}))){if(welcome)startIntro();else help=paused=false;}return;}
 if(game.dead()){if(IsKeyPressed(KEY_ENTER)){
#ifndef EMBERFALL_PLAYER_BUILD
 if(developerActive)startDeveloperSession(true);else
#endif
 {game=Game(std::random_device{}());startIntro();}}return;}
 if(!IsWindowFocused()){game.cancelBow();weaponWheelOpen=false;paused=true;dragging={};return;}
 if(IsKeyPressed(KEY_E)&&game.inDesert()&&game.useDesertPortal()){dragging={};nearbyPage=0;return;}
 if(IsKeyPressed(KEY_E)&&game.inTower()){nearestTowerInteraction();if(towerStory.active()||towerPanelOpen)return;}
 V move{};if(IsKeyDown(KEY_D))move=move+V{1,-1};if(IsKeyDown(KEY_A))move=move+V{-1,1};if(IsKeyDown(KEY_S))move=move+V{1,1};if(IsKeyDown(KEY_W))move=move+V{-1,-1};
 if(IsKeyPressed(KEY_SPACE))game.startRoll(move);
 const bool running=IsKeyDown(KEY_LEFT_SHIFT)||IsKeyDown(KEY_RIGHT_SHIFT);
 if(IsKeyPressed(KEY_ONE))game.potion(Kind::Health);if(IsKeyPressed(KEY_TWO))game.potion(Kind::Mana);
 if(IsKeyPressed(KEY_C))castGameplaySpell();
 if(IsKeyPressed(KEY_Z))activateGameplaySkill(Skill::StrongCut);
 if(IsKeyPressed(KEY_X))activateGameplaySkill(Skill::PiercingArrow);
 if(IsKeyPressed(KEY_V))activateGameplaySkill(Skill::Healing);
 if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))rightClickWorldOrInventory();
 if(towerStory.active()||towerPanelOpen)return;
 if(mouse.x>WorldW&&mouse.y>=594&&mouse.y<702){float wheel=GetMouseWheelMove();if(wheel!=0){if(game.findCorpse(game.openedCorpse)){syncCorpsePage();corpsePage=std::clamp(corpsePage+(wheel<0?1:-1),0,corpsePages()-1);}else nearbyPage+=wheel<0?1:-1;}}
 if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))leftClickGameplay(move);
 if(towerStory.active()||towerPanelOpen)return;
 if(game.bowDrawing){if(worldPointerAllowed(mouse))game.aimBow(bowAimFromPointer());else game.cancelBow();}
 if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){if(dragging.place!=Place::None)finishDrag();else releaseWorldBow();}
 startTowerReturn();if(towerStory.active()||towerPanelOpen)return;
 if(game.bowDrawing&&!IsMouseButtonDown(MOUSE_BUTTON_LEFT))game.cancelBow();
 if(!statsOpen&&!skillsOpen&&!spellBookOpen)game.update(dt,move,false,running);
 if(dragging.place==Place::Corpse&&game.openedCorpse!=dragging.owner)dragging={};
}
void integrationTests(){
 std::ofstream out("interaction-test-v43.txt");int count=0;
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
#include "graphics_qa.hpp"
#include "shadow_qa.hpp"
#include "mushroom_qa.hpp"
#include "bow_qa.hpp"
#include "skills_qa.hpp"
#include "intro_qa.hpp"
#include "tower_qa.hpp"
#include "tower_floors_qa.hpp"
#ifndef EMBERFALL_PLAYER_BUILD
#include "developer_qa.hpp"
#endif
#include "critical_healing_qa.hpp"
#include "tower_stairs_qa.hpp"
#include "tower_furniture_qa.hpp"
#include "desert_qa.hpp"
#include "trade_qa.hpp"
#include "map_labels_qa.hpp"
#include "world_motion_qa.hpp"
int main(int argc,char**argv) {
    bool smoke=false,benchmark=false,integration=false,visualQA=false,hoverQA=false,rollQA=false,large=false;
    bool audioQA=false,noAudio=false,audioPassed=true,startupSmoke=false,openGL=true;
    bool introQA=false,introPassed=true,towerQA=false,towerPassed=true,towerFloorsQA=false,towerFloorsPassed=true;
    bool inspectionQA=false,inspectionPassed=true;
    bool swingQA=false,swingPassed=true;
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
    bool forestQA=false,forestPassed=true,graphicsQA=false,graphicsPassed=true,mushroomQA=false,mushroomPassed=true,bowQA=false,bowPassed=true,skillsQA=false,skillsPassed=true;
    bool desertQA=false,desertPassed=true;
    bool shadowQA=false,shadowPassed=true,tradeQA=false,tradePassed=true,labelsQA=false,labelsPassed=true,worldMotionQA=false,worldMotionPassed=true;
    bool stairsQA=false,stairsPassed=true,towerArtQA=false,towerArtPassed=true;
    bool combatQA=false,combatPassed=true;
    bool desertBenchmark=false,towerBenchmark=false,developerQA=false,developerPassed=true,developerSmoke=false;
#ifndef EMBERFALL_PLAYER_BUILD
    DeveloperLaunch developerLaunch;
#endif
    int frameLimit=0,benchmarkStrongCuts=0,benchmarkPiercingShots=0,desertRegionsVisited=0;
    for(int i=1;i<argc;i++){
        std::string arg=argv[i];
#ifndef EMBERFALL_PLAYER_BUILD
        if(parseDeveloperArgument(arg,i,argc,argv,developerLaunch))continue;
        if(arg=="--trade-qa")tradeQA=true;
        if(arg=="--labels-qa")labelsQA=true;
        if(arg=="--world-motion-qa")worldMotionQA=true;
        if(arg=="--shadow-qa")shadowQA=true;
        if(arg=="--desert-qa")desertQA=true;
        if(arg=="--stairs-qa")stairsQA=true;
        if(arg=="--tower-art-qa")towerArtQA=true;
        if(arg=="--combat-qa")combatQA=true;
        if(arg=="--dev-qa")developerQA=true;
        if(arg=="--dev-smoke"){developerSmoke=true;frameLimit=180;developerLaunch.enabled=true;}
        if(arg=="--smoke"){smoke=true;frameLimit=180;}
        if(arg=="--benchmark"){benchmark=true;frameLimit=900;}
        if(arg=="--desert-benchmark"){benchmark=true;desertBenchmark=true;frameLimit=900;}
        if(arg=="--tower-benchmark"){benchmark=true;towerBenchmark=true;frameLimit=900;}
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
        if(arg=="--graphics-qa")graphicsQA=true;
        if(arg=="--mushroom-qa")mushroomQA=true;
        if(arg=="--bow-qa")bowQA=true;
        if(arg=="--skills-qa")skillsQA=true;
        if(arg=="--intro-qa")introQA=true;
        if(arg=="--tower-qa")towerQA=true;
        if(arg=="--tower-floors-qa")towerFloorsQA=true;
#endif
        if(arg=="--fullscreen")startFullScreen=true;
        if(arg=="--windowed")startFullScreen=false;
        if(arg=="--no-audio")noAudio=true;
        if(arg=="--startup-smoke"){startupSmoke=true;frameLimit=180;}
        if(arg=="--opengl")openGL=true;
        if(arg=="--dxgi")openGL=false;
        if(arg=="--large")large=true;
    }
    fs::path exe=fs::absolute(argv[0]).parent_path();
    if(fs::exists(exe/"assets"))fs::current_path(exe);
    else if(fs::exists(exe.parent_path()/"assets"))fs::current_path(exe.parent_path());
    startupLog.open();startupLog.write("Emberfall v0.47: startup");
    startupLog.write(PlayerBuild?"Build edition: player":"Build edition: developer");
    SetTraceLogCallback(startupTrace);
    SetTraceLogLevel(LOG_INFO);
    if(openGL)sync_present::useOpenGL();
    // The offscreen world is antialiased already. Multisampling the final
    // pixel-aligned fullscreen texture only adds an unnecessary resolve pass.
    SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_WINDOW_HIGHDPI|FLAG_VSYNC_HINT);
    startupLog.write("Creating graphics window");
    InitWindow(large?1920:W,large?1080:H,"Emberfall - Echa zapomnianych ruin | v0.47");
    if(!IsWindowReady()){startupLog.write("ERROR: graphics window initialization failed");return 1;}
    startupLog.write("Graphics window ready");SetTraceLogLevel(LOG_WARNING);
    SetWindowMinSize(1152,720);SetExitKey(KEY_NULL);updateFramePacing(true);
    if(startFullScreen)toggleFullScreen();
    bool diagnostic=worldMotionQA||labelsQA||tradeQA||shadowQA||desertQA||towerArtQA||stairsQA||combatQA||developerQA||towerFloorsQA||towerQA||introQA||smoke||benchmark||integration||visualQA||hoverQA||rollQA||audioQA||inspectionQA||swingQA||crowdQA||progressionQA||controlsQA||corpseQA||patrolQA||environmentQA||presentationQA||motionQA||pointerQA||performanceQA||refreshQA||forestQA||graphicsQA||mushroomQA||bowQA||skillsQA;
    if(!diagnostic){game=Game(std::random_device{}());
#ifndef EMBERFALL_PLAYER_BUILD
        if(!developerLaunch.enabled)
#endif
        startIntro();}
    startupLog.write("Generating world graphics");
    makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();startupLog.write("World graphics ready");
    audioSystem.init(!worldMotionQA&&!labelsQA&&!tradeQA&&!shadowQA&&!desertQA&&!towerArtQA&&!stairsQA&&!combatQA&&!noAudio&&!developerQA&&!towerFloorsQA&&!towerQA&&!introQA&&!visualQA&&!hoverQA&&!rollQA&&!inspectionQA&&!swingQA&&!crowdQA&&!controlsQA&&!corpseQA&&!patrolQA&&!environmentQA&&!presentationQA&&!motionQA&&!pointerQA&&!performanceQA&&!refreshQA&&!forestQA&&!graphicsQA&&!mushroomQA&&!bowQA&&!skillsQA,!worldMotionQA&&!labelsQA&&!tradeQA&&!shadowQA&&!desertQA&&!towerArtQA&&!stairsQA&&!combatQA&&!noAudio&&!startupSmoke&&!developerQA&&!( towerFloorsQA||towerQA||introQA||smoke||benchmark||visualQA||hoverQA||rollQA||audioQA||inspectionQA||swingQA||crowdQA||progressionQA||controlsQA||corpseQA||patrolQA||environmentQA||presentationQA||motionQA||pointerQA||performanceQA||refreshQA||forestQA||graphicsQA||mushroomQA||bowQA||skillsQA));
    startupLog.write(noAudio?"Audio skipped (--no-audio)":"Audio initialization complete");
#ifndef EMBERFALL_PLAYER_BUILD
    if(developerLaunch.enabled&&!diagnostic){
        developerKit=developerLaunch.kit;developerSelection=developerLaunch.destination>=0?developerLaunch.destination:0;
        if(developerLaunch.destination>=0&&developerLaunch.error.empty())startDeveloperSession(true);
        else{welcome=false;developerActive=true;intro={};openDeveloperMenu();developerError=developerLaunch.error;}
    }
#endif
    if(smoke||benchmark)welcome=false;
    if(benchmark)game.skillRanks={1,1};
    if(desertBenchmark){game.activateField(WorldLocation::Desert);game.collectedFragments=1;game.dawnQuest=DawnQuest::AtTower;game.player=DesertArrival;game.messageTime=0;}
    if(towerBenchmark){game.enterMageTower();game.dawnQuest=DawnQuest::AtTower;game.messageTime=0;}
    if(worldMotionQA)worldMotionPassed=runWorldMotionQA();
    if(labelsQA)labelsPassed=runMapLabelsQA();
    if(tradeQA)tradePassed=runTradeQA();
    if(shadowQA)shadowPassed=runShadowQA();
    if(desertQA)desertPassed=runDesertQA();
    if(towerArtQA)towerArtPassed=runTowerFurnitureQA();
    if(stairsQA)stairsPassed=runTowerStairsQA();
    if(combatQA)combatPassed=runCriticalHealingQA();
#ifndef EMBERFALL_PLAYER_BUILD
    if(developerQA)developerPassed=runDeveloperQA();
#endif
    if(skillsQA)skillsPassed=runSkillsQA();
    if(towerQA)towerPassed=runTowerQA();
    if(towerFloorsQA)towerFloorsPassed=runTowerFloorsQA();
    if(introQA)introPassed=runIntroQA();
    if(bowQA)bowPassed=runBowQA();
    if(mushroomQA)mushroomPassed=runMushroomQA();
    if(graphicsQA)graphicsPassed=runGraphicsQA();
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
    if(swingQA)swingPassed=runSwordMotionQA();
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
    while(!worldMotionQA&&!labelsQA&&!tradeQA&&!shadowQA&&!desertQA&&!towerArtQA&&!stairsQA&&!combatQA&&!developerQA&&!towerFloorsQA&&!towerQA&&!introQA&&!visualQA&&!hoverQA&&!rollQA&&!audioQA&&!inspectionQA&&!swingQA&&!crowdQA&&!progressionQA&&!controlsQA&&!corpseQA&&!patrolQA&&!environmentQA&&!presentationQA&&!motionQA&&!pointerQA&&!performanceQA&&!refreshQA&&!forestQA&&!graphicsQA&&!mushroomQA&&!bowQA&&!skillsQA&&!WindowShouldClose()){
        double begin=GetTime();double elapsed=begin-previous;previous=begin;
        performance.begin();
        float dt=float(std::clamp(elapsed,0.0,.05));
        updateFramePacing();
        refreshDisplayLayout();
        bool displayChanged=(intro.active()||towerStory.active())?displayAction(IsKeyPressed(KEY_F11),false):displayInput();
        if(displayChanged)refreshDisplayLayout();
        if(smoke||benchmark){
            if(benchmark&&game.allocationRequired){Attributes points{};points[int(Attribute::MagicPower)]=game.attributePoints;game.distribute(points);SkillRanks learned{};learned[0]=game.skillPoints;game.trainSkills(learned);statsOpen=skillsOpen=false;}
            game.update(1.f/60,{},false);game.hp=game.maxHp();
            if(towerBenchmark){
                int floor=std::min(4,frame/180);if(game.towerFloor!=floor){game.towerFloor=floor;game.effects.clear();}
                V next=TowerCenter+V{std::cos(frame*.012f)*2.8f,std::sin(frame*.012f)*2.8f};
                if(!game.sandTrapped()&&game.walkable(next)){game.animation.advance(next.x-game.player.x,next.y-game.player.y,1.f/60,true);game.player=next;}
                game.towerRuneMask=7;mouse={0,0};
            }
            if(desertBenchmark){
                if(frame%100==0)desertRegionsVisited++;
                const V views[]={DesertArrival,DesertOasis+V{0,-5},DesertBossHome+V{0,-4},{2,2},{2,35},{35,2},{69,35},{35,69},{69,69}};
                if(frame==205)if(auto* boss=game.findEnemy(DesertBossId))boss->sandPrisonCd=.2f;
                V center=views[std::min(8,frame/100)];V next=center+V{std::cos(frame*.04f),std::sin(frame*.04f)}*.7f;
                if(!game.sandTrapped()&&game.walkable(next)){game.animation.advance(next.x-game.player.x,next.y-game.player.y,1.f/60,true);game.player=next;}
                if(frame%64==0)game.attack();if(frame%90==0){game.mana=float(game.maxMana());game.spell({0,1});}game.level=100;game.xp=0;
                for(const auto& enemy:game.enemies)if(enemy.alive){Vector2 p=screen(enemy.pos);p.y-=35;if(worldPointerAllowed(p)){mouse=p;break;}}
            }
            if(benchmark&&!towerBenchmark&&!desertBenchmark){
                if(frame==300)game.selectWeapon(1);if(frame==600)game.selectWeapon(0);
                V next{24+std::sin(frame*.004f)*17,24+std::cos(frame*.004f)*17};
                if(!game.rolling())game.animation.advance(next.x-game.player.x,next.y-game.player.y,1.f/60,true);
                game.player=next;
                if(frame%95==0&&!game.bowDrawing)game.startRoll({std::cos(frame*.004f),-std::sin(frame*.004f)});
                if(frame%120==0&&archetype(game.equipment[0].kind)==Kind::Sword&&game.strongCut())benchmarkStrongCuts++;
                if(frame%32==0)game.attack();
                if(archetype(game.equipment[0].kind)==Kind::Bow){
                    V aim{1,0};for(const auto& e:game.enemies)if(e.alive&&distance(e.pos,game.player)<12){aim=e.pos-game.player;break;}
                    if(frame%90==0){game.preparePiercingArrow();game.beginBow(aim);}if(game.bowDrawing)game.aimBow(aim);if(frame%90==45){bool piercing=game.piercingArrowArmed;if(game.releaseBow()&&piercing)benchmarkPiercingShots++;}
                }
                if(frame%90==0){game.mana=float(game.maxMana());V aim{};for(const auto& e:game.enemies)if(e.alive&&distance(e.pos,game.player)<8){aim=e.pos-game.player;break;}game.spell(aim);}
                // Include hover picking and the outline pass in the performance test.
                for(const Enemy& enemy:game.enemies)if(enemy.alive){
                    Vector2 p=screen(enemy.pos);p.y-=enemy.type==0?18:enemy.type==1?36:enemy.type==2?66:92;
                    if(worldPointerAllowed(p)){mouse=p;break;}
                }
            }
        }else if(!displayChanged)input(dt);
        audioSystem.update(dt,game,developerMenuOpen||towerPanelOpen||towerStory.active()||intro.active()||welcome||help||paused||audioOpen||statsOpen||skillsOpen||spellBookOpen||weaponWheelOpen||game.allocationRequired,smoke||benchmark||IsWindowFocused());
        performance.end(RenderStage::Update);
        renderScene();
        if(frame==0)startupLog.write("First scene rendered");
        BeginDrawing();
        presentCanvas();
        if(((smoke||startupSmoke||developerSmoke)&&frame==119)||(!smoke&&!benchmark&&IsKeyPressed(KEY_F12))){
            fs::create_directories("screenshots");Image image=presentedImage();
            ExportImage(image,developerSmoke?"screenshots/emberfall-dev-startup.png":startupSmoke?"screenshots/emberfall-startup.png":"screenshots/emberfall-v37.png");UnloadImage(image);
        }
        double submitted=GetTime();double cpu=submitted-begin;EndDrawing();double ended=GetTime();performance.end(RenderStage::Present,true);finishFramePacing();double shown=GetTime();
        if(frame==0){startupLog.write("First frame presented");startupLog.write(pacingName());if(*sync_present::error())startupLog.write(sync_present::error());}
        if(frameLimit&&frame>60){cpuTimes.push_back(cpu*1000);frameTimes.push_back(elapsed*1000);endTimes.push_back((ended-submitted)*1000);presentTimes.push_back((shown-ended)*1000);}
        frame++;
        if(frameLimit&&frame>=frameLimit)break;
    }
    if(frameLimit&&!cpuTimes.empty()){
        std::sort(cpuTimes.begin(),cpuTimes.end());std::sort(frameTimes.begin(),frameTimes.end());
        std::ofstream f(developerSmoke?"dev-startup-v43.txt":desertBenchmark?"desert-benchmark-v43.txt":towerBenchmark?"tower-benchmark-v43.txt":startupSmoke?"startup-smoke.txt":benchmark?(startFullScreen?"benchmark-v43-fullscreen.txt":large?"benchmark-v43-large.txt":"benchmark-v43.txt"):"smoke-test-v43.txt");
        f<<"Build edition: "<<(PlayerBuild?"player":"developer")<<"\nFrames: "<<frame<<"\nWindow: "<<GetScreenWidth()<<"x"<<GetScreenHeight()
         <<"\nFullscreen: "<<(fullScreen()?"yes":"no")
         <<"\nRefresh rate (Hz): "<<refreshHz<<"\nV-sync interval: "<<swapInterval
         <<"\nPresentation clock: "<<pacingName()
         <<"\nDXGI synchronized presents: "<<sync_present::presentCount()
         <<"\nOccluded presentations: "<<sync_present::occludedCount()
         <<"\nSuppressed native swaps: "<<sync_present::suppressedSwaps()
         <<"\nActual presentation interval: "<<sync_present::lastSyncInterval()
         <<"\nPresentation fallback: "<<sync_present::error()
         <<"\nStrong cuts performed: "<<benchmarkStrongCuts<<"\nPiercing arrows fired: "<<benchmarkPiercingShots
         <<"\nDeveloper mode: "<<developerActive<<"\nDeveloper menu open: "<<developerMenuOpen<<"\nDeveloper destination: "
#ifndef EMBERFALL_PLAYER_BUILD
         <<DeveloperDestinations[developerSelection].id
#else
         <<"unavailable"
#endif
         <<"\nWorld location: "<<(game.inTower()?"mage-tower":game.inDesert()?"desert":"forest")<<"\nBenchmark scenario: "<<(desertBenchmark?"desert":towerBenchmark?"tower":"forest")<<"\nDesert areas visited: "<<desertRegionsVisited
         <<"\nSand prisons cast: "<<(game.inDesert()&&game.findEnemy(DesertBossId)?game.findEnemy(DesertBossId)->sandPrisonCasts:0)
         <<"\nTower floor: "<<game.towerFloor+1<<"\nIntro active: "<<intro.active()<<"\nDeveloper error: "<<developerError
         <<"\nAudio device opened: "<<(audioSystem.available?"yes":"no")
         <<"\nNative canvas: "<<canvas.texture.width<<"x"<<canvas.texture.height
         <<"\nDesert background buffer: "<<desert_art::mountainBackdrop.texture.width<<'x'<<desert_art::mountainBackdrop.texture.height
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
    startupLog.write("Shutting down");audioSystem.shutdown();
    sync_present::shutdown();
    world_shadows::unload();actor_art::finishShadowFrame();fallen_art::unload();unloadHoverArt();unloadSkillArt();unloadTowerRoomArt();tower_stairs::unload();tower_furniture::unload();desert_art::unload();sand_prison::unload();
    for(auto& t:propArt)UnloadRenderTexture(t);
    UnloadRenderTexture(terrain);UnloadTexture(minimapTerrain);if(canvas.id)UnloadRenderTexture(canvas);if(worldCanvas.id)UnloadRenderTexture(worldCanvas);
    typography.clear();CloseWindow();return worldMotionPassed&&labelsPassed&&swingPassed&&tradePassed&&shadowPassed&&desertPassed&&towerArtPassed&&stairsPassed&&combatPassed&&developerPassed&&towerFloorsPassed&&towerPassed&&introPassed&&audioPassed&&inspectionPassed&&crowdPassed&&progressionPassed&&controlsPassed&&corpsePassed&&patrolPassed&&environmentPassed&&presentationPassed&&motionPassed&&pointerPassed&&performancePassed&&refreshPassed&&forestPassed&&graphicsPassed&&mushroomPassed&&bowPassed&&skillsPassed?0:2;
}




