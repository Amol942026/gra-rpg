#include "raylib.h"
#include <string>
#include <vector>
static std::vector<std::string> drawnText;
static bool captureText=false;
void traceText(Font,const char*,Vector2,float,float,Color);
#define DrawTextEx traceText
#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
#undef DrawTextEx

void traceText(Font font,const char* value,Vector2 p,float size,float spacing,Color color){
    if(captureText)drawnText.emplace_back(value);
    DrawTextEx(font,value,p,size,spacing,color);
}

int main(int argc,char** argv){
    if((argc!=3&&argc!=4)||std::string(argv[1])!="--no-audio")return 3;
    const fs::path output=fs::absolute(argv[2]);
    const fs::path reference=argc==4?fs::absolute(argv[3]):fs::path{};
    if(!fs::create_directory(output))return 4;
    fs::current_path(fs::absolute(argv[0]).parent_path());
    SetTraceLogLevel(LOG_ERROR);
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    sync_present::useOpenGL();InitWindow(W,H,"Emberfall tower paint --no-audio");
    if(!IsWindowReady())return 5;
    audioSystem.init(false,false);
    makeArt();fallen_art::init();desert_art::make();sand_prison::make();initHoverArt();
    makeSkillArt();makeTowerRoomArt();tower_stairs::make();tower_furniture::make();prepareCanvas(1);
    int failures=0,checks=0;
    std::ofstream log(output/"checks.txt");
    auto check=[&](bool ok,const std::string& message){
        checks++;failures+=!ok;
        log<<(ok?"PASS ":"FAIL ")<<message<<'\n';
    };
    for(int i=0;i<tower_furniture::SpriteCount+2;i++){
        Texture2D texture=i<tower_furniture::SpriteCount?tower_furniture::sprites[i].art.texture:
            tower_stairs::art[i-tower_furniture::SpriteCount].texture;
        std::string name=TextFormat("%02d.png",i);
        Image current=LoadImageFromTexture(texture);ImageFlipVertical(&current);
        check(ExportImage(current,(output/name).string().c_str()),"export "+name);
        if(!reference.empty()){
            Image before=LoadImage((reference/name).string().c_str());
            Image paint=LoadImage(("assets/sprites/tower-paint/"+name).c_str());
            bool dimensions=current.width==before.width&&current.height==before.height&&
                current.width==paint.width&&current.height==paint.height;
            check(dimensions,"original coordinates and dimensions "+name);
            if(dimensions){
                Color* a=LoadImageColors(current);Color* b=LoadImageColors(before);Color* c=LoadImageColors(paint);
                int alphaErrors=0,paintErrors=0,changed=0;
                for(int p=0;p<current.width*current.height;p++){
                    alphaErrors+=a[p].a!=b[p].a;
                    paintErrors+=a[p].r!=c[p].r||a[p].g!=c[p].g||a[p].b!=c[p].b||a[p].a!=c[p].a;
                    changed+=a[p].a>210&&(a[p].r!=b[p].r||a[p].g!=b[p].g||a[p].b!=b[p].b);
                }
                check(alphaErrors==0&&paintErrors==0&&changed>40,"painted texture with original alpha "+name);
                UnloadImageColors(a);UnloadImageColors(b);UnloadImageColors(c);
            }
            UnloadImage(before);UnloadImage(paint);
        }
        UnloadImage(current);
    }
    auto reset=[](int floor){
        game=Game{};intro={};towerStory={};clearTransientUi();invalidatePausedWorld();
        welcome=paused=false;game.enterMageTower();game.dawnQuest=DawnQuest::AtTower;
        game.towerFloor=floor;game.player=TowerArrival;game.messageTime=0;
        game.time=2;mouse={0,0};
    };
    auto screenshot=[&](std::string name){
        drawnText.clear();captureText=true;renderScene();captureText=false;
        saveTexture(canvas,(output/(name+".png")).string());
    };
    auto hasText=[](const std::string& text){
        return std::any_of(drawnText.begin(),drawnText.end(),[&](const auto& value){return value.find(text)!=std::string::npos;});
    };
    const Vector2 center{450,510};
    RenderTexture2D wallTarget=LoadRenderTexture(900,550);
    for(bool balcony:{false,true}){
        BeginTextureMode(wallTarget);ClearBackground(BLANK);
        const bool loaded=drawTowerWall(center,balcony);
        EndTextureMode();
        check(loaded,"shared tower masonry is loaded");
        Image wall=LoadImageFromTexture(wallTarget.texture);ImageFlipVertical(&wall);
        auto solid=[&](float x,float height){
            return GetImageColor(wall,int(center.x+x),int(center.y+towerWallFoot(x)-height)).a==255;
        };
        for(int i=-2;i<=(balcony?0:2);i++){
            float x=i*162.f;
            check(solid(x-63,125)&&solid(x+63,125)&&solid(x,5)&&solid(x,254),
                  "stone surrounds window "+std::to_string(i+3));
        }
        check(solid(200,125)!=balcony,"only the observatory opens onto the balcony");
        UnloadImage(wall);
    }
    UnloadRenderTexture(wallTarget);
    for(int floor=0;floor<TowerFloorCount;floor++){
        reset(floor);game.player={10.8f,9};screenshot(TextFormat("floor-%d",floor+1));
        check(!hasText("SCHODY W GÓRĘ")&&!hasText("SCHODY W DÓŁ"),"no stair captions on floor "+std::to_string(floor+1));
        for(int direction:{1,-1}){
            reset(floor);
            if(!game.stairPresent(direction))continue;
            game.player=towerStairApproach(direction);
            check(nearestTowerInteraction()&&game.towerFloor==floor+direction,"stair interaction remains available");
        }
    }
    reset(MerchantFloor);game.player=MerchantPosition+V{-1,0};
    check(nearestTowerInteraction(),"Miron opens his shop");screenshot("miron-shop");
    check(!hasText("Monety w plecaku też")&&hasText("MIRON"),"shop without its old footer");
    check(!audioSystem.available,"audio device remains closed");
    std::ofstream report(output/"report.json");
    report<<"{\"status\":\""<<(failures?"FAIL":"PASS")<<"\",\"checks\":"<<checks
          <<",\"failures\":"<<failures<<",\"audio_device_opened\":false,\"window_hidden\":true}\n";
    audioSystem.shutdown();sync_present::shutdown();CloseWindow();
    return failures?2:0;
}
