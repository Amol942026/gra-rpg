#pragma once

// Curved, illustrated details for the close-up prologue. World actor rigs stay
// at their usual gameplay scale; these shapes can carry finer cloth and stone.
namespace intro_art {
Vector2 bezier(Vector2 a,Vector2 b,Vector2 c,Vector2 d,float t){
    float q=1-t;return {q*q*q*a.x+3*q*q*t*b.x+3*q*t*t*c.x+t*t*t*d.x,
                       q*q*q*a.y+3*q*q*t*b.y+3*q*t*t*c.y+t*t*t*d.y};
}
struct Curve {Vector2 a,b,c,d;};
void shape(Vector2 center,std::initializer_list<Curve> curves,Color color){
    for(const Curve& curve:curves){
        Vector2 last=curve.a;
        for(int i=1;i<=20;i++){
            Vector2 next=bezier(curve.a,curve.b,curve.c,curve.d,i/20.f);
            Vector2 a=last,b=next;
            if((a.x-center.x)*(b.y-center.y)-(a.y-center.y)*(b.x-center.x)>0)std::swap(a,b);
            DrawTriangle(center,a,b,color);last=next;
        }
    }
}
void stroke(Vector2 a,Vector2 b,Vector2 c,Vector2 d,float width,Color color){
    DrawSplineSegmentBezierCubic(a,b,c,d,width,color);
}
void arc(float x,float y,float rx,float ry,float start,float end,Color color,float width=1){
    Vector2 last{x+std::cos(start)*rx,y+std::sin(start)*ry};
    for(int i=1;i<=96;i++){
        float a=start+(end-start)*i/96.f;Vector2 next{x+std::cos(a)*rx,y+std::sin(a)*ry};
        DrawLineEx(last,next,width,color);last=next;
    }
}
void glow(float x,float y,float rx,float ry,Color inner){
    rlPushMatrix();rlTranslatef(x,y,0);rlScalef(1,ry/rx,1);
    DrawCircleGradient(0,0,rx,inner,Fade(inner,0));rlPopMatrix();
}
void star(float x,float y,float size,Color color){
    stroke({x-size,y},{x,y-size*.15f},{x,y-size*.15f},{x+size,y},.6f,color);
    stroke({x,y-size},{x+size*.15f,y},{x+size*.15f,y},{x,y+size},.6f,color);
    DrawCircleV({x,y},size*.2f,color);
}
void pillar(float x,float y,float height){
    ellipse(x,y+height+12,48,17,rgb(20,20,35));
    DrawRectangleGradientH(int(x-28),int(y),56,int(height),rgb(30,31,49),rgb(66,61,82));
    DrawRectangleGradientH(int(x-14),int(y),23,int(height),rgb(73,66,88),rgb(42,40,61));
    for(int i=0;i<6;i++)line(x-21+i*8,y+9,x-21+i*8,y+height-7,Fade(rgb(119,104,131),i%2?.23f:.4f),i%2?1:2);
    for(int i=0;i<8;i++)arc(x,y+15+i*(height-30)/8,28,6,0,PI,rgb(28,28,44),1.5f);
    for(float bottom:{y,y+height}){
        rect(x-36,bottom-9,72,18,rgb(63,57,78));ellipse(x,bottom+9,36,9,rgb(37,35,52));
        ellipse(x,bottom-9,36,9,rgb(88,76,99));arc(x,bottom-8,34,8,0,PI,rgb(115,94,121),1);
    }
    stroke({x+13,y+111},{x-3,y+131},{x+10,y+141},{x-2,y+165},1.5f,rgb(23,24,40));
    stroke({x-31,y+height-20},{x-40,y+height-87},{x-15,y+height-123},{x-24,y+height-185},2.2f,rgb(50,64,60));
    for(int i=0;i<16;i++){
        float px=x-27+std::sin(i*.6f)*9,py=y+height-27-i*9;
        ellipse(px+(i%2?6:-6),py,6,2.7f,rgb(58+i%3*6,74+i%4*5,66));
    }
    for(int i=0;i<3;i++){
        float yy=y+47+i*63;
        arc(x,yy,7,11,-PI*.8f,PI*.8f,Fade(Gold,.4f),1);
        star(x,yy,3,Fade(Gold,.6f));
    }
}
void candle(float x,float y,float time){
    ellipse(x,y+5,19,6,rgb(17,17,28));ellipse(x,y,13,4,rgb(111,80,59));
    rect(x-5,y-26,10,26,rgb(174,153,124));rect(x-4,y-26,3,24,rgb(223,199,157));
    ellipse(x,y-26,5,2,rgb(239,214,170));line(x,y-26,x,y-32,rgb(69,47,39),1.3f);
    float flicker=std::sin(time*8+x)*1.5f;
    glow(x,y-36,48+flicker,58,rgb(250,156,71,34));
    ellipse(x+flicker*.25f,y-36,4,9+flicker,rgb(243,169,80));ellipse(x,y-33,2.5f,5,rgb(255,229,162));
}
void observatory(float time){
    DrawRectangleGradientV(0,0,W,H,rgb(10,13,29),rgb(28,23,43));
    // Deep arched opening, hazy nebulae and fine stars behind the stonework.
    ellipse(720,320,448,248,rgb(22,24,46));
    glow(613,237,222,141,rgb(84,60,125,58));glow(848,252,243,159,rgb(44,72,111,58));
    for(int i=0;i<115;i++){
        float px=325+(i*137)%785,py=135+(i*53)%225,alpha=.2f+.18f*std::sin(time*.8f+i);
        if(i%15==0)star(px,py,3,Fade(rgb(210,200,236),alpha+.15f));
        else DrawCircleV({px,py},i%7?.8f:1.5f,Fade(rgb(186,181,223),alpha));
    }
    arc(720,355,459,266,PI,2*PI,rgb(57,52,77),22);
    arc(720,355,449,254,PI,2*PI,rgb(110,91,127),2);
    arc(720,355,471,278,PI,2*PI,rgb(25,26,43),5);
    for(int i=0;i<22;i++){
        float a=PI+(i+.5f)*PI/22;
        line(720+std::cos(a)*451,355+std::sin(a)*255,720+std::cos(a)*468,355+std::sin(a)*273,rgb(34,33,54),2);
        if(i%2==0)star(720+std::cos(a)*459,355+std::sin(a)*263,2,Fade(Gold,.35f));
    }
    for(int i=0;i<3;i++){
        float x=475+i*235,y=170+i%2*27;
        glow(x+13,y+36,25,68,rgb(191,50,86,26));
        stroke({x,y},{x+26,y+15},{x-9,y+49},{x+20,y+74},3,rgb(153,58,86,160));
        stroke({x,y},{x+26,y+15},{x-9,y+49},{x+20,y+74},.8f,rgb(242,115,133,155));
        line(x+10,y+35,x-8,y+43,rgb(142,55,80,135),1);
    }
    // Rounded stone dais, inset rings, radial joints, worn chips and runes.
    ellipse(720,487,560,159,rgb(16,18,32));ellipse(720,474,555,156,rgb(52,46,65));
    arc(720,474,550,154,0,PI,rgb(91,76,103),2);
    ellipse(720,460,540,148,rgb(42,40,58));
    glow(670,418,490,135,rgb(110,86,134,25));
    for(int i=1;i<6;i++)arc(720,460,90*i,24.65f*i,-PI,PI,Fade(rgb(115,96,135),.2f),1);
    for(int i=0;i<28;i++){
        float a=i*2*PI/28;
        line(720+std::cos(a)*90,460+std::sin(a)*25,720+std::cos(a)*531,460+std::sin(a)*146,Fade(rgb(115,96,135),.17f));
        float px=720+std::cos(a)*509,py=460+std::sin(a)*139;
        star(px,py,3,Fade(Gold,.42f));
    }
    for(int i=0;i<75;i++){
        float a=i*2.39996f,r=100+(i*47)%410,px=720+std::cos(a)*r,py=460+std::sin(a)*r*.274f;
        line(px,py,px+2+i%5,py+.7f,Fade(rgb(144,127,156),.14f));
    }
    glow(510,533,143,43,rgb(125,90,194,33));glow(930,505,122,37,rgb(189,134,219,27));
    // The astrolabe supports five fragments, with smaller concentric engravings.
    ellipse(720,455,146,50,rgb(25,24,43));
    for(int i=0;i<4;i++)arc(720,455,140-i*13,48-i*4.5f,-PI,PI,Fade(Gold,i%2?.18f:.43f),i?1:1.5f);
    for(int i=0;i<24;i++){
        float a=i*PI/12;
        line(720+std::cos(a)*124,455+std::sin(a)*42,720+std::cos(a)*137,455+std::sin(a)*47,Fade(Gold,.46f));
    }
    pillar(242,181,330);pillar(1200,181,330);
    candle(329,490,time);candle(1118,494,time+2);
    // Weathered books and an unfurled parchment beside Eldrin.
    poly({{998,534},{1028,525},{1053,538},{1022,550}},rgb(49,33,49));
    poly({{999,530},{1029,521},{1053,533},{1022,545}},rgb(145,126,100));
    poly({{997,526},{1027,517},{1052,529},{1021,541}},rgb(92,51,72));
    line(1004,527,1021,536,Fade(Gold,.6f),1.4f);line(1011,523,1028,533,Fade(Gold,.6f));
    poly({{1061,548},{1090,538},{1104,544},{1075,555}},rgb(186,159,119));
    for(int i=0;i<4;i++)line(1070+i*5,547-i*1.5f,1075+i*5,549-i*1.5f,rgb(103,80,66),.8f);
    ellipse(1062,548,3,5,rgb(223,196,149));ellipse(1103,543,3,5,rgb(223,196,149));
    for(int i=0;i<32;i++){
        float px=320+(i*163)%790,py=220+std::fmod(i*31+time*(4+i%3),310.f);
        DrawCircleV({px,py},i%3?1:1.5f,Fade(rgb(201,170,225),.13f+.1f*std::sin(time+i)));
    }
}
void mage(float x,float y,float size,float time,float casting){
    float breath=std::sin(time*2)*.55f,lift=casting*13;
    Color purple=rgb(104,65,150),shadow=rgb(55,35,88),light=rgb(155,105,201),gold=rgb(213,177,111);
    Color skin=rgb(211,172,137),skinLight=rgb(239,207,170),silver=rgb(221,218,208);
    glow(x,y+6,46*size,13*size,rgb(3,5,12,150));
    rlPushMatrix();rlTranslatef(x,y,0);rlScalef(size,size,1);
    ellipse(-11,0,9,4,rgb(31,23,29));ellipse(12,1,9,4,rgb(31,23,29));
    // Softly flared skirt, several overlapping panels and a scalloped hem.
    shape({0,-37},{{{-11,-68},{-24,-65},{-20,-34},{-28,-3}},
                       {{-28,-3},{-19,8},{14,8},{26,-3}},
                       {{26,-3},{21,-32},{23,-58},{13,-68}},
                       {{13,-68},{4,-75},{-7,-73},{-11,-68}}},shadow);
    shape({-3,-35},{{{-11,-66},{-19,-59},{-18,-24},{-24,-3}},
                        {{-24,-3},{-13,4},{8,5},{19,-2}},
                        {{19,-2},{17,-21},{18,-53},{10,-66}},
                        {{10,-66},{2,-70},{-5,-70},{-11,-66}}},purple);
    shape({-7,-34},{{{-10,-63},{-12,-40},{-16,-15},{-15,0}},
                        {{-15,0},{-10,2},{-6,3},{-3,2}},
                        {{-3,2},{-3,-18},{-2,-47},{-2,-65}},
                        {{-2,-65},{-4,-66},{-8,-66},{-10,-63}}},rgb(124,79,174));
    for(int i=0;i<7;i++){
        float a=-19+i*6;
        stroke({a*.45f,-58},{a*.6f-2,-40},{a-3,-17},{a,-2},i%2?.65f:1.25f,
               i%2?Fade(light,.55f):Fade(shadow,.6f));
    }
    stroke({-25,-2},{-10,5},{7,6},{23,-2},1.3f,gold);
    stroke({-25,-5},{-10,2},{7,3},{23,-5},.45f,Fade(gold,.7f));
    for(int i=0;i<13;i++)star(-21+i*3.5f,-2+std::sin(i*PI/12)*4,1,Fade(gold,.85f));
    // Embroidered front stole, belt, clasp and small constellations.
    stroke({-4,-63},{-6,-42},{-11,-19},{-9,1},2.4f,shadow);
    stroke({-5,-63},{-7,-42},{-12,-19},{-10,1},.7f,gold);
    stroke({1,-63},{0,-42},{-3,-18},{-2,2},.65f,gold);
    for(int i=0;i<8;i++)star(-3-i*.45f,-16-i*5,1.1f,Fade(gold,.85f));
    stroke({-17,-37},{-7,-34},{6,-34},{19,-37},2.6f,rgb(67,41,76));
    stroke({-17,-38},{-7,-35},{6,-35},{19,-38},.65f,gold);
    ellipse(-3,-36,3,2.2f,gold);ellipse(-3,-36,1.7f,1.2f,rgb(91,63,126));
    for(int i=0;i<6;i++){float px=9+(i%2)*5,py=-12-i*6;star(px,py,1.4f,Fade(gold,.62f));}
    // Far sleeve and fingers around the staff.
    shape({-23,-51},{{{-13,-64},{-24,-65},{-29,-50-lift},{-39,-43-lift}},
                         {{-39,-43-lift},{-40,-38-lift},{-30,-34-lift},{-27,-38-lift}},
                         {{-27,-38-lift},{-17,-44},{-11,-52},{-13,-64}}},rgb(92,54,137));
    stroke({-16,-61},{-23,-58},{-24,-47-lift},{-33,-42-lift},.8f,light);
    stroke({-38,-42-lift},{-36,-40-lift},{-32,-39-lift},{-29,-38-lift},1.2f,gold);
    // Slightly twisted polished wood, grain and a rounded ornamental crown.
    stroke({-39,-1},{-44,-34},{-37,-71},{-41,-106-lift},4.3f,rgb(63,41,38));
    stroke({-40,-2},{-44,-34},{-38,-71},{-42,-105-lift},2.8f,rgb(116,80,58));
    stroke({-41,-3},{-44,-35},{-40,-74},{-43,-105-lift},.8f,rgb(177,133,88));
    for(int i=0;i<7;i++)stroke({-44,-14-i*11},{-42,-15-i*11},{-39,-16-i*11},{-38,-17-i*11},.5f,Fade(gold,.5f));
    ellipse(-40,-41-lift,4.1f,5.4f,skin);ellipse(-42,-42-lift,2,3.5f,skinLight);
    for(int i=0;i<3;i++)line(-43,-43-lift+i*2,-38,-42-lift+i*2,rgb(145,106,86),.45f);
    float gy=-113-lift;
    for(int side:{-1,1})stroke({-41,gy+10},{-41+side*13.f,gy+4},{-41+side*12.f,gy-8},{-41+side*4.f,gy-12},1.6f,gold);
    ellipse(-41,gy+9,5.7f,2.4f,gold);
    glow(-41,gy,22,25,Fade(rgb(165,117,244),.3f+casting*.2f));
    ellipse(-41,gy,6.5f,10,rgb(101,64,173));ellipse(-42.5f,gy-1,4.5f,8,rgb(187,142,239));
    ellipse(-44,gy-4,1.8f,3.6f,rgb(245,225,255));star(-41,gy,9,Fade(rgb(239,213,255),.7f));
    // Rounded shoulders and a heavy hood, with light catching the fabric rim.
    ellipse(0,-65+breath,20,10,shadow);ellipse(-2,-67+breath,17,8,purple);
    stroke({-17,-65+breath},{-6,-58+breath},{8,-59+breath},{18,-65+breath},1.15f,gold);
    shape({2,-86+breath},{{{-16,-91+breath},{-14,-108+breath},{13,-109+breath},{19,-92+breath}},
                                  {{19,-92+breath},{24,-80+breath},{23,-72+breath},{16,-65+breath}},
                                  {{16,-65+breath},{5,-61+breath},{-8,-64+breath},{-13,-74+breath}},
                                  {{-13,-74+breath},{-17,-81+breath},{-19,-84+breath},{-16,-91+breath}}},shadow);
    ellipse(-5,-85+breath,13,18,rgb(29,23,44));
    ellipse(-7,-84+breath,9.8f,13.7f,skin);ellipse(-10,-86+breath,6.6f,10.5f,skinLight);
    ellipse(-1,-83+breath,3.6f,5,rgb(186,141,115));arc(-1,-83+breath,2,3,-PI*.7f,PI*.5f,rgb(138,96,82),.6f);
    // Profile looking at the hero: brow, lid, iris, nose and cheek wrinkles.
    stroke({-16,-89+breath},{-13,-92+breath},{-9,-92+breath},{-6,-89+breath},1.35f,silver);
    stroke({-15,-86+breath},{-13,-88+breath},{-10,-88+breath},{-8,-86+breath},.8f,rgb(85,65,66));
    ellipse(-12,-86.4f+breath,2.3f,1.2f,rgb(246,235,215));ellipse(-13,-86.4f+breath,.8f,1.1f,rgb(87,97,109));
    ellipse(-17,-82+breath,3.4f,2.6f,skinLight);
    stroke({-17,-81+breath},{-17,-80+breath},{-14,-79+breath},{-13,-80+breath},.65f,rgb(158,113,90));
    stroke({-13,-82+breath},{-11,-80+breath},{-9,-80+breath},{-7,-81+breath},.4f,rgb(164,122,100));
    line(-13,-76+breath,-7,-76+breath,rgb(125,86,84),.8f);
    // Flowing beard has rounded lobes and individual strands, rather than a spike.
    shape({-6,-65+breath},{{{-15,-77+breath},{-20,-68+breath},{-13,-58+breath},{-10,-54+breath}},
                               {{-10,-54+breath},{-5,-46+breath},{5,-48+breath},{7,-54+breath}},
                               {{7,-54+breath},{9,-62+breath},{3,-72+breath},{-2,-77+breath}},
                               {{-2,-77+breath},{-7,-75+breath},{-11,-75+breath},{-15,-77+breath}}},rgb(171,170,171));
    for(int i=0;i<10;i++){
        float px=-14+i*1.5f;
        stroke({px,-76+breath},{px-3,-68+breath},{px+2,-57+breath},{-4+i*.7f,-51+breath},
               i%3? .7f:1.2f,i%3?silver:rgb(240,236,222));
    }
    stroke({-15,-77+breath},{-12,-79+breath},{-9,-78+breath},{-8,-76+breath},1.7f,silver);
    stroke({-8,-76+breath},{-7,-77+breath},{-4,-78+breath},{-2,-76+breath},1.6f,silver);
    stroke({-16,-91+breath},{-9,-104+breath},{10,-103+breath},{15,-89+breath},2.3f,purple);
    stroke({-16,-91+breath},{-9,-104+breath},{10,-103+breath},{15,-89+breath},.75f,gold);
    stroke({15,-89+breath},{14,-79+breath},{16,-73+breath},{18,-69+breath},1.1f,light);
    for(int i=0;i<4;i++)star(10+i*.8f,-86+i*5+breath,1,Fade(gold,.75f));
    // Near sleeve gestures toward the hero and lifts as the spell gathers.
    float handX=-22-casting*9,handY=-49-lift;
    shape({0,-53},{{{12,-64},{21,-63},{19,-48},{7,-45}},
                       {{7,-45},{-2,-43},{handX+7,handY+7},{handX,handY+5}},
                       {{handX,handY+5},{handX-2,handY+2},{handX,handY-3},{handX+3,handY-4}},
                       {{handX+3,handY-4},{-1,-51},{7,-58},{12,-64}}},purple);
    stroke({13,-61},{18,-54},{1,-46},{handX+2,handY+1},1.1f,light);
    line(handX,handY+4,handX+2,handY-3,gold,1.3f);
    ellipse(handX-3,handY,4.5f,3,skin);ellipse(handX-4,handY-1,3,1.4f,skinLight);
    for(int i=0;i<3;i++)line(handX-8,handY-2+i*1.5f,handX-3,handY-2+i*1.5f,skinLight,.8f);
    if(casting>0)glow(handX-5,handY-2,17,17,Fade(rgb(184,145,255),casting*.55f));
    rlPopMatrix();
}

void column(Vector2 p,float time,float energy){
    Color blue=rgb(76,160,255),cyan=rgb(169,225,255);
    float pulse=1+.04f*std::sin(time*9),height=232,radius=43*pulse;
    // Additive, translucent light preserves the hero inside the cylinder.
    BeginBlendMode(BLEND_ADDITIVE);
    glow(p.x,p.y-95,radius*2.2f,150,Fade(blue,energy*.27f));
    for(int i=0;i<14;i++){
        float half=radius*(1-i/19.f),strength=energy*(.018f+i*.001f);
        DrawRectangleGradientEx({p.x-half,p.y-height,half*2,height},
            Fade(blue,0),Fade(cyan,strength),Fade(blue,0),Fade(cyan,strength));
    }
    for(int side:{-1,1}){
        stroke({p.x+side*radius,p.y},{p.x+side*radius*1.1f,p.y-75},
               {p.x+side*radius*.8f,p.y-180},{p.x+side*radius*.82f,p.y-height},2.5f,Fade(blue,energy*.75f));
        stroke({p.x+side*(radius-3),p.y},{p.x+side*radius,p.y-75},
               {p.x+side*radius*.7f,p.y-180},{p.x+side*radius*.8f,p.y-height},.9f,Fade(cyan,energy*.9f));
    }
    for(int i=0;i<3;i++){
        float rise=std::fmod(time*.6f+i/3.f,1.f),yy=p.y-rise*height;
        arc(p.x,yy,radius*(1-.14f*rise),12,-PI,PI,Fade(cyan,energy*(1-rise)*.65f),1.2f);
    }
    for(int i=0;i<48;i++){
        float rise=std::fmod(time*(.35f+(i%5)*.035f)+i*.073f,1.f),a=i*2.4f+time*1.4f;
        float xx=p.x+std::cos(a)*radius*(.65f+(i%3)*.12f),yy=p.y-rise*height;
        line(xx,yy+5,xx,yy,Fade(blue,energy*(1-rise)*.65f),1.4f);
        DrawCircleV({xx,yy},1.2f+(i%3)*.5f,Fade(cyan,energy*(1-rise)*.9f));
    }
    glow(p.x,p.y+3,90,31,Fade(blue,energy*.65f));
    for(int i=0;i<3;i++)arc(p.x,p.y+3,radius+8+i*9,14+i*3,-PI,PI,Fade(cyan,energy*(.8f-i*.19f)),1.2f);
    for(int i=0;i<12;i++){
        float a=i*PI/6+time*.3f;star(p.x+std::cos(a)*65,p.y+3+std::sin(a)*22,2.3f,Fade(cyan,energy*.8f));
    }
    EndBlendMode();
}
}
