#pragma once

namespace tower_paint {
constexpr int FirstStair=23;

// Swap the color art during startup. The decoded image remains the source for
// picking and hover outlines, so they use exactly the alpha drawn on screen.
inline void apply(Texture2D texture,Image& source,int index){
    const std::string path=TextFormat("assets/sprites/tower-paint/%02d.png",index);
    if(!texture.id||!FileExists(path.c_str()))return;
    Image painted=LoadImage(path.c_str());
    if(!painted.data)return;
    if(painted.width!=texture.width||painted.height!=texture.height){
        UnloadImage(painted);
        return;
    }
    ImageFormat(&painted,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    if(painted.format!=PIXELFORMAT_UNCOMPRESSED_R8G8B8A8){
        UnloadImage(painted);
        return;
    }
    // FBO color attachments are drawn with an inverted source rectangle.
    ImageFlipVertical(&painted);
    UpdateTexture(texture,painted.data);
    ImageFlipVertical(&painted);
    UnloadImage(source);
    source=painted;
}
}
