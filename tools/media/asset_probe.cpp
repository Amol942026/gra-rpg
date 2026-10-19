// Independent format/adapter check using the exact raylib 5.5 used by Emberfall.
// Never opens an audio device or window, and requires explicit silent mode.
#include <iostream>
#include <string>
#include <vector>
#include "runtime_atlas.hpp"
int main(int argc,char**argv){
    if(argc<3 || std::string(argv[1])!="--no-audio")return 2;
    SetTraceLogLevel(LOG_NONE);
    int images=0,audio=0;bool ok=true;
    ember_media::SpriteClip clip;
    ok&=clip.frameAt(0)==0 && clip.frameAt(1)==0 && clip.frameAt(.5)==30;
    ok&=clip.frameAt(-1)==0 && clip.source(59).width==128;
    for(int i=2;i<argc;i++){
        std::string path=argv[i];auto dot=path.find_last_of('.');
        std::string ext=dot==std::string::npos?"":path.substr(dot);
        if(ext==".png"){
            Image image=LoadImage(path.c_str());
            bool valid=IsImageValid(image) && image.width>0 && image.height>0;
            ok&=valid;if(valid){images++;UnloadImage(image);}
        }else if(ext==".wav"||ext==".ogg"){
            Wave wave=LoadWave(path.c_str());
            bool valid=IsWaveValid(wave) && wave.frameCount>0 && wave.sampleRate==48000 && wave.channels==2;
            ok&=valid;if(valid){audio++;UnloadWave(wave);}
        }else ok=false;
    }
    std::cout<<"{\"status\":\""<<(ok?"PASS":"FAIL")<<"\",\"images\":"<<images
             <<",\"audio_files\":"<<audio<<",\"audio_device_opened\":false,\"game_rebuilt\":false}\n";
    return ok?0:1;
}
