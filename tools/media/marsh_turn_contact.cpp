// Bake ground contact from the warped alpha silhouette, including the reveal edge.
// Offline only. The PNG decoder does not open a window or an audio device.
#include <raylib.h>
#include "../../src/generated/marsh_turn_frames.hpp"
#include "../../src/entity_info.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
namespace fs=std::filesystem;
namespace frames=marsh_turn_frames;
constexpr int Grid=32, Steps=16, Poses=64;
struct Heading {int view;bool flipped;};
constexpr Heading headings[]={{2,true},{3,true},{4,false},{3,false},{2,false},{1,false},{0,false},{1,true}};
struct ImageData {
    int width,height;
    std::vector<Color> pixels;
    explicit ImageData(const fs::path& path,int w,int h):width(w),height(h){
        Image image=LoadImage(path.string().c_str());
        if(!image.data||image.width!=w||image.height!=h)throw std::runtime_error(path.string());
        Color* data=LoadImageColors(image);
        pixels.assign(data,data+w*h);
        UnloadImageColors(data);UnloadImage(image);
    }
};
struct Sample {float x,y,threshold;};
struct Tile {
    int width,height;
    std::vector<unsigned char> alpha;
    Tile(const ImageData& im,int pose,int w,int h,bool flipped):width(w),height(h),alpha(w*h){
        for(int y=0;y<h;y++)for(int x=0;x<w;x++)
            alpha[y*w+x]=im.pixels[(pose/8*h+y)*im.width+pose%8*w+(flipped?w-1-x:x)].a;
    }
    float at(float x,float y)const{
        if(x<.5f||x>width-.5f||y<.5f||y>height-.5f)return 0;
        x-=.5f;y-=.5f;int a=std::min(int(x),width-2),b=std::min(int(y),height-2);
        float tx=x-a,ty=y-b;const auto* p=alpha.data()+b*width+a;
        return (1-ty)*((1-tx)*p[0]+tx*p[1])+ty*((1-tx)*p[width]+tx*p[width+1]);
    }
};
struct Pose {
    const Tile& a;const Tile& b;
    std::array<Sample,Grid*Grid> field;
    float sx,sy;
    Pose(const Tile& first,const Tile& second,const ImageData& map,int pose,int sector):
        a(first),b(second),sx(float(Grid-1)/a.width),sy(float(Grid-1)/a.height){
        for(int y=0;y<Grid;y++)for(int x=0;x<Grid;x++){
            auto c=map.pixels[(sector*Grid+y)*map.width+pose*Grid+x];
            field[y*Grid+x]={(int(c.r)-128)*.75f,(int(c.g)-128)*.75f,c.b/255.f};
        }
    }
    Sample vector(float x,float y)const{
        x=std::clamp(x*sx,0.f,float(Grid-1));y=std::clamp(y*sy,0.f,float(Grid-1));
        int ix=std::min(int(x),Grid-2),iy=std::min(int(y),Grid-2);
        float u=x-ix,v=y-iy,tl=(1-u)*(1-v),tr=u*(1-v),bl=(1-u)*v,br=u*v;
        const auto* f=field.data()+iy*Grid+ix;
        return {tl*f[0].x+tr*f[1].x+bl*f[Grid].x+br*f[Grid+1].x,
                tl*f[0].y+tr*f[1].y+bl*f[Grid].y+br*f[Grid+1].y,
                tl*f[0].threshold+tr*f[1].threshold+bl*f[Grid].threshold+br*f[Grid+1].threshold};
    }
    float coverage(float x,float y,float t)const{
        if(t==0)return a.at(x,y);
        float qx=x,qy=y;
        for(int i=0;i<3;i++){
            const auto m=vector(qx,qy);
            qx=x-(t-.5f)*m.x;qy=y-(t-.5f)*m.y;
        }
        const auto m=vector(qx,qy);
        float weight=std::clamp((t-m.threshold+.013f)/.026f,0.f,1.f);
        weight=weight*weight*(3-2*weight);
        float out=weight<1?(1-weight)*a.at(x-t*m.x,y-t*m.y):0.f;
        if(weight>0)out+=weight*b.at(x+(1-t)*m.x,y+(1-t)*m.y);
        return out;
    }
};
struct Foot {int x,y,rx;};
using Row=std::array<std::int8_t,7>;

float body_floor(const Pose& pose,float t,int ax,int ay,float scale){
    // The playfield samples this creature at its own world scale. A single
    // translucent tip can vanish between two screen pixels when the camera
    // moves. Follow a patch of visible mass, in three subpixel positions.
    int measurements[3]{};
    const int width=int(std::ceil(pose.a.width*scale))+3;
    for(int phase=0;phase<3;phase++){
        std::vector<float> before(width);
        bool found=false;
        for(int y=int(std::ceil((pose.a.height-ay)*scale))+1;y>=-int(ay*scale)&&!found;y--){
            const float sampleY=ay+(y+.5f)/scale;
            float last=0,peak=0,mass=0;
            for(int x=0;x<width;x++){
                const float sampleX=ax+(x-width/2+.5f+.27f*(phase-1))/scale;
                const float alpha=pose.coverage(sampleX,sampleY,t);
                // Two adjacent columns or a continuing thick filament.
                peak=std::max(peak,std::max(last+alpha,before[x]*.7f+alpha));
                mass+=alpha;last=alpha;before[x]=alpha;
            }
            if((peak>=220&&mass>=260)||(peak>=135&&mass>=530)){
                measurements[phase]=y;found=true;
            }
        }
        if(!found)throw std::runtime_error("No visible body mass");
    }
    std::sort(std::begin(measurements),std::end(measurements));
    return measurements[1]/scale;
}

constexpr int Directions=8*Steps;
int index(int pose,int direction){return (direction/Steps*Poses+pose)*Steps+direction%Steps;}
int adjacent_pose(int pose,int step){
    int clip=pose/16,phase=pose%16;
    return clip*16+(clip==0?(phase+step+16)%16:std::clamp(phase+step,0,15));
}
void settle(std::vector<Row>& rows,const std::vector<float>& floors,bool reedLight){
    std::vector<float> correction(rows.size()),stable(rows.size());
    for(int p=0;p<Poses;p++)for(int a=0;a<Directions;a++){
        const int j=a%Steps;
        const float t=float(j)/Steps;
        const float bearing=(1-t)*floors[index(p,a-j)]+t*floors[index(p,(a-j+Steps)%Directions)];
        const float limit=float(std::min(8,2*std::min(j,Steps-j)));
        const int key=index(p,a);
        float target=bearing-floors[key];
        if(reedLight){
            // The light has one trailing reed, often under one screen pixel
            // wide. Do not push it into the peat just to plant a thicker leaf.
            // A tiny allowance between pixel centers avoids reacting to noise.
            target=std::min(target,float(rows[key][0])+1.5f);
        }
        correction[key]=std::clamp(target,-limit,limit);
    }
    // Reject one-angle and one-frame artifacts. Both envelopes are Lipschitz
    // on the heading/phase grid, so their mean obeys the same speed limit.
    for(int p=0;p<Poses;p++)for(int a=0;a<Directions;a++){
        const int k=index(p,a);
        std::array<float,5> n{correction[k],correction[index(p,(a+1)%Directions)],
            correction[index(p,(a+Directions-1)%Directions)],correction[index(adjacent_pose(p,-1),a)],
            correction[index(adjacent_pose(p,1),a)]};
        std::sort(n.begin(),n.end());
        stable[k]=a%Steps==0?0.f:.65f*n[2]+.35f*correction[k];
    }
    std::array<bool,Poses> neutral{};
    for(int p=0;p<Poses;p++){
        neutral[p]=true;
        for(int a=0;a<Directions;a++)
            if(floors[index(p,a)]!=floors[index(0,a)]||rows[index(p,a)]!=rows[index(0,a)]){
                neutral[p]=false;break;
            }
    }
    for(int a=0;a<Directions;a++)for(int p=1;p<Poses;p++)
        if(neutral[p])stable[index(p,a)]=stable[index(0,a)];
    auto low=stable,high=stable;
    for(int pass=0;pass<9;pass++){
        auto lastLow=low,lastHigh=high;
        for(int p=0;p<Poses;p++)for(int a=0;a<Directions;a++){
            const int k=index(p,a);
            for(auto n:{index(p,(a+1)%Directions),index(p,(a+Directions-1)%Directions),
                        index(adjacent_pose(p,-1),a),index(adjacent_pose(p,1),a)}){
                low[k]=std::min(low[k],lastLow[n]+2);
                high[k]=std::max(high[k],lastHigh[n]-2);
            }
        }
        for(int a=0;a<Directions;a++){
            int ref=index(0,a);
            for(int p=1;p<Poses;p++)if(neutral[p]){
                low[ref]=std::min(low[ref],low[index(p,a)]);
                high[ref]=std::max(high[ref],high[index(p,a)]);
            }
            for(int p=1;p<Poses;p++)if(neutral[p]){
                low[index(p,a)]=low[ref];high[index(p,a)]=high[ref];
            }
        }
    }
    for(size_t i=0;i<rows.size();i++){
        auto& row=rows[i];
        const int dy=int(std::lround((low[i]+high[i])*.5f));
        row[2]+=dy-row[0];row[5]+=dy-row[0];row[0]=std::int8_t(dy);
    }
}
int center_x(const Tile& tile){
    int left=tile.width,right=0;
    for(int y=0;y<tile.height;y++)for(int x=0;x<tile.width;x++)
        if(tile.alpha[y*tile.width+x]>=92){left=std::min(left,x);right=std::max(right,x);}
    return (left+right+1)/2;
}
Foot support(const std::vector<int>& bottom,int left,int right,int low,int ax,int ay,int correction,bool light){
    if(low<0)return {0,0,0};
    int length=0,end=left,run=0;
    for(int x=left;x<right;x++){
        run=bottom[x]>=low-2?run+1:0;
        if(run>length){length=run;end=x;}
    }
    int x=end-length/2;
    int ground=bottom[x]-ay+correction;
    int radius=std::clamp(int(std::lround(length*.58f)),2,11);
    return {x-ax,ground,light&&ground < -9?0:radius};
}
Row contacts(const Pose& pose,float t,int ax,int ay,int split,bool light){
    int w=pose.a.width,h=pose.a.height;
    std::vector<int> bottom(w,-1);
    int low[2]={-1,-1};
    int visible=-1;
    for(int y=h-1;y>=0;y--){
        for(int x=0;x<w;x++){
            if(bottom[x]!=-1)continue;
            const auto alpha=pose.coverage(x+.5f,y+.5f,t);
            if(alpha>=48&&visible<0)visible=y;
            if(alpha<92)continue;
            bottom[x]=y;
            int side=x>=split;
            low[side]=std::max(low[side],y);
        }
        if(std::min(low[0],low[1])>=y+2)break;
        if(visible>=0&&y<visible-40)break;
    }
    if(visible<0)throw std::runtime_error("Empty warped silhouette");
    int correction=t==0?0:ay-visible;
    auto l=support(bottom,0,split,low[0],ax,ay,correction,light);
    auto r=support(bottom,split,w,low[1],ax,ay,correction,light);
    if(!l.rx&&!r.rx){
        auto& bearing=l.y>=r.y?l:r;bearing.rx=2;
    }
    int values[]={correction,l.x,l.y,l.rx,r.x,r.y,r.rx};
    Row result;
    for(int i=0;i<7;i++){
        if(std::abs(values[i])>127)throw std::runtime_error("Contact out of byte range");
        result[i]=static_cast<std::int8_t>(values[i]);
    }
    return result;
}
}

int main(int argc,char** argv){
    if(argc!=4||std::string(argv[1])!="--no-audio")return 1;
    try{
        SetTraceLogLevel(LOG_WARNING);
        fs::path pack=argv[2],output=argv[3];
        std::ofstream out(output,std::ios::binary|std::ios::trunc);
        out.exceptions(std::ios::badbit|std::ios::failbit);
        out.write("MWC1",4);
        for(int s=0;s<4;s++){
            int w=frames::CellW[s],h=frames::CellH[s];
            std::vector<ImageData> pages;
            for(int v=0;v<5;v++)pages.emplace_back(pack/("turn-"+std::to_string(s)+"-"+std::to_string(v)+".png"),8*w,8*h);
            ImageData map(pack/("turn-warp-"+std::to_string(s)+".png"),Grid*64,Grid*8);
            std::vector<Row> rows(8*Poses*Steps);
            std::vector<float> floors(rows.size());
            float scale=ember::enemyInfo(ember::PeatToad+s).visualHeight*.92f/frames::RestHeight[s];
            for(int k=0;k<8;k++)for(int frame=0;frame<Poses;frame++){
                auto va=headings[k],vb=headings[(k+1)%8];
                Tile a(pages[va.view],frame,w,h,va.flipped), b(pages[vb.view],frame,w,h,vb.flipped);
                Pose pose(a,b,map,frame,k);
                int centerA=center_x(a),centerB=center_x(b);
                for(int j=0;j<Steps;j++){
                    float t=float(j)/Steps;
                    int split=std::clamp(int(std::lround((1-t)*centerA+t*centerB)),1,w-1);
                    const int key=index(frame,k*Steps+j);
                    rows[key]=contacts(pose,t,frames::AnchorX[s],frames::AnchorY[s],split,s==2);
                    floors[key]=body_floor(pose,t,frames::AnchorX[s],frames::AnchorY[s],scale);
                }
            }
            settle(rows,floors,s==2);
            int maxShift=0;
            for(const auto& row:rows){
                maxShift=std::max(maxShift,std::abs(int(row[0])));
                out.write(reinterpret_cast<const char*>(row.data()),row.size());
            }
            std::cout<<"species "<<s<<" max partial ground correction "<<maxShift<<" px"<<std::endl;
        }
        return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<std::endl;return 2;}
}
