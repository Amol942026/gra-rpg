#pragma once

// Included inside actor_art after Rig. Every view and pointer query uses this
// same articulated model, including equipment, bow poses and roll transforms.
inline void buildHero(Rig& rig,float lift,float stride,float cycle,float tuck,float attack,
                      bool helmet,bool shield,Kind weapon,bool flash,bool rolling,bool striking,
                      float strikeProgress,float bowDraw,int quiverArrows,float run=0){
    rig.reserve(1800);
    const Color skin=flash?Text:rgb(213,177,139),skinLight=flash?Text:rgb(237,206,172);
    const Color metal=flash?Text:rgb(142,165,165),edge=flash?Text:rgb(199,216,208);
    const Color cloth=flash?Text:rgb(43,103,93),clothLight=flash?Text:rgb(71,131,117);
    const Color leather=flash?Text:rgb(95,65,45),pants=flash?Text:rgb(60,74,66);
    bool archer=weapon==Kind::Bow;float drawCharge=std::max(0.f,bowDraw);
    // Cloth hangs in a curved sheet; its scalloped edge and folds move with
    // the walking cycle, while the roll rotates the entire sheet with the body.
    auto capePoint=[&](float u,float down){
        float width=12+6*down,r=(u*2-1)*width;
        float fold=std::cos(u*5*PI)*(.4f+down*1.2f);
        float sway=std::sin(cycle-down*2+u*3)*std::abs(stride)*down*(1.7f+run*1.3f);
        return P3{r+sway,-10-down*(3+run*9)-fold,58+lift-down*(45-run*7)+std::sin(u*4*PI)*down*1.2f};
    };
    for(int row=0;row<7;row++)for(int col=0;col<12;col++){
        float a=col/12.f,b=(col+1)/12.f,top=row/7.f,bottom=(row+1)/7.f;
        P3 p=capePoint(a,top),q=capePoint(b,top),r=capePoint(b,bottom),s=capePoint(a,bottom);
        float light=.77f+.22f*std::cos((a+b)*2.5f*PI);
        rig.face(p,q,r,s,cloth,light);rig.face(s,r,q,p,cloth,light*.85f);
    }
    for(int i=0;i<24;i++)rig.tapered(capePoint(i/24.f,1),capePoint((i+1)/24.f,1),.65f,.65f,Gold);
    for(float side:{0.f,1.f})for(int i=0;i<12;i++)rig.tapered(capePoint(side,i/12.f),capePoint(side,(i+1)/12.f),.7f,.7f,clothLight);
    for(float side:{-1.f,1.f}){
        float wave=stride*side,cosine=std::cos(cycle)*side;
        float walkLift=std::max(0.f,cosine)*std::abs(stride)*7;
        RunningLeg leg=run>0?runningLeg(cycle/(2*PI),side,31+lift):RunningLeg{};
        P3 foot{side*7,wave*7*(1-run)+leg.footForward*run+5*tuck,4+walkLift*(1-run)+(leg.footHeight-4)*run+12*tuck};
        P3 knee{side*7,wave*3*(1-run)+leg.kneeForward*run+14*tuck,(17+lift)*(1-run)+leg.kneeHeight*run+5*tuck};
        rig.tapered(knee,{side*7,0,31+lift},8,9,pants,1.15f);
        rig.rounded(knee,{8,8,9},pants);
        rig.tapered(foot,knee,7.5f,8,leather,.92f);
        rig.rounded(foot+P3{0,3,-1},{10,17,8},leather);
        rig.rounded(foot+P3{0,1,4},{9,10,9},leather);
        rig.rounded(foot+P3{0,5,-3},{10,14,2},rgb(37,32,28));
        rig.tapered(foot+P3{-4,2,5},foot+P3{4,2,5},1.1f,1.1f,Gold);
        rig.rounded(knee+P3{0,3,0},{7,4,8},metal);
    }
    rig.rounded({0,0,31+lift},{24,17,15},cloth);
    rig.rounded({0,0,46+lift},{27,18,28},metal);
    // Curved breastplate: raised rim, overlapping lower plates and a sun clasp.
    for(float side:{-1.f,1.f})for(int j=0;j<8;j++){
        float z=36+j*2.5f,zz=z+2.5f,r=side*(10.5f-std::abs(z-46)*.15f),rr=side*(10.5f-std::abs(zz-46)*.15f);
        rig.tapered({r,5.5f,z+lift},{rr,5.5f,zz+lift},.8f,.8f,edge);
    }
    rig.rounded({0,7,39+lift},{20,5,5},shade(metal,.95f));
    rig.rounded({0,7.6f,35+lift},{19,4,4},shade(metal,.85f));
    rig.rounded({0,0,32+lift},{25,18,4},leather);
    rig.rounded({0,9,32+lift},{5,2,4},Gold);rig.rounded({0,10,32+lift},{2.8f,1,2},leather);
    rig.tapered({-9,8,54+lift},{9,8,36+lift},2.5f,2,leather);
    rig.rounded({0,9.4f,49+lift},{5,2,6},Gold);rig.rounded({0,10.5f,49+lift},{2,1,3},cloth);
    for(float side:{-1.f,1.f}){
        rig.rounded({side*12,-1,29+lift},{7,9,9},leather);
        rig.rounded({side*12,3.5f,31+lift},{6,2,3},shade(leather,1.25f));
        rig.rounded({side*12,4.5f,30+lift},{1.6f,1,1.6f},Gold);
    }
    rig.rounded({0,0,61+lift},{10,10,10},skin);
    // A softer collar and two shoulder plates attach to rounded upper arms.
    rig.rounded({0,-1,57+lift},{28,18,8},cloth);
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
    for(float side:{-1.f,1.f}){
        float reach=side>0?attack*18:attack*6;
        RunningLeg armPhase=run>0?runningLeg(cycle/(2*PI),side,31+lift):RunningLeg{};
        float arm=armPhase.footForward/13;
        P3 shoulder{side*15,0,53+lift},elbow{side*17,(1-stride*side*2)*(1-run)+(2-arm*8)*run+reach*.4f,43+lift+run*2};
        P3 hand{side*(20-run*2),(3-stride*side*5)*(1-run)+(3-arm*14)*run+reach,33+lift+run*(5+std::max(0.f,-arm)*3)+std::abs(attack)*11};
        if(archer&&!rolling){
            if(side<0){hand={-10-run*5,27*(1-run)+(4-arm*12)*run,47+lift-run*8};elbow={-17,12*(1-run)+(3-arm*6)*run,44+lift};}
            else if(bowDraw>=0){hand={4-12*drawCharge,16-23*drawCharge,47+lift};elbow={18,4-9*drawCharge,47+lift};}
        }
        WeaponPose strike{};
        if(striking&&side>0){strike=forwardStrike(strikeProgress,lift);hand=strike.hand;elbow=strike.elbow;}
        hand=hand+P3{-side*5*tuck,9*tuck,6*tuck};elbow=elbow+P3{0,7*tuck,-4*tuck};
        rig.tapered(elbow,shoulder,8,9,cloth);
        rig.rounded(shoulder,{13,17,10},metal);
        rig.rounded(shoulder+P3{0,5,1},{11,5,3},edge);
        rig.tapered(hand,elbow,6,7,skin,1.15f);
        rig.rounded(elbow,{8,9,8},metal);
        rig.rounded(hand+P3{0,-1,5},{7,8,7},leather);
        rig.rounded(hand,{7,7,8},skin);
        rig.rounded(hand+P3{0,3.2f,1},{5,1.5f,5},skinLight);
        for(int i=0;i<3;i++)rig.tapered(hand+P3{-2+i*2.f,3.8f,-1},hand+P3{-2+i*2.f,4,2},.35f,.35f,shade(skin,.73f));
        if(archer&&side<0){
            P3 bottom=hand+P3{0,0,-27},top=hand+P3{0,0,27},previous=bottom;
            for(int part=1;part<=16;part++){
                float t=part/16.f;P3 next=hand+P3{0,6*std::sin(t*PI)*(1-.15f*drawCharge),-27+54*t};
                rig.tapered(previous,next,2.6f,2.6f,rgb(153,100,49));previous=next;
            }
            P3 string=bowDraw>=0&&!rolling?P3{4-12*drawCharge,16-23*drawCharge,47+lift}:hand;
            rig.beam(bottom,string,.75f,.75f,Text);rig.beam(string,top,.75f,.75f,Text);
            rig.rounded(hand+P3{0,3,0},{5,4,9},leather);
            if(bowDraw>=0&&!rolling){
                P3 tip=hand+P3{0,20,0};rig.beam(string-P3{0,3,0},tip,1.4f,1.4f,rgb(190,143,81));
                rig.form(tip,{4,7,3},metal);rig.form(string-P3{0,1,0},{5,5,2},Text);
            }
        }
        if(side>0&&!archer){
            P3 base=hand+P3{0,1,-3},tip=hand+P3{0,8*(1-run)-20*run+attack*8,31-std::abs(attack)*10-run*55};
            bool dagger=weapon==Kind::Dagger;if(dagger)tip=hand+(tip-hand)*.65f;
            if(striking){
                base=hand-strike.blade*.12f;tip=hand+strike.blade*(dagger?.65f:1.f);
                rig.beam(base,tip,weapon==Kind::Axe?3:4,2,weapon==Kind::Axe?leather:edge);
                rig.beam(base+P3{0,0,1.2f},tip+P3{0,0,1.2f},.9f,.6f,Text);
                P3 guard=hand+strike.blade*.12f;float flat=std::hypot(strike.blade.r,strike.blade.f);
                P3 cross{strike.blade.f/flat*7,-strike.blade.r/flat*7,0};rig.beam(guard-cross,guard+cross,2.5f,2.5f,Gold);
            }else{
                rig.beam(base,tip,weapon==Kind::Axe?3:4,2,weapon==Kind::Axe?leather:edge);
                rig.tapered(hand+P3{-7,2,4},hand+P3{7,2,4},2.4f,2.4f,Gold);
            }
            if(weapon==Kind::Axe)rig.form(tip+P3{5,0,-7},{15,4,14},metal);
        }
        if(side<0&&shield&&!archer){
            P3 p=hand+P3{-2,7,1};
            rig.rounded(p,{21,5,29},Gold);rig.rounded(p+P3{0,2,0},{18,4,25},leather);
            rig.rounded(p+P3{0,4.2f,1},{3,2,22},metal);rig.rounded(p+P3{0,4.2f,2},{17,2,3},metal);
            rig.rounded(p+P3{0,5.4f,2},{6,3,7},Gold);
            for(int i=0;i<8;i++){
                float a=i*PI/4;rig.rounded(p+P3{std::cos(a)*7.8f,3.9f,std::sin(a)*11},{1.4f,1.4f,1.4f},edge);
            }
        }
    }
    float head=70+lift;
    rig.rounded({0,0,head},{18,17,21},skin);
    rig.rounded({0,2,head-6},{14,13,9},skin);
    for(float side:{-1.f,1.f}){
        rig.rounded({side*9,0,head},{3.5f,4,6},skin);
        rig.rounded({side*4,7.7f,head+1.6f},{3.2f,1.6f,1.7f},rgb(235,229,208));
        rig.rounded({side*4,8.6f,head+1.6f},{1.4f,1,1.5f},Ink);
        rig.tapered({side*2.3f,8,head+4},{side*5.8f,7,head+4.4f},.7f,.7f,shade(leather,.7f));
        rig.rounded({side*4.8f,7,head-2},{4,2.5f,3},skinLight);
    }
    rig.rounded({0,8.1f,head-1},{3.4f,4,4.5f},skinLight);
    rig.tapered({-2.3f,8.3f,head-5},{2.3f,8.3f,head-5},.65f,.65f,rgb(134,88,78));
    rig.rounded({0,6.5f,head-8},{9,3,2.5f},shade(skin,.82f));
    Color hood=helmet?metal:cloth;
    rig.rounded({0,-3,head+6},{22,19,15},hood);
    rig.rounded({0,-6,head},{21,11,23},hood);
    for(float side:{-1.f,1.f}){
        P3 a{side*8,6,head+8},b{side*10,4,head-5};
        rig.tapered(a,b,.8f,.8f,helmet?Gold:clothLight);
    }
    if(helmet){
        rig.tapered({0,6,head+12},{0,10,head-4},1.2f,1.2f,Gold);
        rig.rounded({0,0,head+13},{3,13,3},edge);
    }else{
        // A few curved strands show below the open hood rather than a flat cap.
        for(int i=0;i<4;i++)rig.rounded({-5+i*3.f,5.5f,head+7-i%2},{4,4,3},leather);
    }
}
