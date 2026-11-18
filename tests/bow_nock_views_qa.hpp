#pragma once
#include <chrono>

// Follow real press/hold/release input, including the last arrow in the bag.
// Save the same pixels used by the silhouette checks for visual review.
template<class Check>void verifyBowNockingViews(Check check){
    const fs::path output="qa-hero-art/bow-nocking";
    fs::create_directories(output);
    constexpr int width=230,height=204;
    const int ready=int(std::round(BowNockDuration*60));
    const int release=ready+30,last=release+15;
    const std::array<int,8> samples={0,ready/3,ready/2,(2*ready)/3,ready-2,ready,release-1,release+4};
    auto stage=LoadRenderTexture(width,height);
    Image contact=GenImageColor(width*8,height*int(samples.size()),BLANK);
    const auto start=std::chrono::steady_clock::now();
    for(int outfit=0;outfit<2;outfit++)for(int d=0;d<8;d++){
        Game actor;
        actor.props.clear();actor.enemies.clear();
        actor.player={24,24};
        actor.equipment[0]=item(outfit?Kind::RiftBow:Kind::Bow);
        if(outfit){actor.equipment[1]=item(Kind::Helmet);actor.equipment[2]=item(Kind::Shield);}
        for(auto& slot:actor.bag)if(slot.kind==Kind::Arrow)slot={};
        const int initial=outfit?20:1;
        actor.bag[0]=item(Kind::Arrow,0,initial);
        const float angle=d*PI/4;
        const V aim=normalized(V{.5f*std::cos(angle)+std::sin(angle),std::sin(angle)-.5f*std::cos(angle)});
        actor.animation.face(aim.x,aim.y);
        check(actor.beginBow(aim),"a real bow press begins the reach");
        bool inside=true,notEmpty=true,timing=true;
        std::vector<int> silhouettes;
        std::vector<unsigned char> restMask;
        for(int f=0;f<=last;f++){
            if(f==release)check(actor.releaseBow(false),"release spawns the arrow after it is nocked");
            if(f<ready)timing&=actor.bowCharge()==0&&actor.arrows.empty()&&actor.ammoCount()==initial;
            if(f>ready+2&&f<release)timing&=actor.bowCharge()>0&&actor.arrows.empty()&&actor.ammoCount()==initial;
            if(f>=release)timing&=actor.ammoCount()==initial-1;
            actor_art::finishShadowFrame();
            BeginTextureMode(stage);ClearBackground(BLANK);
            actor_art::drawPlayer(actor,width*.5f,176,1.54f);
            EndTextureMode();
            Image im=LoadImageFromTexture(stage.texture);
            ImageFlipVertical(&im);ImageFormat(&im,PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
            auto* pix=static_cast<const Color*>(im.data);
            int solid=0;
            for(int y=0;y<height;y++)for(int x=0;x<width;x++)if(pix[y*width+x].a>32){
                solid++;inside&=x>0&&y>0&&x+1<width&&y+1<height;
            }
            notEmpty&=solid>800;
            if(f<ready)silhouettes.push_back(solid);
            if(d==2||d==6){
                if(f==0){
                    restMask.resize(width*height);
                    for(int i=0;i<width*height;i++)restMask[i]=pix[i].a>80;
                }
                if(f==ready/2){
                    // Face-on, the archer's right shoulder is on the viewer's
                    // left. Seen from behind, the same reach is on the right.
                    int left=0,right=0;
                    for(int y=45;y<80;y++)for(int dx=15;dx<55;dx++){
                        int l=y*width+width/2-1-dx,r=y*width+width/2+dx;
                        left+=pix[l].a>80&&!restMask[l];
                        right+=pix[r].a>80&&!restMask[r];
                    }
                    check((d==2?left-right:right-left)>30,"the archer reaches over the anatomical right shoulder on screen");
                }
            }
            if(!outfit){
                auto it=std::find(samples.begin(),samples.end(),f);
                if(it!=samples.end()){
                    const int row=int(it-samples.begin());
                    for(int y=0;y<height;y++)
                        std::copy_n(pix+y*width,width,static_cast<Color*>(contact.data)+(row*height+y)*contact.width+d*width);
                }
                if(d==1||(d>=3&&d<=5)||d==7){
                    const auto dir=output/std::to_string(d);
                    if(f==0)fs::create_directories(dir);
                    if(!ExportImage(im,(dir/(std::to_string(f)+".png")).string().c_str()))throw std::runtime_error("Bow frame export failed");
                }
            }
            UnloadImage(im);
            actor.update(1.f/60,{},false);
        }
        auto ends=std::minmax_element(silhouettes.begin(),silhouettes.end());
        check(inside&&notEmpty,"the head, quiver, carried arrow and limbs fit each viewing direction");
        check(*ends.second-*ends.first>35,"the player visibly handles the arrow while bow tension stays zero");
        check(timing,"the visible retrieval, draw and single inventory consumption occur in order");
    }
    check(ExportImage(contact,(output/"directions.png").string().c_str()),"eight preparations rendered to the contact sheet");
    UnloadImage(contact);UnloadRenderTexture(stage);
    std::ofstream(output/"render-seconds.txt")<<std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
}
