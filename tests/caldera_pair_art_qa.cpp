#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
#include "enemy_ability_fixture.hpp"
#include <chrono>

namespace {
constexpr int Width=296,Height=244;
constexpr Vector2 Anchor{Width*.5f,Height-54.f};
using Pixels=std::vector<Color>;
enum class View {Body,Shadow,Outline,Thumbnail,Combat};

Pixels capture(const Enemy& e,RenderTexture2D target,View view,const fs::path& path={}){
    BeginTextureMode(target);
    ClearBackground(BLANK);
    native_world::beginOpaqueFrame();
    if(view==View::Thumbnail){
        drawCorpseThumbnail(e,{112,100,72,44});
    }else if(view==View::Shadow){
        BeginShaderMode(world_shadows::maskShader);
        world_shadows::beginMaskBlend();rlDisableBackfaceCulling();
        if(e.alive){
            volcano_actors::draw(e.type,Anchor.x,Anchor.y,1,e.animation,e.volcanoAction(),
                                 BLANK,nullptr,true,false,false,game.volcanoSpecialPose(e));
        }else world_shadows::calderaCorpseShadow(e,corpseDrawPoint(e,Anchor));
        EndBlendMode();EndShaderMode();rlEnableBackfaceCulling();
    }else{
        const Color edge=view==View::Outline?Gold:BLANK;
        if(e.alive)actor_art::drawEnemy(e,Anchor.x,Anchor.y,edge);
        else corpseArt(e,Anchor.x,Anchor.y,edge);
        if(view==View::Combat){
            const auto origin=screen(e.pos);
            rlPushMatrix();rlTranslatef(Anchor.x-origin.x,Anchor.y-origin.y,0);
            for(const auto& projectile:game.cinders)volcano_art::bolt(projectile);
            enemy_ability_art::air();
            for(const auto& blast:game.explosions)drawExplosion(blast);
            rlPopMatrix();
        }
    }
    native_world::finishOpaqueBackground();
    EndTextureMode();
    Image im=LoadImageFromTexture(target.texture);
    ImageFlipVertical(&im);ImageFormat(&im,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    auto* p=static_cast<const Color*>(im.data);Pixels pixels(p,p+Width*Height);
    if(!path.empty()&&!ExportImage(im,path.string().c_str()))throw std::runtime_error("export failed");
    UnloadImage(im);return pixels;
}
int difference(const Pixels& a,const Pixels& b){
    int changed=0;
    for(size_t i=0;i<a.size();++i){
        const auto x=a[i],y=b[i];
        if(std::max(x.a,y.a)>24 && std::abs(x.r-y.r)+std::abs(x.g-y.g)+std::abs(x.b-y.b)+std::abs(x.a-y.a)>16)++changed;
    }
    return changed;
}
int visible(const Pixels& p){return int(std::count_if(p.begin(),p.end(),[](Color c){return c.a>=96;}));}
struct PaintClarity {double softFraction{},interiorContrast{},headContrast{};};
PaintClarity paintClarity(const Pixels& frame){
    const auto opaque=[](Color c){return c.a>=146;};
    const auto first=std::find_if(frame.begin(),frame.end(),opaque);
    if(first==frame.end())return {};
    const auto last=std::find_if(frame.rbegin(),frame.rend(),opaque);
    const int top=int(first-frame.begin())/Width;
    const int bodyHeight=int(last.base()-frame.begin()-1)/Width-top+1;
    const int headTop=top+int(bodyHeight*.08),headBottom=top+int(bodyHeight*.50);
    int painted=0,soft=0,interior=0,headPixels=0;
    double detail=0,headDetail=0;
    const auto luminance=[](Color p){return .2126*p.r+.7152*p.g+.0722*p.b;};
    for(int y=1;y<Height-1;++y)for(int x=1;x<Width-1;++x){
        const auto* p=&frame[y*Width+x];
        if(p->a<=18)continue;
        ++painted;
        soft+=p->a<235;
        if(std::min({p->a,p[-1].a,p[1].a,p[-Width].a,p[Width].a})<=230)continue;
        ++interior;
        const double contrast=std::abs(4*luminance(*p)-luminance(p[-1])-luminance(p[1])-luminance(p[-Width])-luminance(p[Width]));
        detail+=contrast;
        if(y>=headTop&&y<headBottom){++headPixels;headDetail+=contrast;}
    }
    return {double(soft)/std::max(1,painted),detail/std::max(1,interior),headDetail/std::max(1,headPixels)};
}
int firePixels(const Pixels& frame,Vector2 center,float radius){
    int count=0;
    for(int y=std::max(0,int(center.y-radius));y<=std::min(Height-1,int(center.y+radius));++y){
        for(int x=std::max(0,int(center.x-radius));x<=std::min(Width-1,int(center.x+radius));++x){
            const float dx=x+.5f-center.x,dy=y+.5f-center.y;
            const auto c=frame[y*Width+x];
            if(dx*dx+dy*dy<=radius*radius&&c.a>=100&&c.r>=190&&c.g>=50&&c.b+50<c.r)++count;
        }
    }
    return count;
}
bool picksExtremities(const Enemy& e,const Pixels& frame){
    Vector2 ends[]={{Width,0},{0,0},{0,Height},{0,0}};
    for(int y=1;y<Height-1;++y)for(int x=1;x<Width-1;++x){
        const int i=y*Width+x;
        if(std::min({frame[i].a,frame[i-1].a,frame[i+1].a,frame[i-Width].a,frame[i+Width].a})<205)continue;
        const Vector2 q{x+.5f,y+.5f};
        if(q.x<ends[0].x)ends[0]=q;
        if(q.x>ends[1].x)ends[1]=q;
        if(q.y<ends[2].y)ends[2]=q;
        if(q.y>ends[3].y)ends[3]=q;
    }
    const auto screenOrigin=screen(e.pos);
    for(auto q:ends){
        const auto hit=pickWorld({screenOrigin.x+q.x-Anchor.x,screenOrigin.y+q.y-Anchor.y});
        if(hit.kind!=HoverKind::Enemy||hit.index!=e.id)return false;
    }
    return true;
}
// A whole painting may change by a few color steps as it turns in the light.
// Measure opacity at the outline and the magnitude of the color change inside it.
struct TurnStep {int changed{};float silhouette{},pigment{};};
TurnStep turnStep(const Pixels& a,const Pixels& b){
    int changed=0,shared=0,either=0;
    double color=0;
    for(size_t i=0;i<a.size();++i){
        const auto l=a[i],r=b[i];
        const bool al=l.a>=128,ar=r.a>=128;
        either+=al||ar;
        if(al&&ar){
            ++shared;
            color+=std::abs(l.r-r.r)+std::abs(l.g-r.g)+std::abs(l.b-r.b);
        }
        changed+=std::max(l.a,r.a)>24 &&
            std::abs(l.r-r.r)+std::abs(l.g-r.g)+std::abs(l.b-r.b)+std::abs(l.a-r.a)>16;
    }
    return {changed,float(either-shared)/std::max(either,1),float(color/(3.*std::max(shared,1)))};
}
void face(Enemy& e,int d){
    e.animation={};e.animation.motionInitialized=true;e.animation.facingInitialized=true;
    e.animation.direction=Direction(d);e.animation.facingAngle=e.animation.targetAngle=volcano_actors::facingAngles[d];
}
void ordinary(Enemy& e,float seconds){
    const float windup=meleeWindup(e.type);
    e.windup=std::max(0.f,windup-seconds);
    e.strike=seconds<windup?0:std::max(0.f,windup+meleeRecovery(e.type)-seconds);
}
bool registeredKeysMatch(int type,const volcano_actors::turn_art::Artwork& art){
    const auto& canonical=volcano_actors::atlases[type-VolcanoFirstType];
    if(canonical.cell!=160)return false;
    for(int v=0;v<8;++v)for(int p=0;p<6;++p){
        const auto t=art.tile(4*v,p,0);
        const auto& page=art.pages[t.page];
        const int row=art.authoredPart(v,p);
        for(int y=0;y<160;++y)for(int x=0;x<160;++x){
            const int localX=x-t.ox,localY=y-t.oy;
            int packed=0;
            if(localX>=0&&localY>=0&&localX<t.w&&localY<t.h)
                packed=page.alpha[(t.y+localY)*page.color.width+t.x+localX];
            const int original=canonical.alpha[(row*160+y)*canonical.art.width+160*v+x];
            if(std::abs(packed-original)>8)return false;
        }
    }
    return true;
}
}

int main(int argc,char** argv){
    if((argc!=4&&argc!=5)||std::string(argv[1])!="--no-audio"||std::string(argv[2])!="--background-test")return 2;
    const bool baseline=argc==5&&std::string(argv[4])=="--baseline";
    fs::path out=argv[3];fs::create_directories(out/"frames");
    SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN|FLAG_WINDOW_ALWAYS_RUN);
    sync_present::useOpenGL();InitWindow(Width,Height,"Emberfall Caldera QA");
    if(!IsWindowReady()||!IsWindowHidden()||IsAudioDeviceReady())return 3;
    volcano_actors::load();
    caldera_attack_art::load();
    spell_art::load();
    world_shadows::init();
    welcome=false;
    auto target=render_targets::colorOnly(Width,Height);
    if(!baseline)fallen_art::initThumbnails();
    int checks=0,failures=0,frames=0;
    std::ofstream log(out/"checks.txt",std::ios::binary);
    auto check=[&](bool ok,const std::string& why){++checks;failures+=!ok;log<<(ok?"PASS ":"FAIL ")<<why<<'\n';};
    using Clock=std::chrono::steady_clock;
    const auto start=Clock::now();
    if(!baseline&&caldera_puppets::ready(caldera_puppets::Imp)&&caldera_puppets::imp_render::ready()){
        const auto useColorBands=[](caldera_puppets::Artwork& atlas){
            const auto original=atlas.texture;
            Image bands=LoadImageFromTexture(original);
            ImageFormat(&bands,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
            auto* ink=static_cast<Color*>(bands.data);
            for(int y=0;y<bands.height;y++)for(int x=0;x<bands.width;x++){
                const int i=y*bands.width+x;
                const int tile=(y/atlas.height)*atlas.columns+x/atlas.cell;
                const auto alpha=ink[i].a;
                ink[i]=tile%2?Color{0,0,255,alpha}:Color{255,0,0,alpha};
            }
            atlas.texture=LoadTextureFromImage(bands);
            UnloadImage(bands);
            SetTextureFilter(atlas.texture,TEXTURE_FILTER_BILINEAR);
            return original;
        };
        auto& body=caldera_puppets::artwork[caldera_puppets::Imp];
        auto& core=caldera_puppets::impCore;
        const auto bodyTexture=useColorBands(body),coreTexture=useColorBands(core);
        game=enemyAbilityArena(FireImpType);
        auto& actor=game.enemies.front();
        actor.animation.facingInitialized=true;
        actor.animation.motionInitialized=true;
        const float step=2*PI/volcano_actors::turn_art::ImpPaintedDirections;
        for(int dir=0;dir<volcano_actors::turn_art::ImpPaintedDirections;dir++){
            actor.animation.facingAngle=(dir+.5f)*step;
            actor.animation.direction=directionAtScreenAngle(actor.animation.facingAngle);
            const auto paint=capture(actor,target,View::Body);
            const auto classification=[](Color c){
                if(c.a<253)return 0;
                if(c.r>190&&c.b<65)return 1;
                if(c.b>190&&c.r<65)return 2;
                return 3;
            };
            int red=0,blue=0,mixed=0,mixedInterior=0;
            for(int y=1;y<Height-1;++y)for(int x=1;x<Width-1;++x){
                const int color=classification(paint[y*Width+x]);
                if(color==1)++red;
                if(color==2)++blue;
                if(color!=3)continue;
                ++mixed;
                // Antialiasing can mix two pieces at their shared edge. A
                // mix across an opaque patch indicates a ghosted painting.
                bool inside=true;
                for(int dy:{-1,0,1})for(int dx:{-1,0,1})inside&=classification(paint[(y+dy)*Width+x+dx])==3;
                mixedInterior+=inside;
            }
            check(red+blue>500&&mixedInterior<.025f*(red+blue),
                  "imp / no ghosted paint inside the silhouette at direction "+std::to_string(dir)+
                  ", red/blue/mixed/inside "+std::to_string(red)+"/"+std::to_string(blue)+"/"+
                  std::to_string(mixed)+"/"+std::to_string(mixedInterior));
        }
        UnloadTexture(body.texture);body.texture=bodyTexture;
        UnloadTexture(core.texture);core.texture=coreTexture;
    }
    for(int type:{FireImpType,ObsidianSentinelType}){
        game=enemyAbilityArena(type);
        auto& e=game.enemies.front();
        const std::string stem=type==FireImpType?"imp":"sentinel";
        const auto folder=out/"frames"/stem;
        fs::create_directories(folder);
        const float contact=enemy_death::groundContactTime(type),duration=enemy_death::duration(type);
        if(!baseline){
            const auto* artwork=volcano_actors::turn_art::forType(type);
            const bool authored=type==FireImpType;
            const bool newRig=caldera_puppets::ready(caldera_puppets::Imp);
            if(authored){
                check(newRig&&caldera_puppets::solidColor.id,"imp / native cutout layers and hover shader loaded");
                check(caldera_puppets::impCore.texture.id!=0,"imp / synchronized head and torso atlas loaded");
                check(caldera_puppets::imp_render::ready(),"imp / whole pose shader and RGB detail loaded");
                check(!artwork,"imp / old interpolated atlases are skipped with the new rig installed");
            }else{
                check(artwork&&volcano_actors::turn_art::filtered(),stem+" / real directional atlas and shader loaded");
                check(artwork&&registeredKeysMatch(type,*artwork),stem+" / packed UVs match all 48 canonical painted masks");
            }
            face(e,0);e.animation.moveBlend=1;e.animation.phase=.31f;
            auto previous=capture(e,target,View::Body);
            int changed=0,worst=0,worstPigment=0,alphaMismatch=0,sharpenedViews=0,combinedHeadViews=0;
            float silhouette=0,pigment=0;
            PaintClarity clarity;
            std::array<double,360> headContrast{};
            for(int degree=1;degree<=360;++degree){
                e.animation.facingAngle=degree*DEG2RAD;
                e.animation.direction=directionAtScreenAngle(e.animation.facingAngle);
                auto next=capture(e,target,View::Body);
                const auto sharp=paintClarity(next);
                if(authored){
                    const auto pose=caldera_puppets::fireImp(e.animation,-1);
                    const auto end=pose.pieces.begin()+pose.count;
                    const int core=(degree%360)+caldera_puppets::FirstCore;
                    const auto coreParts=std::count_if(pose.pieces.begin(),end,[](const auto& p){
                        return p.tile<4||p.tile>=caldera_puppets::FirstCore;
                    });
                    combinedHeadViews+=coreParts==1&&std::any_of(pose.pieces.begin(),end,[&](const auto& p){return p.tile==core;});
                    caldera_puppets::imp_render::sharpen=false;
                    const auto original=capture(e,target,View::Body);
                    caldera_puppets::imp_render::sharpen=true;
                    bool colorChanged=false;
                    for(size_t i=0;i<next.size();++i){
                        const Color a=original[i],b=next[i];
                        alphaMismatch+=a.a!=b.a;
                        colorChanged|=a.a>=230&&(a.r!=b.r||a.g!=b.g||a.b!=b.b);
                    }
                    sharpenedViews+=colorChanged;
                }
                headContrast[degree%360]=sharp.headContrast;
                clarity.softFraction+=sharp.softFraction/360;
                clarity.interiorContrast+=sharp.interiorContrast/360;
                const auto delta=turnStep(previous,next);
                changed+=delta.changed>0;
                if(delta.silhouette>silhouette){silhouette=delta.silhouette;worst=degree;}
                if(delta.pigment>pigment){pigment=delta.pigment;worstPigment=degree;}
                previous=std::move(next);
            }
            const bool turns=changed>=350&&silhouette<.20f&&pigment<(authored?21.f:18.f);
            check(turns,stem+" / one-degree samples, visible direction changes="+
                  std::to_string(changed)+" max silhouette="+std::to_string(silhouette)+" at degree="+std::to_string(worst)+
                  " max interior RGB delta="+std::to_string(pigment)+" at degree="+std::to_string(worstPigment));
            check((authored?newRig:artwork&&artwork->flow.id) &&
                  clarity.softFraction<(type==FireImpType?.24:.16) && clarity.interiorContrast>22,
                  stem+" / crisp moving silhouette and painted detail, translucent fraction="+
                  std::to_string(clarity.softFraction)+" interior contrast="+std::to_string(clarity.interiorContrast));
            if(type==FireImpType){
                check(combinedHeadViews==360,"imp / head and torso stay on one shared heading for the full turn");
                check(alphaMismatch==0&&sharpenedViews==360,"imp / full turn preserves exact alpha and sharpens paint; changed-alpha pixels="+
                      std::to_string(alphaMismatch)+", sharpened views="+std::to_string(sharpenedViews));
                const auto weakest=*std::min_element(headContrast.begin(),headContrast.end());
                check(weakest>18,"imp / eye, muzzle and horn contrast in the weakest painted direction="+std::to_string(weakest));
            }
            for(int deg=worst-1;deg<=worst;++deg){
                e.animation.facingAngle=deg*DEG2RAD;
                e.animation.direction=directionAtScreenAngle(e.animation.facingAngle);
                capture(e,target,View::Body,folder/("turn-"+std::to_string(deg)+".png"));
            }
        }
        for(int d=0;d<8;++d){
            face(e,d);e.alive=true;e.deathElapsed=-1;e.windup=e.strike=0;e.abilityAge=-1;
            e.animation.idleTime=.18f;
            const std::string name=stem+" / "+std::to_string(d)+" / ";
            const auto neutral=capture(e,target,View::Body,folder/("idle-"+std::to_string(d)+".png"));
            if(baseline)continue;
            const int solid=visible(neutral);
            check(solid>350,name+"body rendered");
            bool clipped=false;
            for(int y=0;y<Height;++y)for(int x=0;x<Width;++x)
                if((x<2||y<2||x>Width-3||y>Height-3)&&neutral[y*Width+x].a>32)clipped=true;
            check(!clipped,name+"full silhouette visible");
            check(visible(capture(e,target,View::Shadow))>60,name+"shadow covers ground");
            check(visible(capture(e,target,View::Outline))>solid,name+"hover outline enlarges the silhouette");
            int mismatch=0,inside=0;
            for(int y=4;y<Height-4;y+=5)for(int x=4;x<Width-4;x+=5){
                bool opaque=true,empty=true;
                for(int dy:{-2,0,2})for(int dx:{-2,0,2}){
                    int a=neutral[(y+dy)*Width+x+dx].a;opaque&=a>225;empty&=a<12;
                }
                if(!opaque&&!empty)continue;
                const Vector2 point{x+.5f,y+.5f};
                bool hit=actor_art::drawEnemy(e,Anchor.x,Anchor.y,BLANK,&point);
                mismatch+=opaque?!hit:hit;inside+=opaque;
            }
            check(inside>5&&mismatch==0,name+"opaque and empty regions agree with mouse picking");
            check(picksExtremities(e,neutral),name+"map hover selects the outer painted silhouette");
            const auto health=enemyHealthBar(e,Anchor);
            bool clear=true;
            for(int y=int(health.y)-1;y<=int(health.y+health.height)+1;++y)
                for(int x=int(health.x);x<int(health.x+health.width);++x)clear&=neutral[y*Width+x].a<96;
            check(clear,name+"the health bar clears the head and horns");
            e.animation.idleTime+=.48f;
            check(difference(neutral,capture(e,target,View::Body))>12,name+"breathing changes the painted pose");
            e.animation.moveBlend=1;e.animation.phase=0;
            const auto walkStart=capture(e,target,View::Body);e.animation.phase=.28f;
            check(difference(walkStart,capture(e,target,View::Body))>60,name+"opposing phases of the stride");
            e.animation.phase=1;
            check(difference(walkStart,capture(e,target,View::Body))<3,name+"closed walk loop");
            for(int step=0;step<8;++step){
                e.animation.phase=step/8.f;
                capture(e,target,View::Body,folder/("gait-"+std::to_string(d)+"-"+std::to_string(step)+".png"));
            }
            e.animation.phase=.11f;e.animation.moveBlend=.1f;ordinary(e,.20f);
            const auto wind=capture(e,target,View::Body);ordinary(e,meleeWindup(type));
            const auto impact=capture(e,target,View::Body);
            check(std::abs(e.volcanoAction()-.45f)<.0001f&&difference(wind,impact)>60,name+"regular attack hits on key .45");
            if(type==FireImpType){
                int corePixels[2]{};
                int index=0;
                for(float charge:{.12f,.76f}){
                    ordinary(e,meleeWindup(type)*charge);
                    const auto hands=volcano_actors::impCastHands(e,e.volcanoAction());
                    const auto posed=capture(e,target,View::Body);
                    const Vector2 center{Anchor.x+hands.center.x,Anchor.y+hands.center.y};
                    corePixels[index++]=firePixels(posed,center,12);
                    if(charge>.5f){
                        for(auto hand:{hands.left,hands.right}){
                            bool painted=false;
                            for(int y=-3;y<=3;++y)for(int x=-3;x<=3;++x){
                                const Vector2 point{Anchor.x+hand.x+x,Anchor.y+hand.y+y};
                                painted|=actor_art::drawEnemy(e,Anchor.x,Anchor.y,BLANK,&point);
                            }
                            check(painted,name+"casting hand matches the painted anatomy");
                        }
                        const auto mid=Vector2Scale(Vector2Add(hands.left,hands.right),.5f);
                        check(Vector2Distance(mid,hands.center)<1.f,name+"fire core is held between the hands");
                        int fire=0,paint=0;
                        for(int y=0;y<Height;++y)for(int x=0;x<Width;++x){
                            const Vector2 point{x+.5f-Anchor.x,y+.5f-Anchor.y};
                            if(std::min(Vector2DistanceSqr(point,hands.left),Vector2DistanceSqr(point,hands.right))>=81.f)continue;
                            const Color pixel=posed[y*Width+x];
                            if(pixel.a<100)continue;
                            ++paint;fire+=pixel.r>180&&pixel.r>pixel.b+84&&pixel.g>60;
                        }
                        check(paint>=85&&fire<.45f*paint,name+"painted fingers remain legible around the charged ember");
                    }
                }
                check(corePixels[1]>corePixels[0]+20,name+"ball visibly grows during the windup, core pixels="+
                      std::to_string(corePixels[0])+" to "+std::to_string(corePixels[1]));
                ordinary(e,meleeWindup(type));
                game.launchCinder(e);
                const auto origin=screen(e.pos),p=volcano_art::boltAnchor(game.cinders.back());
                const auto hands=volcano_actors::impCastHands(e,e.volcanoAction());
                check(std::hypot(p.x-origin.x-hands.center.x,p.y-origin.y-hands.center.y)<1.f,
                      name+"projectile leaves from the ball's last position");
                game.cinders.clear();
                auto previous=volcano_actors::impCastHands(e,0);
                float worst=0;
                for(int tick=1;tick<=int(std::ceil(60*(meleeWindup(type)+meleeRecovery(type))));++tick){
                    ordinary(e,tick/60.f);
                    const auto next=volcano_actors::impCastHands(e,e.volcanoAction());
                    worst=std::max({worst,Vector2Distance(next.left,previous.left),Vector2Distance(next.right,previous.right)});
                    previous=next;
                }
                check(worst<7.5f,name+"no hand skips through the gathering, release or recovery; peak pixel step="+std::to_string(worst));
                ordinary(e,meleeWindup(type));
            }
            if(type==ObsidianSentinelType){
                const auto& ability=enemyAbilityProfile(type);
                e.abilityAge=ability.windup*.5f;
                const auto anchor=volcano_actors::sentinelBladePoint(e);
                bool touches=false;
                for(int y=-3;y<=3;++y)for(int x=-3;x<=3;++x){
                    Vector2 q{Anchor.x+anchor.x+x,Anchor.y+anchor.y+y};
                    touches|=actor_art::drawEnemy(e,Anchor.x,Anchor.y,BLANK,&q);
                }
                check(touches,name+"cleave warning follows the painted blade");
                const auto warning=capture(e,target,View::Body);e.abilityAge=ability.windup;
                check(difference(warning,capture(e,target,View::Body))>100,name+"cleave telegraph and impact are distinct");
                check(picksExtremities(e,capture(e,target,View::Body)),name+"map hover follows the extended cleave silhouette");
            }
            const auto live=capture(e,target,View::Body);
            game.beginEnemyDeath(e,e.flash);e.alive=false;e.windup=e.strike=0;e.abilityAge=-1;
            check(difference(live,capture(e,target,View::Body))<3,name+"fall starts in the captured attacking pose");
            for(float side:{-1.f,1.f}){
                e.deathDirection={side,-side};e.deathElapsed=contact*.55f;
                const auto falling=capture(e,target,View::Body);e.deathElapsed=std::nextafter(contact,0.f);
                const auto justBefore=capture(e,target,View::Body);
                e.deathElapsed=contact;const auto rest=capture(e,target,View::Body);
                check(visible(rest)>280&&difference(falling,rest)>90,name+"fall reaches a substantial resting silhouette");
                const int seam=difference(justBefore,rest);
                check(seam<8,name+"last fall frame meets the actual corpse; changed pixels="+std::to_string(seam));
                if(type==FireImpType){
                    saves::Archive stored(false);stored(e);
                    saves::Archive input(true,std::move(stored.bytes));
                    Enemy loaded;input(loaded);
                    const auto pixels=capture(loaded,target,View::Body);
                    check(input.at==input.bytes.size()&&std::memcmp(rest.data(),pixels.data(),rest.size()*sizeof(Color))==0,
                          name+"the loaded corpse retains its paint, pose and anchor");
                }
                const auto base=volcano_actors::groundFootprint(e);
                const auto floor=corpseDrawPoint(e,Anchor);
                const Rectangle patch{floor.x+base.x,floor.y+base.y,base.width,base.height};
                const auto groundShadow=capture(e,target,View::Shadow);
                int grounded=0;
                for(int y=0;y<Height;++y)for(int x=0;x<Width;++x)
                    if(rest[y*Width+x].a>155&&groundShadow[y*Width+x].r>155&&CheckCollisionPointRec({x+.5f,y+.5f},patch))++grounded;
                check(grounded>30,name+"visible body overlaps its rendered ground-contact shadow");
                e.deathElapsed=duration+.2f;
                check(difference(rest,capture(e,target,View::Body))==0,name+"unchanged final ground pose");
                check(visible(capture(e,target,View::Thumbnail))>60,name+"corpse thumbnail draws in the nearby loot HUD");
            }
        }
        if(baseline)continue;
        for(int d=0;d<8;++d){
            face(e,d);e.alive=true;e.strike=0;e.windup=0;e.abilityAge=-1;e.deathElapsed=-1;
            const float next=d==7?2*PI:volcano_actors::facingAngles[d+1];
            e.animation.facingAngle=.55f*volcano_actors::facingAngles[d]+.45f*next;
            e.animation.phase=.3f;e.animation.moveBlend=.65f;
            ordinary(e,.26f);
            const auto live=capture(e,target,View::Body);
            game.beginEnemyDeath(e,0);e.alive=false;e.deathElapsed=.00001f;
            const auto early=capture(e,target,View::Body);const int delta=difference(live,early);
            check(delta<visible(live)*.15f,stem+" / fractional heading keeps the captured attack when struck: "+
                  std::to_string(d)+" changed="+std::to_string(delta));
        }
        face(e,type==FireImpType?1:3);e.alive=true;e.deathElapsed=-1;e.windup=e.strike=0;e.abilityAge=-1;
        auto frame=[&](const char* clip,int index,View view=View::Body){
            char file[64];std::snprintf(file,sizeof(file),"%s-%03d.png",clip,index);
            capture(e,target,view,folder/file);++frames;
        };
        for(int f=0;f<24;++f){e.animation.moveBlend=1;e.animation.phase=f/24.f;frame("walk",f);}
        e.animation.moveBlend=0;e.animation.phase=0;
        const int attackFrames=int(std::ceil(30*(meleeWindup(type)+meleeRecovery(type))))+1;
        bool thrown=false;
        for(int f=0;f<attackFrames;++f){
            const float time=f/30.f;
            ordinary(e,time);
            if(type==FireImpType){
                if(!thrown&&time>=meleeWindup(type)){
                    game.launchCinder(e);thrown=true;
                }else if(thrown)game.updateVolcanoAttacks(1.f/30,game.player,false,0);
            }
            frame("attack",f,View::Combat);
        }
        game.cinders.clear();
        e.windup=e.strike=0;
        if(type==ObsidianSentinelType){
            const auto& ability=enemyAbilityProfile(type);
            e.abilityDirection={0,1};
            for(int f=0;f<=48;++f){e.abilityAge=f*ability.duration()/48;frame("cleave",f,View::Combat);}
            e.abilityAge=-1;
        }
        game.beginEnemyDeath(e,0);e.alive=false;e.deathDirection={1,-1};
        for(int f=0;f<24;++f){e.deathElapsed=contact*std::min(1.f,f/20.f);frame("fall",f);}
    }
    if(!baseline){
        game=enemyAbilityArena(FireImpType);
        auto& caster=game.enemies.front();
        game.player=caster.pos+V{2.15f,-.7f};
        face(caster,0);ordinary(caster,meleeWindup(FireImpType));
        game.launchCinder(caster);
        const auto origin=screen(caster.pos);
        const auto hpBefore=game.hp;
        const auto firstCore=volcano_art::boltAnchor(game.cinders.front());
        const V direction=normalized(game.player-caster.pos);
        auto expectedImpact=screen(game.player-direction*.42f);expectedImpact.y-=39;
        const auto flightAxis=Vector2Normalize(Vector2Subtract(expectedImpact,firstCore));
        Vector2 latestCore{},previous=firstCore;
        float reversal=0;
        for(int i=0;i<120&&!game.cinders.empty();++i){
            const auto current=volcano_art::boltAnchor(game.cinders.front());
            reversal=std::max(reversal,-Vector2DotProduct(Vector2Subtract(current,previous),flightAxis));
            previous=current;
            latestCore=Vector2Add(Anchor,Vector2Subtract(current,origin));
            game.updateVolcanoAttacks(1.f/120,game.player,false,0);
        }
        check(reversal<.5f,"imp / ball moves towards the torso without reversing after the throw");
        check(game.hp<hpBefore&&game.cinders.empty()&&game.explosions.size()==1,
              "imp / one blast replaces the projectile at actual player contact");
        if(!game.explosions.empty()){
            std::vector<Explosion> justHit;
            justHit.swap(game.explosions);
            const auto noBlast=capture(caster,target,View::Combat);
            justHit.swap(game.explosions);
            const auto blast=capture(caster,target,View::Combat,out/"fireball-contact.png");
            check(firePixels(blast,latestCore,20)>firePixels(noBlast,latestCore,20)+20,
                  "imp / the first explosion lights the last flight position at torso height");
            const auto chest=Vector2Add(Anchor,Vector2Subtract(expectedImpact,origin));
            check(Vector2Distance(chest,latestCore)<9&&firePixels(blast,chest,12)>30,
                  "imp / projectile and fireburst both reach the collision side of the player's chest");
        }
    }
    const double elapsed=std::chrono::duration<double>(Clock::now()-start).count();
    check(!IsAudioDeviceReady()&&IsWindowHidden(),"silent hidden renderer");
    std::ofstream(out/"summary.json")<<"{\"checks\":"<<checks<<",\"failures\":"<<failures<<",\"preview_frames\":"<<frames
       <<",\"render_readback_export_seconds\":"<<elapsed<<"}\n";
    UnloadRenderTexture(target);spell_art::unload();caldera_attack_art::unload();volcano_actors::unload();CloseWindow();return failures?1:0;
}
