#pragma once
#include <map>

class Typography {
    std::map<std::pair<int,int>,Font> fonts;
    float pixelScale=1;
    Font& font(float logicalSize,bool title) {
        int pixels=std::max(10,int(std::round(logicalSize*pixelScale)));
        int style=title?2:logicalSize<=16?1:0;
        auto key=std::make_pair(style,pixels);
        auto it=fonts.find(key);
        if(it==fonts.end()){
            std::vector<int> codes;
            for(int c=32;c<384;c++)codes.push_back(c);
            for(int c:{8212,8211,8594})codes.push_back(c);
            const char* path=title?"assets/Cinzel.ttf":style==1?"assets/Lato-Bold.ttf":"assets/Lato-Regular.ttf";
            Font f=LoadFontEx(path,pixels,codes.data(),int(codes.size()));
            // Coverage is antialiased in the glyph itself. Sample it one-to-one;
            // do not blur it a second time with bilinear texture filtering.
            SetTextureFilter(f.texture,TEXTURE_FILTER_POINT);
            it=fonts.emplace(key,f).first;
        }
        return it->second;
    }
public:
    void clear(){for(auto& entry:fonts)UnloadFont(entry.second);fonts.clear();}
    void setScale(float s){
        if(std::abs(s-pixelScale)>.0001f){clear();pixelScale=s;}
        // Font atlas creation and GPU upload happen before active rendering,
        // including sizes first used by combat, tooltips and modal menus.
        if(fonts.empty()){
            for(float size:{14.f,15.f,16.f,17.f,18.f,19.f,20.f,21.f,22.f,24.f})font(size,false);
            for(float size:{20.f,24.f,25.f,28.f,30.f,32.f,34.f,39.f})font(size,true);
        }
    }
    size_t atlasCount()const{return fonts.size();}
    float width(const std::string& text,float size,bool title){
        Font& f=font(size,title);
        return MeasureTextEx(f,text.c_str(),float(f.baseSize),title?1.f:0.f).x/pixelScale;
    }
    void draw(const std::string& text,float x,float y,float size,Color color,bool title){
        Font& f=font(size,title);
        Vector2 p{std::round(x*pixelScale)/pixelScale,std::round(y*pixelScale)/pixelScale};
        DrawTextEx(f,text.c_str(),p,f.baseSize/pixelScale,title?1.f/pixelScale:0.f,color);
    }
};
