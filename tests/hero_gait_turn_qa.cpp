// Silent captures of the real character renderer. Run with a fresh output folder.
#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
#include "hero_gait_sole.hpp"
#include <chrono>
#include <iostream>

namespace {
constexpr int View = 216, AnchorY = 184;
constexpr int Turns = 360, Samples = 24;
const Rectangle HeadCrop{70, 10, 76, 76};
const Rectangle FigureCrop{24, 0, 168, 196};

struct Sheet {
    int width,height,columns;
    Image image;
    Sheet(int w,int h,int count,int cols):width(w),height(h),columns(cols),
        image(GenImageColor(w*cols,h*((count+cols-1)/cols),BLANK)) {}
    ~Sheet(){UnloadImage(image);}
    void put(int index,const Image& frame,Rectangle crop){
        const auto* from=static_cast<const Color*>(frame.data);
        auto* to=static_cast<Color*>(image.data);
        for(int y=0;y<height;y++){
            const int top=(index/columns)*height;
            std::copy_n(from+(int(crop.y)+y)*frame.width+int(crop.x),width,
                        to+(top+y)*image.width+(index%columns)*width);
        }
    }
    void save(const fs::path& path){if(!ExportImage(image,path.string().c_str()))throw std::runtime_error("PNG export failed");}
};

template<class Draw>Image capture(RenderTexture2D& surface,Draw draw){
    actor_art::finishShadowFrame();
    BeginTextureMode(surface);ClearBackground(BLANK);draw();EndTextureMode();
    Image image=LoadImageFromTexture(surface.texture);ImageFlipVertical(&image);
    ImageFormat(&image,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);return image;
}
void orient(AnimationState& a,float angle){
    a.facingInitialized=a.motionInitialized=true;
    a.direction=Direction((8+int(std::round(angle/(PI/4)))%8)%8);
    a.targetAngle=a.facingAngle=angle;
}
void captureTurns(const fs::path& root,RenderTexture2D& surface){
    // Test the painted face in isolation as well as the same face seated in
    // actual headgear. A fixed idle clock isolates discontinuities in yaw.
    const struct {const char* name;Kind kind;} outfits[]={
        {"face",Kind::None},{"bare",Kind::None},{"helmet",Kind::Helmet},
        {"hood",Kind::WolfHood},{"closed",Kind::RivetedGreathelm}};
    Game actor;
    for(auto outfit:outfits){
        actor.equipment[1]=outfit.kind==Kind::None?Item{}:item(outfit.kind);
        for(int sign:{1,-1}){
            Sheet sheet(76,76,Turns,20);
            for(int frame=0;frame<Turns;frame++){
                float angle=sign*frame*(2*PI/Turns);
                Image image=capture(surface,[&]{
                    if(std::string(outfit.name)=="face"){
                        actor_art::Rig face(108,48,2.65f,Direction::E);
                        face.setSmoothSurfaces();face.setFacing(angle);
                        face.paintedHead({},false,false);face.draw();
                    }else{
                        orient(actor.animation,angle);
                        actor_art::drawPlayer(actor,108,AnchorY,2);
                    }
                });
                sheet.put(frame,image,HeadCrop);UnloadImage(image);
            }
            sheet.save(root/(std::string("turn-")+outfit.name+(sign>0?"-cw.png":"-ccw.png")));
        }
    }
}
void captureGaits(const fs::path& root,RenderTexture2D& surface){
    Game actor;
    for(bool run:{false,true}){
        // Keep separate sheets per direction, small enough for reading on a laptop.
        for(int d=0;d<8;d++){
            Sheet sheet(168,196,Samples,6);
            for(int i=0;i<Samples;i++){
                auto& a=actor.animation;
                orient(a,d*PI/4);a.moving=true;a.sprinting=run;a.moveBlend=1;
                a.runBlend=a.swordStow=run?1:0;a.phase=float(i)/Samples;
                Image frame=capture(surface,[&]{actor_art::drawPlayer(actor,108,166,1.5f);});
                sheet.put(i,frame,FigureCrop);UnloadImage(frame);
            }
            sheet.save(root/((run?std::string("run-"):std::string("walk-"))+std::to_string(d)+".png"));
        }
    }
    actor.animation={};
    // Advance real input through sprint, stow, draw, stop and a direction reversal.
    constexpr int steps=120;
    Sheet preview(168,196,steps,8);
    for(int i=0;i<steps;i++){
        const bool run=i>=38&&i<87;
        const float speed=(run?WalkSpeed*RunMultiplier:WalkSpeed)/60;
        const float angle=i<38?PI/4:i<67?0:i<90?PI:i<108?PI*.5f:PI/4;
        const V dir=normalized(V{std::cos(angle)*.5f+std::sin(angle),std::sin(angle)-std::cos(angle)*.5f});
        actor.animation.advance(dir.x*speed,dir.y*speed,1.f/60,run);
        Image frame=capture(surface,[&]{actor_art::drawPlayer(actor,108,166,1.5f);});
        preview.put(i,frame,FigureCrop);UnloadImage(frame);
    }
    preview.save(root/"moving-turn.png");
}
void capturePaths(const fs::path& root,RenderTexture2D& surface){
    Game actor;
    constexpr float degrees[]={-.5f,.5f,33.3f,65.6f,92.4f,221.2f};
    for(int mode=0;mode<3;mode++){
        actor_art::gpuHeroDraw=mode!=2;
        actor_art::fastPacking=mode==0;
        Sheet sheet(76,76,int(std::size(degrees)),int(std::size(degrees)));
        for(int i=0;i<int(std::size(degrees));i++){
            orient(actor.animation,degrees[i]*PI/180);
            Image frame=capture(surface,[&]{actor_art::drawPlayer(actor,108,AnchorY,2);});
            sheet.put(i,frame,HeadCrop);UnloadImage(frame);
        }
        sheet.save(root/("renderer-"+std::to_string(mode)+".png"));
    }
    actor_art::gpuHeroDraw=actor_art::fastPacking=true;
}
void captureSprintTransition(const fs::path& root,RenderTexture2D& surface){
    using namespace hero_motion;
    Game actor;
    actor.player={45,45};
    actor.props.clear();actor.enemies.clear();
    const V direction=normalized(V{1,-1});
    constexpr float dt=1.f/60;
    for(int i=0;i<45;i++)actor.update(dt,direction,false,false);

    constexpr int steps=150;
    // The extended sword passes beyond the walking pose during the draw.
    const Rectangle crop{0,0,float(View),196};
    Sheet sheet(View,196,steps,10);
    std::ofstream log(root/"shift.csv");
    log<<"frame,shift,phase,run,travel,stow,lean,hand_l,hand_r,sole_l,sole_r\n";
    for(int frame=0;frame<steps;frame++){
        const bool sprint=frame>=25&&frame<102;
        const V start=actor.player;
        actor.update(dt,direction,false,sprint);
        const auto& a=actor.animation;
        const auto pose=evaluate(a.phase,a.moveBlend,a.runBlend,a.idleTime);
        const auto forward=pose.vector(data::Spine,{0,0,1});
        auto handTravel=[&](int hand,int shoulder){return (pose.world[hand].p-pose.world[shoulder].p).y;};
        log<<frame<<','<<sprint<<','<<a.phase<<','<<a.runBlend<<','<<distance(start,actor.player)<<','<<a.swordStow
           <<','<<std::atan2(forward.y,forward.z)*180/PI
           <<','<<handTravel(data::HandL,data::UpperArmL)<<','<<handTravel(data::HandR,data::UpperArmR)
           <<','<<walk_checks::soleHeight(pose,data::FootL)<<','<<walk_checks::soleHeight(pose,data::FootR)<<'\n';
        Image image=capture(surface,[&]{actor_art::drawPlayer(actor,96,166,1.5f);});
        sheet.put(frame,image,crop);UnloadImage(image);
    }
    sheet.save(root/"shift.png");
}
void joints(const fs::path& root){
    using namespace hero_motion;
    std::ofstream log(root/"joints.csv");log<<"run,phase,bone,x,y,z\n";
    std::ofstream soles(root/"soles.csv");soles<<"run,phase,left,right\n";
    for(int run=0;run<=1;run++)for(int step=0;step<=120;step++){
        const auto p=evaluate(step/120.f,1,float(run),0);
        soles<<run<<','<<step<<','<<walk_checks::soleHeight(p,data::FootL)<<','
             <<walk_checks::soleHeight(p,data::FootR)<<'\n';
        for(int b=0;b<data::BoneCount;b++){
            const auto v=p.world[b].p;
            log<<run<<','<<step<<','<<b<<','<<v.x<<','<<v.y<<','<<v.z<<'\n';
        }
    }
}
}
int main(int argc,char** argv){
    const bool sprintOnly=argc==4&&std::string(argv[3])=="--sprint";
    if((argc!=3&&!sprintOnly)||std::string(argv[1])!="--no-audio")return 3;
    const fs::path root=fs::absolute(argv[2]);
    fs::create_directories(root);
    SetTraceLogLevel(LOG_ERROR);SetConfigFlags(FLAG_WINDOW_HIDDEN);sync_present::useOpenGL();
    InitWindow(View,View,"Emberfall gait --no-audio");
    if(!IsWindowReady())return 4;
    audioSystem.init(false,false);hero_paint::load();actor_art::prepareCreatureBatch();
    if(audioSystem.available||!hero_paint::headsLoaded)return 5;
    RenderTexture2D surface=LoadRenderTexture(View,View);
    const auto start=std::chrono::steady_clock::now();
    if(sprintOnly)captureSprintTransition(root,surface);
    else{captureTurns(root,surface);captureGaits(root,surface);capturePaths(root,surface);joints(root);}
    const double seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
    std::ofstream(root/"render-seconds.txt")<<seconds<<'\n';
    UnloadRenderTexture(surface);hero_paint::unload();audioSystem.shutdown();sync_present::shutdown();CloseWindow();
    return 0;
}
