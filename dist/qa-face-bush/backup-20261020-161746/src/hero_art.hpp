#pragma once

// Included inside actor_art after Rig. Every view and pointer query uses this
// same articulated model, including equipment, bow poses and roll transforms.
inline void gearInlay(Rig& rig,const GearAppearance& look,P3 center,float width,float height,bool back=false){
    if(!look.present())return;
    auto span=equipment_paint_data::spans[int(look.value.kind)];
    for(int i=0;i<span.count;i++){
        auto s=equipment_paint_data::strokes[span.start+i];
        float left=(s.x-8)*width/16,right=(s.x+s.width-8)*width/16,top=(8-s.y)*height/16,bottom=(7-s.y)*height/16;
        P3 a=center+P3{left,0,top},b=center+P3{right,0,top},c=center+P3{right,0,bottom},d=center+P3{left,0,bottom};
        if(back)rig.face(d,c,b,a,equipment_art::color(look.paint.accent));
        else rig.face(a,b,c,d,equipment_art::color(look.paint.accent));
    }
}
inline void buildHero(Rig& rig,float lift,float stride,float cycle,float tuck,
                      bool helmet,bool shield,Kind weapon,bool flash,bool rolling,bool striking,
                      float strikeProgress,float bowDraw,int quiverArrows,float run,bool empowered,const AnimationState& state,float rollProgress,const HeroAppearance& appearance){
    rig.reserve(1800);
    rig.setSmoothSurfaces();
    const auto& held=appearance[0];const auto& headwear=appearance[1];const auto& ward=appearance[2];
    const auto& armor=appearance[3];const auto& boots=appearance[4];const auto& jewel=appearance[5];
    auto paint=[&](unsigned color){return flash?Text:equipment_art::color(color);};
    const bool plated=armor.shape==Kind::Armor,cape=armor.shape==Kind::Cloak||!appearance.explicitGear;
    const bool fur=armor.value.kind==Kind::WolfCloak||armor.value.kind==Kind::Cloak||armor.value.kind==Kind::BoarArmor;
    const bool robe=armor.value.kind==Kind::SandRobe;
    const Color skin=flash?Text:rgb(213,177,139),skinLight=flash?Text:rgb(237,206,172);
    const Color metal=plated?paint(armor.paint.primary):paint(0x657f79),edge=plated?shade(metal,1.3f):paint(0x9bb5aa);
    const Color cloth=armor.present()?paint(armor.shape==Kind::Cloak?armor.paint.primary:armor.paint.secondary):paint(0x495e5a),clothLight=shade(cloth,1.25f);
    const Color trim=armor.present()?paint(armor.paint.trim):paint(0xa49370);
    const Color weaponMetal=held.present()?paint(held.paint.primary):edge,weaponTrim=held.present()?paint(held.paint.trim):Gold;
    const Color weaponGrip=held.present()?paint(held.paint.secondary):paint(0x60452e);
    const Color leather=flash?Text:rgb(95,65,45),pants=flash?Text:rgb(60,74,66);
    bool archer=weapon==Kind::Bow;float drawCharge=std::max(0.f,bowDraw);
    bool sheathed=weapon==Kind::Sword&&!striking&&(run>.1f||rolling);
    float moving=state.motionInitialized?state.moveBlend:(state.moving?1.f:0.f);
    auto pose=hero_motion::evaluate(state.phase,moving,bowDraw>=0?0:state.runBlend,state.idleTime,striking?strikeProgress:-1,empowered,rollProgress);
    if(archer&&bowDraw>=0&&!rolling){
        for(int side=0;side<2;side++){
            int upper=side?hero_motion::data::UpperArmR:hero_motion::data::UpperArmL;
            int fore=side?hero_motion::data::ForearmR:hero_motion::data::ForearmL;
            int handBone=side?hero_motion::data::HandR:hero_motion::data::HandL;
            auto sh=pose.world[upper].p;
            hero_motion::Vec target=side?hero_motion::Vec{4-12*drawCharge,16-23*drawCharge,47}:hero_motion::Vec{-10,27,47};
            auto arm=sword_pose::solveArm({sh.x,sh.y,sh.z},{target.x,target.y,target.z},side?1:-1);
            hero_motion::Vec elbow{arm.elbow.r,arm.elbow.f,arm.elbow.z},hand{arm.hand.r,arm.hand.f,arm.hand.z};
            hero_motion::attach(pose,upper,sh,elbow,hero_motion::restHead(fore));
            hero_motion::attach(pose,fore,elbow,hand,hero_motion::restHead(handBone));
            pose.world[handBone]={hand,pose.world[hero_motion::data::Spine].q};
        }
        pose.bind();
    }
    auto rest=[](int bone){auto p=hero_motion::restHead(bone);return P3{p.x,p.y,p.z};};
    auto world=[](hero_motion::Vec p){return P3{p.x,p.y,p.z};};
    lift=0;stride*=moving;
    auto clothMotion=hero_secondary::evaluate(state.phase,moving,state.runBlend,state.idleTime,striking?strikeProgress:-1,empowered,rollProgress);
    rig.setSkeleton(&pose);rig.setBone(-2);
    // Krita-painted panels conform to the existing joints. Item-specific
    // silhouettes, inlays and materials remain independent of this paint.
    auto panel=[&](P3 center,float width,float height,int cell,float radiusR,float radiusZ,Color tint=WHITE,bool back=false){
        if(flash)return;
        constexpr int cols=8,rows=6;
        auto point=[&](float u,float v){
            float r=(u-.5f)*width,z=(.5f-v)*height;
            float front=center.f*std::sqrt(std::max(.035f,1-r*r/(radiusR*radiusR)-z*z/(radiusZ*radiusZ)));
            return P3{center.r+r,front+(back?-.48f:.48f),center.z+z};
        };
        for(int y=0;y<rows;y++)for(int x=0;x<cols;x++){
            float u=x/float(cols),v=y/float(rows),uu=(x+1)/float(cols),vv=(y+1)/float(rows);
            rig.decal(point(u,v),point(uu,v),point(uu,vv),point(u,vv),cell,u,v,uu,vv,flash?Text:tint,back);
        }
    };
    // Cloth hangs in a curved sheet; its scalloped edge and folds move with
    // the walking cycle, while the roll rotates the entire sheet with the body.
    auto capePoint=[&](float u,float down){
        float width=12+6*down,r=(u*2-1)*width;
        float fold=std::cos(u*5*PI)*(.4f+down*1.2f);
        float sway=std::sin(cycle-down*2+u*3)*std::abs(stride)*down*(1.7f+run*1.3f);
        return P3{r+sway+clothMotion.capeSway*.32f*down*down,-10-down*(3+run*9)-fold-clothMotion.capeLift*.4f*down,58+lift-down*(45-run*7-tuck*15)+std::sin(u*4*PI)*down*1.2f+clothMotion.capeLift*.18f*down};
    };
    if(cape){for(int row=0;row<7;row++)for(int col=0;col<12;col++){
        float a=col/12.f,b=(col+1)/12.f,top=row/7.f,bottom=(row+1)/7.f;
        P3 p=capePoint(a,top),q=capePoint(b,top),r=capePoint(b,bottom),s=capePoint(a,bottom);
        float light=.77f+.22f*std::cos((a+b)*2.5f*PI);
        rig.face(p,q,r,s,cloth,light);rig.face(s,r,q,p,cloth,light*.85f);
    }
    for(int i=0;i<24;i++)rig.tapered(capePoint(i/24.f,1),capePoint((i+1)/24.f,1),.65f,.65f,trim);
    for(float side:{0.f,1.f})for(int i=0;i<12;i++)rig.tapered(capePoint(side,i/12.f),capePoint(side,(i+1)/12.f),.7f,.7f,clothLight);
    }
    for(int side=0;side<2;side++){
        int thigh=side?hero_motion::data::ThighR:hero_motion::data::ThighL;
        int shin=side?hero_motion::data::ShinR:hero_motion::data::ShinL;
        int ankle=side?hero_motion::data::FootR:hero_motion::data::FootL;
        P3 hip=rest(thigh),knee=rest(shin),foot=rest(ankle);
        rig.setBone(thigh);
        rig.tapered(knee,hip,8,9,pants,1.15f);rig.rounded(knee,{8,8,9},pants);
        Color bootColor=boots.present()?paint(boots.paint.primary):pants;
        Color bootTrim=boots.present()?paint(boots.paint.trim):leather;
        rig.setBone(shin);rig.tapered(foot,knee,7.5f,8,boots.present()?bootColor:pants,.92f);
        if(boots.present())rig.rounded(knee+P3{0,3,0},{7,4,8},bootTrim);
        if(boots.present()){
            panel({foot.r,4.6f,foot.z+11},7.5f,15,5,5,11,bootTrim);
            gearInlay(rig,boots,foot+P3{0,4,12},6,9);
            if(boots.value.kind==Kind::CobraBoots)for(int i=0;i<3;i++)for(float s:{-1.f,1.f})rig.tapered(foot+P3{0,4.8f,7+i*4.f},foot+P3{s*3.6f,4,9+i*4.f},.8f,.6f,bootTrim);
            if(boots.value.kind==Kind::BanditBoots)for(int i=0;i<3;i++)rig.rounded(foot+P3{0,0,6+i*5.f},{9,9,1.5f},paint(boots.paint.secondary));
        }
        rig.setBone(ankle);
        rig.rounded(foot+P3{0,3,-1},{10,17,8},boots.present()?bootColor:leather);
        rig.rounded(foot+P3{0,1,4},{9,10,9},boots.present()?bootColor:leather);
        rig.rounded(foot+P3{0,5,-3},{10,14,2},rgb(37,32,28));
        if(boots.present())rig.tapered(foot+P3{-4,2,5},foot+P3{4,2,5},1.1f,1.1f,bootTrim);
    }
    rig.setBone(hero_motion::data::Pelvis);
    rig.rounded({0,0,31+lift},{24,17,15},cloth);
    rig.setBone(hero_motion::data::Spine);
    rig.rounded({0,0,46+lift},plated?P3{27,18,28}:P3{25,17,27},plated?metal:cloth);
    if(!plated){panel({0,8.6f,46},23,25,3,12.5f,14,clothLight);panel({0,-8.6f,46},23,25,3,12.5f,14,cloth,true);}
    // Curved breastplate: raised rim, overlapping lower plates and a sun clasp.
    if(plated){for(float side:{-1.f,1.f})for(int j=0;j<8;j++){
        float z=36+j*2.5f,zz=z+2.5f,r=side*(10.5f-std::abs(z-46)*.15f),rr=side*(10.5f-std::abs(zz-46)*.15f);
        rig.tapered({r,5.5f,z+lift},{rr,5.5f,zz+lift},.8f,.8f,edge);
    }
    rig.rounded({0,7,39+lift},{20,5,5},shade(metal,.95f));
    rig.rounded({0,7.6f,35+lift},{19,4,4},shade(metal,.85f));
    gearInlay(rig,armor,{0,9.5f,46},12,13);
    gearInlay(rig,armor,{0,-9.5f,46},12,13,true);
    if(armor.value.kind==Kind::ScarabArmor)for(int row=0;row<4;row++)rig.rounded({0,9.2f,37+row*4.f},{19-row*2.f,3,5},row%2?metal:paint(armor.paint.secondary));
    }
    if(robe){
        rig.setBone(hero_motion::data::Pelvis);
        rig.tapered({0,0,34},{0,0,12},23,31,cloth);
        rig.tapered({-7,9,32},{-11,11,13},1.4f,1.4f,trim);rig.tapered({7,9,32},{11,11,13},1.4f,1.4f,trim);
    }
    rig.setBone(hero_motion::data::Pelvis);
    rig.rounded({0,0,32+lift},{25,18,4},leather);
    rig.rounded({0,9,32+lift},{5,2,4},Gold);rig.rounded({0,10,32+lift},{2.8f,1,2},leather);
    rig.setBone(hero_motion::data::Spine);
    rig.tapered({-9,8,54+lift},{9,8,36+lift},2.5f,2,leather);
    rig.rounded({0,9.4f,49+lift},{5,2,6},Gold);rig.rounded({0,10.5f,49+lift},{2,1,3},cloth);
    for(float side:{-1.f,1.f}){
        rig.rounded({side*12,-1,29+lift},{7,9,9},leather);
        rig.rounded({side*12,3.5f,31+lift},{6,2,3},shade(leather,1.25f));
        rig.rounded({side*12,4.5f,30+lift},{1.6f,1,1.6f},Gold);
    }
    rig.setBone(hero_motion::data::Head);
    rig.rounded({0,0,61+lift},{10,10,10},skin);
    // A softer collar and two shoulder plates attach to rounded upper arms.
    rig.setBone(hero_motion::data::Spine);
    rig.rounded({0,-1,57+lift},{28,18,8},cloth);
    if(fur)for(int i=0;i<9;i++)rig.rounded({-12+i*3.f,-1,57-std::abs(i-4)*.65f},{6,21,7},paint(armor.value.kind==Kind::BoarArmor?0xbfa782:0xb2bdb0));
    if(armor.value.kind==Kind::BoarArmor)for(float side:{-1.f,1.f})rig.tapered({side*13,3,52},{side*16,2,60},3,1,paint(armor.paint.accent));
    if(jewel.shape==Kind::Amulet){
        Color chain=paint(jewel.paint.trim);
        rig.tapered({-6,7,58},{0,10.5f,48},.7f,.7f,chain);rig.tapered({6,7,58},{0,10.5f,48},.7f,.7f,chain);
        rig.rounded({0,11,48},{7,3,8},chain);rig.rounded({0,12.4f,48},{4.5f,2,5.5f},paint(jewel.paint.accent));
    }
    if(archer){
        rig.rounded({-7,-17,44+lift},{12,12,31},leather);
        rig.rounded({-7,-17,59+lift},{14,13,4},Gold);
        rig.tapered({8,9,54+lift},{-9,-17,32+lift},2.8f,2,leather);
        for(int i=0;i<std::clamp(quiverArrows,0,3);i++){
            float r=-10+i*3.f,top=73+i%2*5.f+lift;
            rig.tapered({r,-17,45+lift},{r,-17,top},1.1f,1.1f,rgb(182,134,75));
            rig.form({r,-17,top-3},{5,2,6},Text);rig.form({r,-17,top-4},{2,5,6},Gold);
        }
    }
    if(weapon==Kind::Sword){
        // The scabbard hangs from the belt at the left hip. Its body follows
        // the torso, never the pumping hand. A drawn blade leaves it empty.
        rig.setBone(hero_motion::data::Pelvis);
        P3 mouth{-18,-7,37+lift},axis{-.06f,-.44f,-.895991f},tip=mouth+axis*37;
        Color darkLeather=shade(weaponGrip,.62f);
        rig.tapered({-12,0,33+lift},mouth+P3{0,0,-4},2.1f,2.1f,leather);
        rig.tapered({-12,-5,32+lift},mouth+axis*7,2,2,leather);
        rig.tapered(mouth+axis*1.2f,tip,5.7f,3.8f,darkLeather);
        rig.tapered(mouth, mouth+axis*2,6.6f,6.2f,weaponTrim);
        rig.tapered(mouth+axis*34.5f,tip,4.3f,3.8f,metal);
        rig.tapered(mouth+axis*9,mouth+axis*10,5.9f,5.9f,leather);
        rig.tapered(mouth+axis*23,mouth+axis*24,5.2f,5.2f,leather);
        if(sheathed){
            P3 guard=mouth-axis*1.5f,grip=mouth-axis*7.5f;
            rig.tapered(grip,guard,3.5f,3.5f,weaponGrip);
            for(int band=0;band<3;band++)rig.tapered(mouth-axis*(3+band*1.3f),mouth-axis*(3.4f+band*1.3f),3.8f,3.8f,darkLeather);
            rig.tapered(guard+P3{-6,0,0},guard+P3{6,0,0},2.5f,2.5f,weaponTrim);
            rig.rounded(grip-axis*1.2f,{4.5f,4.5f,4.5f},paint(held.paint.accent));
        }
        rig.setBone(hero_motion::data::Spine);
    }
    for(float side:{-1.f,1.f}){
        int upper=side>0?hero_motion::data::UpperArmR:hero_motion::data::UpperArmL;
        int fore=side>0?hero_motion::data::ForearmR:hero_motion::data::ForearmL;
        int handBone=side>0?hero_motion::data::HandR:hero_motion::data::HandL;
        P3 shoulder=rest(upper),elbow=rest(fore),hand=rest(handBone);
        rig.setBone(upper);
        rig.tapered(elbow,shoulder,8,9,cloth);
        rig.rounded(shoulder,plated?P3{13,17,10}:P3{11,14,8},plated?metal:cloth);
        if(plated)rig.rounded(shoulder+P3{0,5,1},{11,5,3},edge);
        if(armor.value.kind==Kind::GuardianArmor)rig.rounded(shoulder+P3{side*2,-1,3},{14,16,7},paint(armor.paint.secondary));
        rig.setBone(fore);
        rig.tapered(hand,elbow,6,7,skin,1.15f);
        rig.rounded(elbow,{8,9,8},plated?metal:cloth);
        P3 cuff=hand+(elbow-hand)*.42f;
        rig.rounded(cuff,{7.2f,8.2f,9},leather);
        panel({cuff.r,cuff.f+4.2f,cuff.z},7.4f,9.5f,4,5.8f,8.5f);
        rig.setBone(handBone);
        rig.rounded(!archer&&!rolling?hand+(elbow-hand)*.17f:hand+P3{0,-1,5},{7,8,7},leather);
        rig.rounded(hand,{7,7,8},skin);
        rig.rounded(hand+P3{0,3.2f,1},{5,1.5f,5},skinLight);
        for(int i=0;i<3;i++)rig.tapered(hand+P3{-2+i*2.f,3.8f,-1},hand+P3{-2+i*2.f,4,2},.35f,.35f,shade(skin,.73f));
        if(side>0&&jewel.shape==Kind::Ring){
            rig.rounded(hand+P3{1,2,1},{3.5f,5.5f,3},paint(jewel.paint.trim));
            rig.rounded(hand+P3{1,4.6f,1.5f},{3.5f,2.2f,3.5f},paint(jewel.paint.accent));
        }
        if(archer&&side<0){
            rig.setBone(-1);
            P3 grip=world(pose.world[hero_motion::data::HandL].p);
            P3 axis=world(pose.vector(hero_motion::data::HandL,{0,0,1}));
            P3 forward=world(pose.vector(hero_motion::data::HandL,{0,1,0}));
            float half=held.value.kind==Kind::BanditBow?23:held.value.kind==Kind::MoonBow?30:held.value.kind==Kind::MirageBow?29:27;
            P3 bottom=grip-axis*half,top=grip+axis*half,previous=bottom;
            for(int part=1;part<=16;part++){
                float t=part/16.f;P3 next=grip+axis*(-half+2*half*t)+forward*(6*std::sin(t*PI)*(1-.15f*drawCharge));
                rig.tapered(previous,next,2.6f,2.6f,weaponMetal);previous=next;
                if(part==2||part==14)rig.rounded(next,{3.8f,3.8f,4},weaponTrim);
                if(part==6||part==10)rig.rounded(next,{3,3,3},paint(held.paint.accent));
            }
            P3 string=bowDraw>=0&&!rolling?world(pose.world[hero_motion::data::HandR].p):grip;
            rig.beam(bottom,string,.75f,.75f,Text);rig.beam(string,top,.75f,.75f,Text);
            if(bowDraw>=0&&!rolling){
                P3 tip=grip+P3{0,20,0};rig.beam(string-P3{0,3,0},tip,1.4f,1.4f,rgb(190,143,81));
                rig.form(tip,{4,7,3},metal);rig.form(string-P3{0,1,0},{5,5,2},Text);
            }
        }
        if(side>0&&held.present()&&!archer&&!sheathed){
            bool dagger=weapon==Kind::Dagger;
            rig.setBone(hero_motion::data::Weapon);
            P3 blade=swordPoint(sword_pose::idleBlade(0));
            if(dagger)blade=blade*.65f;
            P3 base=hand-blade*.12f,tip=hand+blade;
            rig.beam(base,tip,weapon==Kind::Axe?3:held.value.kind==Kind::GuardianSword?5.5f:4,2,weapon==Kind::Axe?weaponGrip:weaponMetal);
            rig.beam(hand-blade*.20f,hand+blade*.06f,3.5f,3.5f,weaponGrip);
            rig.beam(hand+P3{0,0,.8f},tip+P3{0,0,.8f},.9f,.6f,shade(weaponMetal,1.3f));
            P3 guard=hand+blade*.10f;float flat=std::max(.001f,std::hypot(blade.r,blade.f));
            P3 cross{blade.f/flat*7,-blade.r/flat*7,0};
            rig.beam(guard-cross,guard+cross,2.5f,2.5f,weaponTrim);
            if(weapon==Kind::Axe)rig.form(tip+P3{5,0,-7},{held.value.kind==Kind::TuskAxe?20.f:15.f,4,14},weaponMetal);
            if(held.value.kind==Kind::TuskAxe)rig.tapered(tip+P3{-7,0,-5},tip+P3{-9,0,5},4,1,paint(held.paint.accent));
            if(held.value.kind==Kind::StingDagger||held.value.kind==Kind::DuneBlade)rig.beam(tip-blade*.25f,tip+cross*.45f,3,1,weaponMetal);
            if(beastGear(held.value.kind))for(int i=0;i<3;i++){
                P3 p=hand+blade*(.3f+i*.15f)+P3{0,0,1.2f};rig.beam(p-cross*.2f,p+cross*.2f,1,1,paint(held.paint.accent));
            }
        }
        if(side<0&&shield){
            bool stored=archer||run>.1f||rolling;float front=stored?-1.f:1.f;
            rig.setBone(stored?hero_motion::data::Spine:handBone);
            P3 p=stored?P3{archer?7.f:0.f,-15,archer?42.f:45.f}:hand+P3{-2,7,1};
            rig.rounded(p,{21,5,29},paint(ward.paint.trim));rig.rounded(p+P3{0,front*2,0},{18,4,25},paint(ward.paint.primary));
            rig.rounded(p+P3{0,front*4.2f,1},{3,2,22},paint(ward.paint.secondary));
            gearInlay(rig,ward,p+P3{0,front*5.5f,2},14,19,stored);
            for(int i=0;i<8;i++){float a=i*PI/4;rig.rounded(p+P3{std::cos(a)*7.8f,front*3.9f,std::sin(a)*11},{1.4f,1.4f,1.4f},edge);}
        }
    }
    rig.setBone(hero_motion::data::Head);
    float head=70+lift;
    rig.rounded({0,0,head},{18,17,21},skin);
    rig.rounded({0,1.6f,head-6},{12.5f,12,8},skin);
    if(hero_paint::atlas.id)panel({0,8.6f,head},18.4f,22,0,9.6f,11.3f);
    for(float side:{-1.f,1.f}){
        rig.rounded({side*9,0,head},{3.5f,4,6},skin);
        if(!hero_paint::atlas.id){
        rig.rounded({side*4,7.7f,head+1.6f},{3.2f,1.6f,1.7f},rgb(235,229,208));
        rig.rounded({side*4,8.6f,head+1.6f},{1.4f,1,1.5f},Ink);
        rig.tapered({side*2.3f,8,head+4},{side*5.8f,7,head+4.4f},.7f,.7f,shade(leather,.7f));
        rig.rounded({side*4.8f,7,head-2},{4,2.5f,3},skinLight);
        }
    }
    if(!hero_paint::atlas.id){
    rig.rounded({0,8.1f,head-1},{3.4f,4,4.5f},skinLight);
    rig.tapered({-2.3f,8.3f,head-5},{2.3f,8.3f,head-5},.65f,.65f,rgb(134,88,78));
    rig.rounded({0,6.5f,head-8},{9,3,2.5f},shade(skin,.82f));
    }
    Color hood=helmet?paint(headwear.paint.primary):paint(0x463b32);
    rig.rounded({0,-3,head+6},{22,19,15},hood);
    rig.rounded({0,-6,head},{21,11,23},hood);
    for(float side:{-1.f,1.f}){
        P3 a{side*8,6,head+8},b{side*10,4,head-5};
        rig.tapered(a,b,.8f,.8f,helmet?paint(headwear.paint.trim):paint(0x75573d));
    }
    if(helmet){
        bool soft=headwear.value.kind==Kind::WebCowl;
        if(!soft){rig.tapered({0,6,head+12},{0,10,head-4},1.2f,1.2f,paint(headwear.paint.trim));rig.rounded({0,0,head+13},{3,13,3},paint(headwear.paint.accent));}
        gearInlay(rig,headwear,{0,8,head+8},10,7);
        if(headwear.value.kind==Kind::BoneHelmet)for(float side:{-1.f,1.f})rig.rounded({side*9,4,head-2},{4,5,16},paint(headwear.paint.accent));
        if(headwear.value.kind==Kind::GuardianHelmet)for(int i=0;i<3;i++)rig.rounded({-6+i*6.f,-1,head+13},{5,10,4},paint(headwear.paint.secondary));
        if(headwear.value.kind==Kind::ScarabHelmet)rig.rounded({0,0,head+14},{8,14,3},paint(headwear.paint.secondary));
    }else{
        panel({0,8.7f,head+4},22,22,1,12.5f,13.5f);
        panel({0,-10.8f,head+4},22,25,2,12.5f,14,WHITE,true);
    }
    rig.setSkeleton(nullptr);
}
