#pragma once

// Small geometric character rigs: all parts turn in depth, including the face,
// back, weapon and limbs. A back view is not a mirrored front sprite.
namespace actor_art {
struct P3 {
    float r, f, z;
    P3 operator+(P3 b) const { return {r+b.r, f+b.f, z+b.z}; }
    P3 operator-(P3 b) const { return {r-b.r, f-b.f, z-b.z}; }
    P3 operator*(float n) const { return {r*n, f*n, z*n}; }
};
struct WeaponPose {P3 hand,elbow,blade;};
inline WeaponPose forwardStrike(float progress,float lift=0) {
    float p=std::clamp(progress,0.f,1.f),reach=std::sin(p*PI);
    float angle=(68-136*p)*DEG2RAD;
    P3 hand{20+(12-24*p-20)*reach,3+31*reach,32+lift+14*reach};
    P3 elbow{17-3*reach,1+17*reach,40+lift+8*reach};
    P3 blade=P3{0,8,31}*(1-reach)+P3{std::sin(angle)*43,std::cos(angle)*43,5}*reach;
    return {hand,elbow,blade};
}
struct Face {
    std::array<Vector2,4> points;
    float depth;
    Color color;
};
class Rig {
    float x, y, scale, fx, fy, rx, ry;
    float rollAngle=0, tuck=0;
    std::vector<Face> faces;
public:
    Rig(float px,float py,float size,Direction dir):x(px),y(py),scale(size) {
        float angle=int(dir)*PI/4;
        fx=std::cos(angle);fy=std::sin(angle)/.52f;
        float len=std::hypot(fx,fy);fx/=len;fy/=len;rx=fy;ry=-fx;
        faces.reserve(320);
    }
    void setRoll(float progress,float amount) {rollAngle=progress*2*PI;tuck=amount;}
    P3 rollPoint(P3 p) const {
        if(tuck<=0)return p;
        float z=(p.z-38)*(1-.32f*tuck),f=p.f;
        return {p.r*(1-.12f*tuck),f*std::cos(rollAngle)+z*std::sin(rollAngle),
                38-9*tuck+z*std::cos(rollAngle)-f*std::sin(rollAngle)};
    }
    Vector2 project(P3 p) const {
        return {x+(p.r*rx+p.f*fx)*scale,y+((p.r*ry+p.f*fy)*.52f-p.z)*scale};
    }
    void face(P3 a,P3 b,P3 c,P3 d,Color col,float lighting=1) {
        a=rollPoint(a);b=rollPoint(b);c=rollPoint(c);d=rollPoint(d);
        Vector2 pa=project(a),pb=project(b),pc=project(c),pd=project(d);
        // Opaque closed meshes need only their camera-facing surfaces. This
        // keeps the richer silhouettes inexpensive and matches pointer picking.
        float area=(pb.x-pa.x)*(pc.y-pa.y)-(pb.y-pa.y)*(pc.x-pa.x)
                  +(pc.x-pa.x)*(pd.y-pa.y)-(pc.y-pa.y)*(pd.x-pa.x);
        if(area<=.001f)return;
        float depth=0;
        for(P3 p:{a,b,c,d})depth+=(p.r*ry+p.f*fy)*.86f+p.z*.52f;
        faces.push_back({{pa,pb,pc,pd},depth*.25f,shade(col,lighting)});
    }
    void prism(const std::array<P3,8>& p,Color col) {
        // Bottom, back, front, left, right, top, with a fixed world light.
        face(p[0],p[3],p[2],p[1],col,.6f);
        face(p[0],p[1],p[5],p[4],col,.78f+.12f*fx);
        face(p[3],p[7],p[6],p[2],col,.82f-.12f*fx);
        face(p[0],p[4],p[7],p[3],col,.84f+.13f*rx);
        face(p[1],p[2],p[6],p[5],col,.84f-.13f*rx);
        face(p[4],p[5],p[6],p[7],col,1.13f);
    }
    void box(P3 center,P3 size,Color col) {
        float r=size.r/2,f=size.f/2,z=size.z/2;
        prism({center+P3{-r,-f,-z},center+P3{r,-f,-z},center+P3{r,f,-z},center+P3{-r,f,-z},
               center+P3{-r,-f,z},center+P3{r,-f,z},center+P3{r,f,z},center+P3{-r,f,z}},col);
    }
    // Three octagonal rings give bodies curved shoulders and bevelled edges.
    // The fixed vertices avoid per-frame trigonometry for every body part.
    void form(P3 center,P3 size,Color col) {
        constexpr float ring[8][2]={{1,0},{.7071f,.7071f},{0,1},{-.7071f,.7071f},
                                   {-1,0},{-.7071f,-.7071f},{0,-1},{.7071f,-.7071f}};
        std::array<std::array<P3,8>,3> p;
        const float level[]={-.5f,.08f,.5f},radius[]={.72f,1.f,.72f};
        for(int j=0;j<3;j++)for(int i=0;i<8;i++)p[j][i]=center+P3{ring[i][0]*size.r*.5f*radius[j],ring[i][1]*size.f*.5f*radius[j],size.z*level[j]};
        for(int j=0;j<2;j++)for(int i=0;i<8;i++){
            int n=(i+1)%8;float normalR=(ring[i][0]+ring[n][0])*.5f,normalF=(ring[i][1]+ring[n][1])*.5f;
            float light=.86f+.18f*(normalR*rx+normalF*fx)+(j?.14f:-.06f);
            face(p[j][i],p[j][n],p[j+1][n],p[j+1][i],col,light);
        }
        for(int i=0;i<8;i+=2){
            face(center+P3{0,0,size.z*.5f},p[2][i],p[2][(i+1)%8],p[2][(i+2)%8],col,1.18f);
            face(center-P3{0,0,size.z*.5f},p[0][(i+2)%8],p[0][(i+1)%8],p[0][i],col,.6f);
        }
    }
    void spike(P3 base,P3 tip,float width,float depth,Color col) {
        P3 axis=tip-base;float n=std::sqrt(axis.r*axis.r+axis.f*axis.f+axis.z*axis.z);if(n<.001f)return;
        axis=axis*(1/n);float flat=std::hypot(axis.r,axis.f);
        P3 u=flat>.001f?P3{axis.f/flat,-axis.r/flat,0}:P3{1,0,0};
        P3 v{axis.f*u.z-axis.z*u.f,axis.z*u.r-axis.r*u.z,axis.r*u.f-axis.f*u.r};
        u=u*(width*.5f);v=v*(depth*.5f);
        std::array<P3,4> p={base-u-v,base+u-v,base+u+v,base-u+v};
        for(int i=0;i<4;i++)face(p[i],p[(i+1)%4],tip,tip,col,.83f+i*.09f);
        face(p[3],p[2],p[1],p[0],col,.65f);
    }
    void limb(P3 a,P3 b,float width,float depth,Color col) {
        float r=width/2,f=depth/2;
        prism({a+P3{-r,-f,0},a+P3{r,-f,0},a+P3{r,f,0},a+P3{-r,f,0},
               b+P3{-r,-f,0},b+P3{r,-f,0},b+P3{r,f,0},b+P3{-r,f,0}},col);
    }
    // A weapon needs a cross-section perpendicular to its blade, including
    // when the sword becomes almost horizontal during the forward cut.
    void beam(P3 a,P3 b,float width,float depth,Color col) {
        P3 axis=b-a;float n=std::sqrt(axis.r*axis.r+axis.f*axis.f+axis.z*axis.z);
        if(n<.001f)return;axis=axis*(1/n);
        float flat=std::hypot(axis.r,axis.f);
        P3 u=flat>.001f?P3{axis.f/flat,-axis.r/flat,0}:P3{1,0,0};
        P3 v{axis.f*u.z-axis.z*u.f,axis.z*u.r-axis.r*u.z,axis.r*u.f-axis.f*u.r};
        u=u*(width*.5f);v=v*(depth*.5f);
        prism({a-u-v,a+u-v,a+u+v,a-u+v,b-u-v,b+u-v,b+u+v,b-u+v},col);
    }
    void blob(float width,float height,Color col,float lean=0) {
        constexpr int sides=12;
        const float radii[]={.8f,1.f,.78f,.22f};
        const float levels[]={1, .32f*height,.76f*height,height};
        for(int j=0;j<3;j++)for(int i=0;i<sides;i++){
            float a=i*2*PI/sides,b=(i+1)*2*PI/sides;
            auto point=[&](float angle,int ring){return P3{std::cos(angle)*width*radii[ring],std::sin(angle)*width*radii[ring]+lean*ring/3,levels[ring]};};
            face(point(a,j),point(b,j),point(b,j+1),point(a,j+1),col,.78f+j*.09f+std::cos(a)*.1f);
        }
    }
    bool contains(Vector2 point) const {
        for (const Face& face : faces) {
            const auto& p = face.points;
            if (CheckCollisionPointTriangle(point,p[0],p[1],p[2]) ||
                CheckCollisionPointTriangle(point,p[0],p[2],p[3])) return true;
        }
        return false;
    }
    void draw(Color outline=BLANK) {
        std::stable_sort(faces.begin(),faces.end(),[](const Face&a,const Face&b){return a.depth<b.depth;});
        // Outline pass goes behind the entire opaque body, so internal edges
        // are covered by the normal rendering instead of becoming a wireframe.
        if(outline.a)for(const Face& face:faces)for(int i=0;i<4;i++)
            DrawLineEx(face.points[i],face.points[(i+1)%4],4,outline);
        for(const auto& f:faces)poly({f.points[0],f.points[1],f.points[2],f.points[3]},f.color);
    }
};

// species: player, slime, wolf, skeleton, guardian, boar, spider, bandit, treant.
inline bool draw(int species,float x,float y,float size,const AnimationState& state,
                 float attack=0,bool helmet=false,bool shield=false,Kind weapon=Kind::Sword,bool flash=false,
                 Color outline=BLANK,const Vector2* probe=nullptr,float roll=-1,float swingProgress=-1) {
    bool rolling=species==0&&roll>=0;
    bool striking=species==0&&!rolling&&(swingProgress>=0||attack>0);
    float strikeProgress=swingProgress>=0?swingProgress:std::asin(std::clamp(attack,0.f,1.f))/PI;
    float tuck=rolling?std::clamp(std::min(roll,1-roll)/.18f,0.f,1.f):0;
    tuck=tuck*tuck*(3-2*tuck);
    const float cycle=state.phase*(2*PI);
    const float stride=state.moving?std::sin(cycle):0;
    const float lift=state.moving?std::abs(std::cos(cycle))*1.7f:std::sin(state.idleTime*2.4f)*.6f;
    const float bodyScale=species==4?1.38f:species==8?1.18f:1.f;
    const Color skin=flash?Text:rgb(212,180,131),bone=flash?Text:rgb(202,202,166);
    const Color metal=flash?Text:rgb(144,169,159),cloth=species==7?rgb(125,64,48):rgb(49,102,94),leather=rgb(88,68,48);
    const Color stone=flash?Text:rgb(104,133,123),fur=flash?Text:rgb(131,148,140);
    if(!probe){float w=size*(species==4?35:species==2?28:23),h=size*(species==4?15:11);
        ellipse(x,y+3,w*1.22f,h*1.18f,rgb(4,12,16,22));ellipse(x,y+2,w,h,rgb(4,12,16,45));ellipse(x,y+2,w*.7f,h*.72f,rgb(4,12,16,78));}
    Rig rig(x,y,size*bodyScale,state.direction);
    if(rolling)rig.setRoll(roll,tuck);

    if(species==1){
        float squash=(state.moving?std::sin(cycle):std::sin(state.idleTime*2.5f)*.18f)+attack*1.8f;
        float h=31+squash*4;
        float lean=(state.moving?std::cos(cycle)*3:0)+attack*7;
        rig.blob(23-squash*2,h,flash?Text:rgb(112,173,112),lean);
        // Eyes, muzzle, and a distinct ridge on the back make all directions readable.
        for(float r:{-7.f,7.f}){
            rig.box({r,17+lean*.6f,18+squash*2},{6,4,7},rgb(219,232,173));
            rig.box({r+1,19.3f+lean*.6f,18+squash*2},{3,1,5},Ink);
        }
        rig.box({0,21,9+squash},{8,2,2},rgb(44,78,55));
        rig.box({-5,-9,h-2},{7,6,3},rgb(158,199,130));
        rig.box({7,-5,h-4},{5,5,3},rgb(155,196,125));
        for(int n=0;n<5;n++){float r=-12+n*6.f;
            rig.form({r,-3+(n%2)*6.f,h-7-n%2*5.f},{4,4,3},rgb(178,219,145));}
        rig.form({-10,5,h*.66f},{5,4,8},rgb(195,233,176));
    } else if(species==2) {
        // Four legs use opposing diagonal pairs, rather than the humanoid gait.
        for(float r:{-8.f,8.f})for(float f:{-14.f,14.f}){
            float swing=state.moving?std::sin(cycle+(r*f>0?0:PI)):0;
            float stepLift=state.moving?std::max(0.f,std::cos(cycle+(r*f>0?0:PI)))*6:0;
            P3 foot{r,f+swing*6,3+stepLift};
            rig.limb(foot,{r,f,23+lift},5,6,shade(fur,.73f));
            rig.box(foot+P3{0,2,-1},{6,9,4},rgb(61,73,70));
        }
        rig.form({0,0,26+lift},{21,40,19},fur);
        rig.form({0,15,30+lift},{26,15,23},shade(fur,.86f));
        rig.form({0,23+attack*6,39+lift},{19,19,18},fur);
        rig.form({0,35+attack*8,33+lift},{12,16,10},shade(fur,1.1f));
        rig.box({0,43+attack*8,35+lift},{10,3,5},Ink);
        for(float r:{-7.f,7.f}){
            rig.spike({r,20,46+lift},{r*1.1f,18,59+lift},8,6,shade(fur,.8f));
            rig.box({r,22,51+lift},{3,2,7},rgb(66,78,76));
            rig.box({r,29+attack*6,41+lift},{4,2,3},Gold);
            rig.box({r,30.2f+attack*6,42+lift},{1.5f,1,1.5f},Text);
            rig.spike({r*.65f,37+attack*8,32+lift},{r*.7f,39+attack*8,27+lift},2.5f,2.5f,bone);
        }
        for(int n=0;n<7;n++)for(float side:{-1.f,1.f})
            rig.beam({side*(6+n%2),-14+n*5.f,34+lift},{side*10,-17+n*5.f,30+lift},2,1.5f,shade(fur,n%2?.78f:1.2f));
        rig.form({0,21,27+lift},{15,9,10},rgb(174,185,163));
        float wag=std::sin(state.idleTime*3)*3;
        rig.limb({0,-20,29+lift},{wag,-36,36+lift},8,7,shade(fur,.8f));
        rig.limb({wag,-36,36+lift},{wag*1.5f,-42,29+lift},5,5,shade(fur,.65f));
    } else if(species==5){
        Color hide=flash?Text:rgb(113,82,57);
        for(float r:{-9.f,9.f})for(float f:{-13.f,12.f}){
            float phase=cycle+(r*f>0?0:PI),step=state.moving?std::sin(phase)*5:0;
            P3 foot{r,f+step,3+(state.moving?std::max(0.f,std::cos(phase))*4:0)};
            rig.limb(foot,{r,f,21+lift},7,7,shade(hide,.7f));rig.box(foot,{8,9,5},Ink);
        }
        rig.form({0,-2,25+lift},{29,39,24},hide);rig.form({0,18+attack*8,27+lift},{24,21,23},shade(hide,.8f));
        rig.form({0,32+attack*10,19+lift},{18,12,12},rgb(157,105,78));
        for(float r:{-8.f,8.f}){
            rig.box({r,21+attack*8,34+lift},{4,3,4},Gold);
            rig.spike({r,31+attack*10,17+lift},{r*1.6f,35+attack*10,28+lift},5,5,flash?Text:rgb(226,209,158));
            rig.spike({r,11,39+lift},{r*1.3f,8,49+lift},9,8,shade(hide,.6f));
            rig.box({r*.55f,38+attack*10,21+lift},{3,1.5f,2},rgb(63,40,33));
        }
        for(int i=0;i<6;i++)rig.box({0,-16+i*6.f,41+lift},{6,5,7},shade(hide,.55f));
        for(int i=0;i<7;i++)for(float side:{-1.f,1.f})rig.beam({side*9,-17+i*5.f,34+lift},{side*13,-19+i*5.f,28+lift},1.5f,1.5f,shade(hide,i%2?.65f:1.16f));
        rig.beam({0,-23,25},{7,-31,28},3,3,hide);
    } else if(species==6){
        Color chitin=flash?Text:rgb(61,76,66),venom=rgb(160,196,77);
        for(int i=0;i<4;i++)for(float side:{-1.f,1.f}){
            float phase=cycle+i*PI*.5f+(side<0?PI:0),step=state.moving?std::sin(phase)*5:0;
            float f=-15+i*10.f;P3 root{side*10,f,14+lift},joint{side*23,f+side*4,21+lift},foot{side*31,f+step,2+(state.moving?std::max(0.f,std::cos(phase))*4:0)};
            rig.beam(root,joint,4,4,chitin);rig.beam(joint,foot,3,3,shade(chitin,.7f));
        }
        rig.form({0,-10,20+lift},{26,28,22},chitin);rig.form({0,13+attack*7,18+lift},{20,19,17},shade(chitin,1.15f));
        for(float side:{-1.f,1.f}){
            rig.box({side*5,23+attack*7,22+lift},{5,2,5},venom);
            rig.beam({side*5,22+attack*8,13+lift},{side*8,31+attack*10,6+lift},3,3,rgb(201,210,173));
        }
        rig.form({0,-11,32+lift},{8,15,2},venom);
        for(int i=0;i<4;i++)rig.beam({-9,-20+i*6.f,26+lift},{9,-20+i*6.f,26+lift},1.5f,1.5f,shade(chitin,1.4f));
        for(float side:{-1.f,1.f}){rig.form({side*3,24+attack*7,19+lift},{2.5f,2,2.5f},Gold);
            for(int i=0;i<4;i++)rig.form({side*23,-11+i*10.f,19+lift},{5,5,5},shade(chitin,1.2f));}
    } else if(species==8){
        Color bark=flash?Text:rgb(102,85,57),leaf=flash?Text:rgb(67,112,61);
        for(float side:{-1.f,1.f}){
            float step=stride*side;P3 foot{side*11,step*7,3},knee{side*9,step*3,23+lift};
            rig.beam(foot,knee,12,11,bark);rig.beam(knee,{side*8,0,39+lift},10,10,shade(bark,.8f));
            rig.beam(foot,{side*22,step*7+11,1},5,5,bark);
            P3 hand{side*29,4+attack*22-stride*side*4,47+lift};
            rig.beam({side*15,0,68+lift},{side*24,0,56+lift},9,9,bark);rig.beam({side*24,0,56+lift},hand,8,8,bark);
            for(int f=0;f<3;f++)rig.beam(hand+P3{side*f*2,0,-3},{side*(30+f*4.f),13+attack*26,36+lift},3,3,bark);
            rig.form({side*20,-3,76+lift},{24,20,13},leaf);
            rig.form({side*26,0,79+lift},{10,12,8},shade(leaf,1.26f));
        }
        rig.form({0,0,56+lift},{30,24,42},bark);rig.form({0,0,88+lift},{23,22,24},shade(bark,1.05f));
        for(float side:{-1.f,1.f})rig.box({side*6,12,91+lift},{5,2,5},rgb(216,203,100));
        rig.box({0,13,83+lift},{9,2,3},Ink);
        rig.beam({-8,0,97},{-17,-3,113+lift},6,5,bark);rig.beam({8,0,97},{19,2,110+lift},5,5,bark);
        rig.form({-15,-2,113+lift},{24,20,15},leaf);rig.form({13,2,111+lift},{26,22,17},shade(leaf,1.12f));
        for(int n=0;n<5;n++)rig.form({-20+n*9.f,-1+(n%2)*5,117+lift},{10,13,7},shade(leaf,1.1f+n*.055f));
        for(float side:{-1.f,1.f})for(int n=0;n<4;n++)rig.beam({side*12,5,36+n*9.f+lift},{side*13,2,42+n*9.f+lift},1.5f,2,shade(bark,.6f));
        for(int i=0;i<4;i++)rig.box({-10+i*6.f,13,54+lift},{2,2,25},shade(bark,.55f));
    } else {
        bool skeleton=species==3,golem=species==4;
        Color body=golem?stone:skeleton?bone:species==7?(flash?Text:rgb(106,74,51)):metal;
        float thick=golem?12:skeleton?4:7;
        float spacing=golem?10:7;
        for(int i=0;i<2;i++){
            float side=i==0?-1.f:1.f,wave=stride*side;
            float stepLift=state.moving?std::max(0.f,std::cos(cycle)*side)*7:0;
            P3 foot{side*spacing,wave*7+5*tuck,4+stepLift+12*tuck};
            P3 knee{side*spacing,wave*3+14*tuck,16+lift+5*tuck};
            rig.limb(foot,knee,thick,thick,skeleton?bone:golem?stone:rgb(51,66,62));
            rig.limb(knee,{side*spacing,0,29+lift},thick,thick,skeleton?bone:golem?stone:rgb(66,79,70));
            rig.box(foot+P3{0,3,-2},{thick+2,thick+7,5},skeleton?bone:golem?shade(stone,.78f):leather);
        }
        rig.box({0,0,28+lift},{golem?29.f:20.f,13,8},skeleton?bone:golem?stone:leather);
        if(skeleton){
            rig.box({0,-2,43+lift},{4,5,26},bone);
            for(int j=0;j<4;j++)rig.box({0,1,35+j*5.f+lift},{20-j,11,2.8f},bone);
        } else {
            rig.form({0,0,43+lift},{golem?35.f:25.f,golem?21.f:15.f,27},body);
            if(!golem){
                rig.box({0,-10,37+lift},{28,3,38},cloth);
                rig.box({0,-12,53+lift},{22,3,8},shade(cloth,1.2f));
                rig.box({0,8.1f,31+lift},{26,2,4},leather);
                rig.box({0,9.5f,31+lift},{5,2,5},Gold);
                rig.limb({-9,8.2f,52+lift},{9,8.2f,35+lift},3,2,leather);
                for(float side:{-1.f,1.f}){
                    rig.beam({side*8,7,36+lift},{side*8,7,49+lift},1.2f,1.2f,shade(body,1.3f));
                    rig.box({side*9,-12.2f,48+lift},{2,1,3},Gold);
                    rig.form({side*9,1,29+lift},{5,8,6},leather);
                }
                if(species==0){rig.form({0,8,46+lift},{6,2,7},shade(metal,1.22f));rig.box({0,9.3f,46+lift},{2,1,4},Gold);}
            }else{
                rig.box({0,11,44+lift},{8,3,16},Gold);
                rig.box({0,13,44+lift},{3,2,9},rgb(255,220,141));
                rig.box({-11,11,51+lift},{2,2,8},shade(stone,.55f));
                for(int n=0;n<4;n++)rig.beam({-12+n*7.f,10,34+lift},{-9+n*7.f,10,42+n%2*7.f+lift},1.3f,1.5f,shade(stone,.52f));
                rig.form({10,-5,55+lift},{9,8,3},rgb(94,139,77));
            }
        }
        for(int i=0;i<2;i++){
            float side=i==0?-1.f:1.f,reach=side>0?attack*18:attack*6;
            float shoulder=side*(golem?24.f:17.f);
            P3 hand{side*(golem?26.f:20.f),3-stride*side*5+reach,32+lift+std::abs(attack)*11};
            P3 elbow{shoulder,1-stride*side*2+reach*.4f,40+lift};
            WeaponPose strike{};
            if(striking&&side>0){strike=forwardStrike(strikeProgress,lift);hand=strike.hand;elbow=strike.elbow;}
            hand=hand+P3{-side*5*tuck,9*tuck,6*tuck};
            elbow=elbow+P3{0,7*tuck,-4*tuck};
            rig.limb({shoulder,0,51+lift},elbow,thick,thick,skeleton?bone:body);
            rig.limb(elbow,hand,thick-1,thick-1,skeleton?bone:golem?stone:skin);
            rig.form({shoulder,0,52+lift},{golem?17.f:skeleton?6.f:12.f,16,8},body);
            rig.box(hand,{golem?13.f:6.f,golem?12.f:6.f,golem?13.f:6.f},skeleton?bone:golem?stone:skin);
            if(!skeleton&&!golem){rig.form(elbow,{9,8,7},body);rig.form(hand+P3{0,-1,5},{7,7,3},leather);
                rig.beam({shoulder-4,5,53+lift},{shoulder+4,5,53+lift},1.5f,1,Gold);}
            if(side>0&&!golem){
                P3 base=hand+P3{0,1,-3},tip=hand+P3{0,8+attack*8,31-std::abs(attack)*10};
                bool dagger=weapon==Kind::Dagger||species==7;
                if(dagger)tip=hand+(tip-hand)*.65f;
                if(striking){
                    base=hand-strike.blade*.12f;tip=hand+strike.blade*(dagger?.65f:1.f);
                    rig.beam(base,tip,weapon==Kind::Axe?3:4,2,weapon==Kind::Axe?leather:rgb(201,216,209));
                    rig.beam(base+P3{0,0,1.2f},tip+P3{0,0,1.2f},.9f,.6f,Text);
                    P3 guard=hand+strike.blade*.12f;
                    float flat=std::hypot(strike.blade.r,strike.blade.f);
                    P3 cross{strike.blade.f/flat*7,-strike.blade.r/flat*7,0};
                    rig.beam(guard-cross,guard+cross,3,3,Gold);
                }else{
                    rig.limb(base,tip,weapon==Kind::Axe?3:4,2,weapon==Kind::Axe?leather:rgb(201,216,209));
                    rig.box(hand+P3{0,2,4},{14,3,3},Gold);
                }
                if(weapon==Kind::Axe)rig.box(tip+P3{5,0,-7},{15,4,14},metal);
            }
            if(side<0&&shield&&!skeleton&&!golem){
                P3 p=hand+P3{-2,7,1};
                rig.form(p,{19,4,27},Gold);rig.form(p+P3{0,2.5f,0},{15,2,22},leather);
                rig.box(p+P3{0,4,0},{3,2,20},metal);rig.box(p+P3{0,4,2},{15,2,3},metal);
                rig.form(p+P3{0,5.5f,2},{6,2,7},Gold);
            }
        }
        float headZ=66+lift;
        rig.form({0,0,headZ},{golem?24.f:17.f,golem?22.f:16.f,golem?21.f:18.f},golem?stone:skeleton?bone:skin);
        if(!skeleton&&!golem){
            Color hood=helmet?metal:cloth;
            rig.form({0,-1,headZ+8},{20,19,7},hood);
            rig.box({0,-8,headZ},{19,4,18},hood);
            if(helmet)rig.box({0,9,headZ+1},{3,2,16},Gold);
            for(float side:{-1.f,1.f}){rig.beam({side*7,7,headZ+8},{side*9,5,headZ-6},1.2f,1.2f,shade(hood,1.32f));
                rig.box({side*4.5f,8.6f,headZ+4.5f},{4,1,1},shade(leather,.75f));}
        }
        for(float r:{-4.5f,4.5f})rig.box({r,golem?11.4f:8.5f,headZ+2},{skeleton?5.f:golem?6.f:2.7f,1.5f,skeleton?5.f:3.f},golem?Gold:Ink);
        if(skeleton){
            rig.box({0,8.8f,headZ-3},{3,1,3},Ink);
            for(int i=0;i<4;i++)rig.box({-5.f+i*3,8.4f,headZ-7},{2,2,3},shade(bone,1.13f));
        }else if(!golem)rig.box({0,10,headZ-2},{3,3,4},shade(skin,.94f));
        if(skeleton){rig.beam({-8,4,headZ+7},{-2,7,headZ+5},1,1,shade(bone,.55f));rig.form({0,8,headZ-5},{12,4,4},shade(bone,.85f));}
    }
    if(probe)return rig.contains(*probe);
    if(striking&&strikeProgress>.10f&&strikeProgress<.9f){
        // The ribbon follows recent blade positions in the actor's isometric
        // plane. It stays in front of the body instead of circling its waist.
        float start=std::max(.06f,strikeProgress-.19f);
        for(int i=0;i<10;i++){
            float a=start+(strikeProgress-start)*i/10,b=start+(strikeProgress-start)*(i+1)/10;
            auto first=forwardStrike(a,lift),second=forwardStrike(b,lift);
            Vector2 innerA=rig.project(first.hand+first.blade*.52f),outerA=rig.project(first.hand+first.blade*1.04f);
            Vector2 innerB=rig.project(second.hand+second.blade*.52f),outerB=rig.project(second.hand+second.blade*1.04f);
            float opacity=std::sin(strikeProgress*PI)*(.1f+i*.043f);
            poly({innerA,outerA,outerB,innerB},Fade(Gold,opacity));
            DrawLineEx(outerA,outerB,1.5f*size,Fade(Text,opacity));
        }
    }
    rig.draw(outline);
    return false;
}
}
