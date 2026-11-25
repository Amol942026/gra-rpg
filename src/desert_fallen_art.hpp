#pragma once

// Included after the live desert rigs. These poses reuse their painted material
// cells and are drawn into the corpse cache from the same eight camera headings.
namespace desert_fallen_art {

struct Pose {
 float side;
 float time=1,fold=1,recoil=0;
 AnimationState state{};
 float attack=0;
 bool flash=false;
 float special=-1;
 float lower(float begin,float end)const{return enemy_death::window(time,begin,end);}
 float stride()const{return (state.motionInitialized?state.moveBlend:(state.moving?1.f:0.f))*(1-fold);}
 P3 point(float r,float f,float z)const{return {r*side,f,z};}
 hero_motion::Transform tilt(P3 center,float bank,float twist=0,float pitch=0)const{
  using namespace hero_motion;
  auto orientation=mul(axisAngle({0,0,1},twist*side),axisAngle({0,1,0},bank*side));
  return {{center.r*side,center.f,center.z},mul(orientation,axisAngle({1,0,0},pitch))};
 }
};

inline void scorpion(Rig& rig,Pose pose){
 const Color shell=pose.flash?Text:rgb(172,111,59),amber=rgb(222,153,79),joint=rgb(105,71,47);
 const Color groove=rgb(108,64,39),lip=rgb(237,173,97);
 const auto p=[&](float r,float f,float z){return pose.point(r,f,z);};
 const auto mix=[](P3 a,P3 b,float t){return a*(1-t)+b*t;};
 const float walk=pose.stride();
 const float walkBob=creature_motion::sample(3,pose.state.phase).bob*walk;
 const float front=pose.attack*(1-pose.fold);
 for(int i=0;i<4;i++)for(float side:{-1.f,1.f}){
  const float f=-19+i*10.f;
  const bool uphill=side<0;
  const auto step=creature_motion::sample(3,pose.state.phase+i*.25f+(side*pose.side<0?.5f:0));
  const float reach=step.travel*walk,fold=pose.lower(uphill?.28f:.09f,uphill?.85f:.72f);
  const auto root=p(side*9,f,13+walkBob+(uphill?-3:-6)*pose.fold);
  const auto knee=mix(p(side*(24+reach*2),f-4+reach*4,8),p(side*(uphill?23:26),f+(uphill?-10:-5),uphill?7.2f:5.f),fold);
  const auto shin=mix(p(side*(31+reach*2),f-10+reach*4,3+step.lift*walk*4),p(side*(uphill?31:32),f+(uphill?-5:1),3.1f),fold);
  const auto tip=mix(shin+p(side*2,-3,-1),p(side*(uphill?29:27),f+(uphill?1:7),1.2f),fold);
  rig.tapered(root,knee,4.65f,4.5f,shell,.76f,8);
  rig.rounded(knee,{4.1f,4.4f,4.2f},shade(joint,1.15f));
  rig.tapered(knee,shin,3.9f,3.55f,shade(shell,.93f),.57f,8);
  rig.tapered(shin,tip,2.2f,2.2f,groove,.13f);
 }
 const auto bank=pose.tilt({pose.recoil*2,-6,16.3f+walkBob-3.6f*pose.fold},21*pose.fold-6*pose.recoil);
 rig.setAttachment(&bank);
 rig.rounded({0,0,-1},{31,40,22},shade(shell,.83f),108);
 for(int i=0;i<5;i++){
  const float f=-15+i*7.f,width=25-std::abs(i-2)*2.6f;
  rig.rounded({0,f,3.2f},{width,12,18-std::abs(i-2)*1.3f},shade(shell,1.03f+i*.025f),108);
  rig.tube({{-width*.38f,f,8},{-width*.21f,f-1.1f,11},{0,f-1.2f,12.4f},{width*.21f,f-1.1f,11},{width*.38f,f,8}},1.25f,1.25f,amber);
 }
 rig.setAttachment(nullptr);
 const auto head=pose.tilt({2*pose.fold,14+3*front,13.5f-4*pose.fold},13*pose.fold,8*pose.fold);
 rig.setAttachment(&head);
 rig.rounded({0,0,0},{25,19,15},shade(shell,.86f),108);
 rig.rounded({0,6,-1},{15,10,10},shade(shell,1.05f),108);
 for(float side:{-1.f,1.f}){
  rig.tapered({side*3,7,5.2f},{side*7,5.8f,5.9f},1.05f,1.05f,groove);
  rig.spike({side*3,10,0},{side*5,14,-.5f},2.6f,2.8f,groove);
 }
 rig.setAttachment(nullptr);
 for(float side:{-1.f,1.f}){
  const float release=pose.lower(side<0?.28f:.18f,.89f);
  const float r=side*(25+(side<0?0:3)*release);
  const float f=38+13*front-(side<0?5:12)*release,z=16-(side<0?7.4f:10.2f)*release;
  const auto palm=p(r,f,z),elbow=p(side*(24-7*release),f-12,13-(side<0?5:7)*release);
  rig.tapered(p(side*8,18,12-4*pose.fold),elbow,7.2f,6.1f,shell,.88f,8);
  rig.rounded(elbow,{7.7f,8.5f,7.2f},joint,108);
  rig.tapered(elbow,palm,6.9f,6.6f,shell,1.13f,8);
  auto hinge=pose.tilt({r,f,z},-side*16*release,side*12*release);
  rig.setAttachment(&hinge);
  rig.rounded({0,0,0},{16,20,12.8f},shade(shell,1.23f),108);
  rig.tapered({-4.8f,5,1.1f},{-7.f,14,.1f},6.9f,6.3f,amber,.62f,8);
  rig.spike({-7,13.8f,.1f},{-3.8f,19,-1.65f},4.3f,4.1f,lip);
  rig.tapered({5.3f,5,1.8f},{7.1f,11,.9f},6.2f,6.1f,shell,.63f,8);
  rig.spike({7.1f,11,.9f},{3.4f,16,-1.5f},3.8f,3.8f,amber);
  rig.setAttachment(nullptr);
 }
 // The tail folds beside the thorax rather than losing the shape of its joints.
 constexpr std::array<P3,8> tail{{{0,-24,12},{3,-33,16},{13,-42,22},{24,-43,26},
                                  {34,-39,23},{40,-30,16},{39,-20,8},{34,-13,4.1f}}};
 P3 previous{};
 for(std::size_t i=0;i<tail.size();i++){
  const auto item=tail[i];const float n=float(i),a=n*.34f;
  const float unwind=pose.lower(.05f+n*.025f,.68f+n*.025f);
  const auto raised=p(std::sin(pose.state.idleTime*1.8f)*n*.45f*(1-pose.fold)*pose.side+n*pose.recoil,
                      -25-std::sin(a)*25+front*n*5,18+n*8-std::max(0.f,n-4)*9);
  const auto center=mix(raised,p(item.r,item.f,item.z),unwind);
  const float width=11.7f-float(i)*.84f;
  if(i)rig.tapered(previous,center,width*.69f,width*.63f,joint);
  rig.rounded(center,{width,width+1,width*.86f},shade(shell,1.02f+.024f*float(i)),108);
  rig.rounded(center+P3{0,-.7f,width*.40f},{width*.50f,width*.47f,1.7f},shade(amber,.99f));
  previous=center;
 }
 rig.spike(previous,previous+mix(p(0,14,-12),p(-10,2,-2.65f),pose.lower(.27f,.89f)),5.3f,4.1f,rgb(104,60,36));
}

inline void scarab(Rig& rig,Pose pose){
 const Color copper=pose.flash?Text:rgb(167,95,57),bright=rgb(222,157,101),under=rgb(52,42,46);
 const Color patina=rgb(51,116,107),deep=rgb(42,68,69);
 const Color seam=rgb(64,39,40);
 const auto p=[&](float r,float f,float z){return pose.point(r,f,z);};
 const auto& ability=enemyAbilityProfile(9);
 const float recover=pose.special-ability.windup;
 const float charge=(pose.special<0?0:recover<0?enemy_death::window(pose.special,0,ability.windup):
                  1-enemy_death::window(recover,0,.24f))*(1-pose.fold);
 const float stomp=(pose.special<0?0:enemy_death::window(recover,0,.085f)*
                         (1-enemy_death::window(recover,.085f,.30f)))*(1-pose.fold);
 const float walk=pose.stride()*(1-charge),front=pose.attack*(1-pose.fold);
 const float walkBob=std::cos(pose.state.phase*4*PI)*.85f*walk;
 const auto walkAxis=desert_motion::scarabStepAxis(pose.state);
 const float tip=pose.lower(.18f,.88f),release=pose.lower(.03f,.24f)*(1-pose.lower(.28f,.60f));
 for(int i=0;i<3;i++)for(float side:{-1.f,1.f}){
  const float f=-16+i*13.7f;const bool uphill=side<0;
  const auto step=desert_motion::sample(pose.state.phase+(i%2?.5f:0)+(side*pose.side<0?.5f:0));
  const float fold=pose.lower(uphill?.35f:.11f,uphill?.94f:.73f);
  const float stride=step.travel*walk*desert_motion::ScarabStride;
  const float right=stride*walkAxis.x*pose.side,forward=stride*walkAxis.y;
  const float lift=step.lift*walk*8.3f;
  const float brace=(i==2?charge*3+stomp*3:0);
  const auto root=p(side*11.5f,f,17+walkBob-(uphill?3:9)*tip);
  const auto knee=p(side*(25+brace-fold*(uphill?6:1.5f))+right*.56f,
                    f+forward*.58f-fold*(i==1?5:8),10+lift*.24f+3*release-(uphill?2.4f:6)*fold);
  const auto ankle=p(side*(33+brace-(uphill?12:3)*fold)+right,f+forward-7+(uphill?10:14)*fold,
                    2.7f+lift+release*(uphill?6:2)+(uphill?2:0)*tip);
  rig.tapered(root,knee,5.3f,4.8f,deep,.83f);
  rig.tapered(root+(knee-root)*.20f,knee,4.8f,4.6f,copper,.73f,9);
  rig.rounded(knee,{5.3f,4.9f,5.0f},under);
  rig.tapered(knee,ankle,4.15f,3.4f,shade(copper,1.12f),.48f);
  rig.tapered(ankle,ankle+p(-side*4,4+fold*(uphill?1:3),-1.25f),2.2f,2.0f,under,.22f);
  if(i!=1)rig.spike(ankle,ankle+p(side*2.0f,-2,-.65f),1.4f,1.4f,patina);
 }
 const auto bank=pose.tilt({2*pose.recoil+3*tip,-3+2*tip,25+walkBob-10.1f*tip+4.5f*charge-2.8f*stomp},
                            48*tip-12*pose.recoil,-12*tip,13*charge-7*stomp-5*tip);
 rig.setAttachment(&bank);
 rig.rounded({0,-1,-5.5f},{38,44,16},under);
 rig.rounded({0,-1,-1},{41,48,24},deep);
 for(float side:{-1.f,1.f}){
  const float gap=1.6f*tip+1.15f*charge;
  rig.rounded({side*(9.6f+gap),-3+side*.9f,3+side*.48f},{18.7f,42.8f,28.5f},copper,109);
  rig.tube({{side*6,-23,7},{side*13,-19,10.7f},{side*17,-12,13},
            {side*18,-3,14},{side*16,7,13.1f},{side*11,15,9}},1.35f,1.34f,bright);
  for(int i=0;i<4;i++){
   const float f=-14+i*6.5f;
   const float bow=std::sin((i+1)/5.f*PI);
   const P3 a{side*(5+gap),f+2,13.4f+bow*3.5f};
   const P3 b{side*(10.5f+gap),f+.3f,14.8f+bow*2.6f};
   const P3 c{side*(16+gap),f-3,10.6f+bow*3.6f};
   rig.tube({a,b,c},.72f,.76f,i%2?shade(bright,.94f):patina);
  }
 }
 rig.tube({{0,-23,6},{0,-15,11},{0,-4,15.5f},{0,8,13.7f},{0,16,9}},1.4f,1.29f,seam);
 rig.rounded({0,15,1.5f},{30,18,20.5f},patina,109,false,-1,false,true);
 rig.tube({{-12,16,7},{-7,14,11},{0,14,12.3f},{7,14,11},{12,16,7}},
          1.5f,1.5f,bright);
 rig.setAttachment(nullptr);
 const auto head=pose.tilt({5*tip,20+4.5f*front+stomp*3,19-11.9f*tip+6*charge-3.6f*stomp},
                           19*tip,-16*tip,11*charge-9*stomp);
 rig.setAttachment(&head);
 rig.rounded({0,0,0},{23,20,14},deep);
 rig.rounded({0,2,2.6f},{18,16,10.7f},shade(copper,1.16f),109);
 for(float side:{-1.f,1.f}){
  const float glow=(1-tip)*(.35f+.65f*std::max(charge,stomp));
  const P3 eye{side*7.3f,6.5f,5.4f};
  rig.rounded(eye,{5.3f,5.4f,3.3f},under);
  rig.rounded(eye+P3{side*.4f,1.3f,.6f},{3.4f,3.1f,2.35f},rgb(67+int(glow*94),108+int(glow*124),95+int(glow*105)));
  const P3 jaw{side*6.9f,7,-1},elbow{side*(10.7f+3*charge),14+6*front,-1.5f+3*front};
  rig.tapered(jaw,elbow,4.6f,3.9f,copper,.62f);
  rig.spike(elbow,{side*(6.5f+4*charge),19+8*front,-1+2.5f*front},2.6f,2.75f,bright);
  rig.tube({{side*6,6.8f,5.9f},{side*10,11,5.9f-2.5f*tip},
            {side*14,13.2f,4.8f-3.3f*tip}},1.25f,1.2f,patina);
 }
 rig.setAttachment(nullptr);
}

template<size_t N> inline float cobraCurve(const float (&keys)[N],float t){
 const float frame=std::clamp(t,0.f,1.f)*(N-1);
 const size_t index=std::min(size_t(frame),N-2);
 return keys[index]+(keys[index+1]-keys[index])*(frame-index);
}

struct CobraVertex {P3 point;Vector2 uv;float light;};

inline void cobraPlate(Rig& rig,CobraVertex a,CobraVertex b,CobraVertex c,CobraVertex d,int cell,Color color){
 rig.paintedCreatureQuad({a.point,b.point,c.point,d.point},{a.uv,b.uv,c.uv,d.uv},
                         {a.light,b.light,c.light,d.light},cell,color);
}

inline void cobraHood(Rig& rig,bool flash){
 constexpr std::array<float,8> Length{14,10,5,-1,-7,-13,-19,-22};
 constexpr std::array<float,8> Width{5.4f,12.3f,18.7f,22,20.9f,15.7f,8.3f,4.1f};
 constexpr int Columns=6;
 using Sheet=std::array<std::array<CobraVertex,Columns+1>,Length.size()>;
 Sheet front{},back{};
 for(size_t row=0;row<Length.size();row++)for(int i=0;i<=Columns;i++){
  const float t=2.f*i/Columns-1,side=t*t;
  const float r=t*Width[row],f=Length[row],bow=side*3.3f;
  const float thickness=.63f+1.35f*(1-side);
  const Vector2 uv{.5f+r/46,(14-f)/36};
  front[row][i]={{r,f,bow-thickness},uv,.97f+.10f*(1-side)-.055f*t};
  back[row][i]={{r,f,bow+thickness},uv,.91f+.09f*(1-side)-.048f*t};
 }
 const int chest=flash?-1:creature_paint::CobraHoodFront,crown=flash?-1:creature_paint::CobraHoodBack;
 for(size_t row=0;row+1<Length.size();row++)for(int col=0;col<Columns;col++){
  cobraPlate(rig,front[row][col],front[row][col+1],front[row+1][col+1],front[row+1][col],
             chest,flash?Text:rgb(183,118,69));
  cobraPlate(rig,back[row][col+1],back[row][col],back[row+1][col],back[row+1][col+1],
             crown,flash?Text:rgb(132,80,51));
 }
 const Color rim=flash?Text:rgb(151,96,61);
 for(size_t row=0;row+1<Length.size();row++){
  rig.face(back[row][0].point,front[row][0].point,front[row+1][0].point,back[row+1][0].point,rim);
  rig.face(front[row][Columns].point,back[row][Columns].point,back[row+1][Columns].point,front[row+1][Columns].point,rim);
 }
 for(int col=0;col<Columns;col++){
  rig.face(front.front()[col].point,back.front()[col].point,back.front()[col+1].point,front.front()[col+1].point,rim);
  rig.face(back.back()[col].point,front.back()[col].point,front.back()[col+1].point,back.back()[col+1].point,rim);
 }
}

inline void cobraSkull(Rig& rig,const hero_motion::Transform& attachment,bool flash){
 struct Profile {float f,width,center,height;};
 constexpr Profile Rings[]{{-10,1.2f,1.4f,1.6f},{-7.3f,6.4f,1.9f,4.3f},{-2,9.4f,2,5.4f},
                           {3.8f,8.3f,1.8f,4.5f},{8.3f,6.4f,1.05f,3.3f},{11.8f,4.9f,.65f,2.5f},
                           {13.4f,2.8f,.65f,1.5f}};
 constexpr int Sides=12;
 static const auto circle=[](){
  std::array<Vector2,Sides> ring{};
  for(int i=0;i<Sides;i++)ring[i]={std::cos(i*2*PI/Sides),-std::sin(i*2*PI/Sides)};
  return ring;
 }();
 std::array<std::array<CobraVertex,Sides>,std::size(Rings)> vertices{};
 for(size_t row=0;row<std::size(Rings);row++)for(int i=0;i<Sides;i++){
  const auto shape=Rings[row];const auto p=circle[i];
  const auto n=hero_motion::rotate(attachment.q,{p.x,0,p.y});
  vertices[row][i]={{p.x*shape.width,shape.f,shape.center+p.y*shape.height},
                    {float((i+9)%Sides)/Sides,(shape.f-Rings[0].f)/23.4f},
                    .29f+.74f*rig.surfaceLight({n.x,n.y,n.z})};
 }
 const int plate=flash?-1:creature_paint::CobraHead;
 const Color brass=flash?Text:rgb(180,113,66);
 for(size_t row=0;row+1<std::size(Rings);row++)for(int i=0;i<Sides;i++){
  const int next=(i+1)%Sides;
  auto b=vertices[row][next],c=vertices[row+1][next];
  b.uv.x=c.uv.x=vertices[row][i].uv.x+1.f/Sides;
  cobraPlate(rig,vertices[row][i],b,c,vertices[row+1][i],plate,brass);
 }
 for(int i=0;i<Sides;i++){
  const int next=(i+1)%Sides;
  const P3 snout{0,Rings[std::size(Rings)-1].f,.65f},nape{0,Rings[0].f,1.4f};
  rig.face(vertices.back()[i].point,vertices.back()[next].point,snout,snout,brass,.88f);
  rig.face(vertices.front()[next].point,vertices.front()[i].point,nape,nape,brass,.74f);
 }
}

inline void cobra(Rig& rig,Pose pose){
 rig.setEdgeCoverage();
 const Color scales=pose.flash?Text:rgb(136,83,51),belly=rgb(198,144,92);
 const Color dark=rgb(69,37,30),ivory=rgb(235,209,168);
 const auto p=[&](float r,float f,float z){return pose.point(r,f,z);};
 const auto mix=[](P3 a,P3 b,float t){return a*(1-t)+b*t;};
 const float walk=pose.stride(),body=pose.lower(.16f,.91f);
 const float strike=(pose.special>=0?cobraCurve(cobra_curve_data::cast,pose.special/enemyAbilityProfile(SandCobraType).duration()):pose.attack)*(1-pose.fold);
 const float phase=pose.state.phase;
 const auto sway=[&](float offset){
  const float cycle=phase-offset;
  return cobraCurve(cobra_curve_data::sway,cycle-std::floor(cycle));
 };
 std::vector<P3> spine;
 std::vector<float> widths;
 constexpr int trunkRings=25,neckRings=8;
 spine.reserve(trunkRings+neckRings+1);
 widths.reserve(trunkRings+neckRings+1);
 for(int i=0;i<trunkRings;i++){
  const float t=float(i)/(trunkRings-1),angle=.38f+t*PI*1.75f;
  const float r=8.5f+16.5f*t,f=8+13*t;
  const float back=1-t;
  const P3 coiled{std::cos(angle)*(23-8*t),std::sin(angle)*(19-8*t)-9,2.4f+7.f*t};
  const auto raised=mix(coiled,{0,-8-24*back,2.4f+7.f*t},enemy_death::window(t,.71f,1));
  const P3 slither{sway(back*.73f)*(.8f+7.7f*back),-8-56*back,2.4f+7.f*t*t};
  spine.push_back(mix(mix(raised,slither,walk),p(std::cos(angle)*r,std::sin(angle)*f-13,2.f+6.8f*t),body));
  widths.push_back(enemy_death::window(t,0,.72f)*.95f);
 }
 const P3 neckBase=spine.back();
 for(int i=1;i<=neckRings;i++){
  const float n=float(i)/neckRings,fold=pose.lower(.27f-.17f*n,.93f-.09f*n);
  const float idle=std::sin(pose.state.idleTime*1.5f)*.55f*(1-body);
  const P3 raised{neckBase.r*(1-n)+n*(idle+sway(0)*walk*.4f+pose.recoil*pose.side*9),
                  -8+16*n+strike*n*n*24,9.4f+n*60+strike*n*n*3};
  const P3 settled=p(24+11*n-28*n*n,-14+n*58,8.8f+5*std::sin(n*PI)-2.3f*n);
  spine.push_back(mix(raised,settled,fold));
  widths.push_back(.95f+.17f*std::sin(n*PI));
 }
 const auto upperNeck=spine.back();
 const auto collar=spine[spine.size()-3];
 spine.push_back(upperNeck+P3{0,3,3});
 widths.push_back(0);
 const int skin=(creature_paint::cobraSkin.id||creature_paint::solidCobraSkin)&&!pose.flash?creature_paint::CobraCell:-1;
 rig.tube(spine,13.2f,12.4f,scales,skin,true,widths);
 const float hoodFold=pose.lower(.18f,.88f);
 const auto hoodCenter=mix(P3{collar.r*pose.side,collar.f-2,collar.z-4},P3{14.5f,29.7f,11.4f},hoodFold);
 const auto hood=pose.tilt(hoodCenter,47*hoodFold,-25*hoodFold,74*(1-hoodFold));
 rig.setAttachment(&hood);
 cobraHood(rig,pose.flash);
 rig.setAttachment(nullptr);
 const float headFold=pose.lower(.10f,.84f);
 const auto headCenter=mix(P3{upperNeck.r*pose.side,upperNeck.f+6+strike*3,upperNeck.z+6},P3{6.7f,43.2f,8.15f},headFold);
 const auto head=pose.tilt(headCenter,37*headFold,-22*headFold,10*(1-headFold));
 rig.setAttachment(&head);
 cobraSkull(rig,head,pose.flash);
 rig.rounded({0,5.6f,-2.8f},{15.8f,15.6f,5.7f},belly,
             pose.flash?-1:100+creature_paint::CobraJaw,false,-1,true);
 rig.tube({{-7.3f,1.8f,-1.3f},{-7.1f,5.6f,-1.5f},{-5.5f,10.2f,-1.4f},
           {0,13,-1.2f},{5.5f,10.2f,-1.4f},{7.1f,5.6f,-1.5f},{7.3f,1.8f,-1.3f}},1.1f,.8f,dark);
 for(float side:{-1.f,1.f}){
  rig.rounded({side*3.85f,11.5f,2.4f},{1.7f,1.7f,1.2f},dark);
  rig.tapered({side*5.5f,7.4f,4.8f},{side*7.9f,3.5f,4.8f},1.85f,1.36f,rgb(118,65,40),1.03f);
  const float eyeOpen=1-headFold;
  if(eyeOpen>.01f){
   rig.rounded({side*7.45f,5.2f,3.75f},{3.6f,4.8f,3.35f*eyeOpen},dark);
   rig.rounded({side*8.35f,5.4f,3.78f},{2.55f,3.65f,2.45f*eyeOpen},rgb(242,176,64));
   rig.rounded({side*9.17f,5.7f,3.85f},{1.13f,1.05f,2.45f*eyeOpen},rgb(47,27,22));
  }
  rig.spike({side*4.7f,9.7f,-1.24f},{side*4.9f,10.4f,-4.35f},1.8f,1.55f,ivory);
 }
 rig.setAttachment(nullptr);
}

inline void dune(Rig& rig,Pose pose){
 const Color sand=pose.flash?Text:rgb(219,175,108),dark=pose.flash?Text:rgb(144,93,54),amber=rgb(246,202,105);
 const float walk=pose.stride(),front=pose.attack*(1-pose.fold);
 const float collapse=pose.lower(.06f,.91f);
 const float sway=std::sin(pose.state.idleTime*1.5f)*3*(1-pose.fold)
                  +creature_motion::sample(5,pose.state.phase).sway*walk*4-8*pose.recoil;
 const auto at=[&](P3 p){return sandCollapse(p,collapse,pose.side);};
 sandMound(rig,sway,sand,collapse,pose.side);
 // The limbs run into the same sand surface as the moving crest.
 const float sink=pose.lower(.12f,.83f),shrink=1-sink;
 if(shrink>.001f){
  for(int i=0;i<6;i++){
   const float side=i%2?1.f:-1.f,f=-29+i/2*25.f;
   const float step=creature_motion::sample(5,pose.state.phase+i/6.f).travel*walk;
   const auto placement=desert_motion::duneFoot(i,step,sink);
   const auto knee=at({side*(43+step*5-19*sink),f,14}),foot=at({placement.right,placement.forward,3});
   rig.rounded(knee,P3{30,25,25}*shrink,sand,111);
   rig.tapered(knee,foot,21*shrink,18*shrink,dark,.35f);
  }
  for(float side:{-1.f,1.f}){
   const auto shoulder=at({side*42,6,49+sway});
   const auto hand=at({side*(64-front*29-25*sink),27+front*48-12*sink,16+front*15});
   rig.tapered(shoulder,hand,28*shrink,23*shrink,sand,.35f);
   for(int i=0;i<4;i++)rig.form(hand+P3{(i-1.5f)*7,10+i*3.f,-4+i%2*3.f}*shrink,
                               P3{6,8,4}*shrink,shade(sand,.83f+i*.08f));
  }
 }
 const float open=1-pose.lower(.16f,.78f);
 for(float side:{-1.f,1.f}){
  const auto eye=at(sandFront(-13+side*13,83+sway+front*5,sway,1.5f));
  rig.rounded(eye,{14,4,1.6f+8.4f*open},dark);
  if(open>.001f){
   rig.rounded(eye+P3{0,2.6f*open,0},{9*open,3*open,5*open},rgb(255,223,118));
   rig.rounded(eye+P3{side,4.2f,1}*open,{4*open,2*open,2*open},Text);
  }
  rig.beam(at(sandFront(-13+side*6,91+sway+front*5,sway,1)),
           at(sandFront(-13+side*21,94+sway+front*5,sway,1)),1.2f+2.8f*open,1.2f+2.8f*open,sand);
 }
 rig.rounded(at(sandFront(-13,67+sway+front*5,sway,1)),{31,3,1.3f+4.7f*open},dark);
 if(open>.001f)for(int i=0;i<7;i++){
  const auto tooth=at(sandFront(-25+i*4.f,70+sway+front*5,sway,3));
  rig.spike(tooth,tooth+P3{0,1,-6}*open,1.8f*open,1.8f*open,amber);
 }
}

inline void drawPose(Rig& rig,int type,const Pose& pose){
 rig.setCreatureSurfaces();
 if(pose.time>0)rig.setGroundBounded();
 switch(type){
 case 8:scorpion(rig,pose);break;
 case CopperScarabType:scarab(rig,pose);break;
 case SandCobraType:cobra(rig,pose);break;
 case DesertBossType:dune(rig,pose);break;
 }
}
}

inline void buildDesert(Rig& rig,int type,const AnimationState& state,float attack,bool flash,float special=-1){
 desert_fallen_art::drawPose(rig,type,{1,0,0,0,state,std::max(0.f,attack),flash,special});
}

inline void buildDesertCorpse(Rig& rig,int type,int side=1){
 desert_fallen_art::drawPose(rig,type,{side<0?-1.f:1.f});
}

inline void buildDesertDeath(Rig& rig,const Enemy& e,float progress){
 const float contact=enemy_death::groundContact(e.type);
 const float time=std::clamp(progress/contact,0.f,1.f);
 const float facing=e.deathMotion.facingInitialized?e.deathMotion.facingAngle:int(e.deathMotion.direction)*PI/4;
 const float rest=int(e.animation.direction)*PI/4;
 rig.setFacing(facing+std::remainder(rest-facing,2*PI)*enemy_death::window(time,.05f,.90f));
 const float fold=enemy_death::window(time,.06f,.89f);
 const float recoil=.45f*enemy_death::window(time,0,.21f)*(1-enemy_death::window(time,.24f,.58f));
 desert_fallen_art::drawPose(rig,e.type,{float(e.deathSide()),time,fold,recoil,e.deathMotion,std::max(0.f,e.deathAttack),
                                       false,(e.type==CopperScarabType||e.type==SandCobraType)?e.deathSpecial:-1.f});
}
