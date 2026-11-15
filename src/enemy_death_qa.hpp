#pragma once

inline bool runEnemyDeathQA(){
 fs::create_directories("qa-deaths");
 std::ofstream report("qa-deaths/enemy-death-tests.txt");
 int checks=0,failures=0;
 auto check=[&](bool passes,int type,int direction,const char* detail){
  ++checks;if(!passes)++failures;
  report<<(passes?"PASS: ":"FAIL: ")<<enemyInfo(type).name<<" / "<<direction<<" / "<<detail<<'\n';
 };
 const double start=GetTime();
 for(int type=VolcanoFirstType;type<=VolcanoBossType;type++){
  const auto& atlas=volcano_actors::atlases[type-VolcanoFirstType];
  check(atlas.authoredRest&&IsShaderValid(volcano_corpse_material::shader),type,0,
        "the painted corpse materials are loaded for every directional cutout");
 }
 check(varkhor_death_art::ash.id&&varkhor_death_art::airborneAsh.id&&varkhor_death_art::phaseLocation>=0,
       VolcanoBossType,0,"Varkhor's ash atlases and granular dissolve shader are ready");
 check(enemy_death_fx::guardianSmokeTexture.id!=0,StoneGuardianType,0,"the guardian's stone smoke texture is loaded");
 constexpr int Width=320,Height=310;
 const Vector2 anchor{160,206};
 RenderTexture2D sample=LoadRenderTexture(Width,Height);
 // Verify the real shader independently of the different silhouettes of
 // two poses. Opaque pixels shared by both poses must never reveal terrain.
 BeginTextureMode(sample);ClearBackground(BLANK);native_world::beginOpaqueFrame();
 const bool mixed=corpse_pose_mix::draw(anchor,.5f,
     [](Vector2 p){DrawRectangleRec({p.x-15,p.y-14,20,12},{255,0,0,255});},
     [](Vector2 p){DrawRectangleRec({p.x-5,p.y-14,20,12},{0,0,255,255});});
 native_world::finishOpaqueBackground();EndTextureMode();
 Image mixProbe=LoadImageFromTexture(sample.texture);ImageFlipVertical(&mixProbe);
 const Color both=GetImageColor(mixProbe,int(anchor.x),int(anchor.y)-7);
 const Color incoming=GetImageColor(mixProbe,int(anchor.x)+10,int(anchor.y)-7);
 const Color outgoing=GetImageColor(mixProbe,int(anchor.x)-10,int(anchor.y)-7);
 UnloadImage(mixProbe);
 const auto half=[](unsigned char channel){return channel>=127&&channel<=128;};
 check(mixed&&both.a==255&&half(both.r)&&half(both.b),1,0,"overlapping opaque poses keep full coverage and linearly blend their colors");
 check(half(incoming.a)&&half(incoming.b)&&half(outgoing.a)&&half(outgoing.r),1,0,"a pixel unique to either pose fades with premultiplied coverage");
 using namespace actor_art::legacy_death;
 for(int flank:{1,-1}){
  const Motion rest{1,1,0,0,float(flank),0,Turn(SkeletonFallPitch),Turn(flank*SkeletonFallRoll)};
  const auto face=skeleton({0,8,70},rest),back=skeleton({0,-8,70},rest),hips=skeleton({0,0,34},rest);
  check(face.f<hips.f&&face.z>back.z,2,0,"the skeleton's head falls behind its hips with the face turned up");
  const Motion outlaw{1,1,0,0,float(flank),0,Turn(BanditFallPitch),Turn(flank*BanditFallRoll)};
  const auto hood=bandit({0,8,68},outlaw),hoodBack=bandit({0,-8,68},outlaw),belt=bandit({0,0,35},outlaw);
  check(hood.f<belt.f&&hood.z>hoodBack.z,6,0,"the bandit falls backward with the hood and chest turned upward");
 }
 Game arena;
 arena.developerSession=true;
 constexpr std::array<float,7> times{0,.15f,.34f,.53f,.73f,.92f,1};
 auto samplePose=[&](Enemy e,float progress){
  e.deathElapsed=progress>=1?-1:progress*enemy_death::duration(e.type);return e;
 };
 auto creature=[&](int type,int direction){
  Enemy e{900+type,type,{25,25},{25,25},100,100};
  e.animation.direction=Direction(direction);e.animation.facingInitialized=true;
  e.animation.facingAngle=direction*PI/4+.10f;e.animation.motionInitialized=true;
  e.animation.moving=true;e.animation.moveBlend=.84f;e.animation.phase=.28f;e.animation.idleTime=.4f;return e;
 };
 auto capturePixels=[&](auto draw){
  BeginTextureMode(sample);ClearBackground(BLANK);native_world::beginOpaqueFrame();
  draw();
  native_world::finishOpaqueBackground();EndTextureMode();
  Image image=LoadImageFromTexture(sample.texture);ImageFlipVertical(&image);
  const auto* data=static_cast<const std::uint32_t*>(image.data);
  std::vector<std::uint32_t> result(data,data+Width*Height);
  UnloadImage(image);return result;
 };
 auto snapshot=[&](const Enemy& e,bool live,bool centered=false,Color outline=BLANK){
  return capturePixels([&]{
   if(live)actor_art::drawEnemy(e,anchor.x,anchor.y);
   else if(centered)restingCorpseArt(e,anchor.x,anchor.y);
   else corpseArt(e,anchor.x,anchor.y,outline);
  });
 };
 const auto alpha=[](std::uint32_t pixel){return pixel>>24;};
 struct Change {float rgba=0,coverage=0;};
 auto imageChange=[&](const auto& before,const auto& after){
  std::uint64_t rgba=0,coverage=0,foreground=0;
  for(std::size_t i=0;i<before.size();i++){
   const auto a=before[i],b=after[i];
   foreground+=std::max(alpha(a),alpha(b));
   coverage+=std::abs(int(alpha(a))-int(alpha(b)));
   for(int shift:{0,8,16,24})rgba+=std::abs(int((a>>shift)&255)-int((b>>shift)&255));
  }
  return Change{float(rgba)/(4*std::max<std::uint64_t>(1,foreground)),float(coverage)/std::max<std::uint64_t>(1,foreground)};
 };
 auto blendPhase=[](int type,float weight){
  float low=0,high=enemy_death::groundContact(type);
  for(int step=0;step<22;step++){
   const float middle=(low+high)*.5f;
   if(enemy_death::restingBlend(type,middle)<weight)low=middle;else high=middle;
  }
  return (low+high)*.5f;
 };
 auto overlap=[&](const auto& a,const auto& b){
  int shared=0,total=0;
  for(std::size_t i=0;i<a.size();i++){shared+=alpha(a[i])>=80&&alpha(b[i])>=80;total+=alpha(a[i])>=80||alpha(b[i])>=80;}
  return float(shared)/std::max(1,total);
 };
 auto bounds=[&](const auto& pixels){
  int left=Width,right=0,top=Height,bottom=0;
  for(int y=0;y<Height;y++)for(int x=0;x<Width;x++)if(alpha(pixels[y*Width+x])>=128){
   left=std::min(left,x);right=std::max(right,x);top=std::min(top,y);bottom=std::max(bottom,y);
  }
  return std::array<int,4>{left,right,top,bottom};
 };
 struct BodyProjection {double area=0,thickness=0,span=0;};
 auto bodyProjection=[&](const auto& pixels,const std::array<int,4>& box){
  double area=0,mx=0,my=0,xx=0,xy=0,yy=0;
  for(int y=box[2];y<=box[3];y++)for(int x=box[0];x<=box[1];x++){
   const double a=alpha(pixels[y*Width+x])/255.0;
   area+=a;mx+=a*x;my+=a*y;xx+=a*x*x;xy+=a*x*y;yy+=a*y*y;
  }
  if(area<=0)return BodyProjection{};
  // Compare the characteristic lengths along the sprite's own axes so that
  // a fall through 90 degrees does not masquerade as a change of body size.
  const double varX=xx/area-(mx/area)*(mx/area),varY=yy/area-(my/area)*(my/area);
  const double cross=xy/area-(mx/area)*(my/area);
  const double spread=std::hypot(varX-varY,2*cross),trace=varX+varY;
  return BodyProjection{area,std::sqrt(std::max(0.0,(trace-spread)*.5)),std::sqrt(std::max(0.0,(trace+spread)*.5))};
 };
 const auto isCaldera=[](int type){return (type>=VolcanoFirstType&&type<=VolcanoBossType)||type==CinderhornType;};
 const auto detachedPieces=[&](auto unvisited){
  int islands=0;
  for(int y=1;y<Height-1;y++)for(int x=1;x<Width-1;x++){
   const int pixel=y*Width+x;
   if(alpha(unvisited[pixel])<128)continue;
   unvisited[pixel]=0;std::vector<int> island{pixel};
   for(std::size_t i=0;i<island.size();i++){
    for(int step:{1,-1,Width,-Width}){
     const auto p=island[i]+step;
     if(p<0||p>=Width*Height||alpha(unvisited[p])<128)continue;
     unvisited[p]=0;island.push_back(p);
    }
   }
   if(island.size()>=24)++islands;
  }
  return islands;
 };
 constexpr float PhaseProbe=.00025f;
 Change peakSwitch{},peakFrame{};
 std::array<int,EnemyTypeCount> standingHeight{},restingSpan{};
 int guardianStoneIslands=0;
 struct AdjacentProbe {int type=-1,side=0,frame=0;float rate=0;};
 AdjacentProbe peakFrameRgba{},peakFrameAlpha{};
 for(int type=0;type<EnemyTypeCount;type++)for(int direction=0;direction<8;direction++)for(int side:{1,-1}){
  const Enemy live=creature(type,direction);Enemy fallen=live;
  const V impulse=flankImpulse(live.animation,side);
  arena.hit(fallen,10000,DamageType::Physical,false,impulse);
  const auto first=snapshot(fallen,false),last=snapshot(samplePose(fallen,1),false);
  const Vector2 atRest=corpseDrawPoint(samplePose(fallen,1),anchor);
  const float contact=enemy_death::groundContact(type);
  const V screenImpulse=normalized({(impulse.x-impulse.y)*2,impulse.x+impulse.y});
  check(fallen.deathSide()==side&&(atRest.x-anchor.x)*screenImpulse.x+(atRest.y-anchor.y)*screenImpulse.y>6,
        type,direction,"the body drifts away from the lethal strike and keeps the assigned flank");
  if(side==1&&(fallen_art::flankIndex(type)>=0||type>=8)){
   Enemy opposite=samplePose(fallen,1);opposite.deathDirection=impulse*-1;
   check(snapshot(samplePose(fallen,1),false,true)!=snapshot(opposite,false,true),type,direction,
         "opposite impacts produce distinct resting art at the same anchor");
  }
  const auto living=snapshot(live,true);
  const auto liveBounds=bounds(living);
  const auto moving=samplePose(fallen,contact*.70f);
  const auto middle=snapshot(moving,false);
  check(overlap(living,first)>.89f,type,direction,"initial silhouette follows the previous living pose");
  check(overlap(first,middle)<.96f&&overlap(middle,last)<.99f,type,direction,"the in-flight pose changes before becoming the corpse");
  if(type==StoneGuardianType&&direction==int(Direction::SE)){
   constexpr Color forest{61,72,47,255};
   const auto smokeAt=[&](float elapsed,Color background){
    auto smoking=fallen;smoking.deathElapsed=elapsed;
    return capturePixels([&]{
     ClearBackground(background);
     enemy_death_fx::guardianSmoke(smoking,corpseDrawPoint(smoking,anchor));
    });
   };
   struct SmokeShape {int pixels=0,width=0,aboveGround=0;};
   const auto smokeShape=[&](float elapsed){
    SmokeShape shape;
    const auto pixels=smokeAt(elapsed,forest);
    int left=Width,right=-1;
    for(std::size_t p=0;p<pixels.size();p++){
     const auto value=pixels[p];
     if(std::max({int(value&255)-forest.r,int((value>>8)&255)-forest.g,int((value>>16)&255)-forest.b})<11)continue;
     left=std::min(left,int(p%Width));right=std::max(right,int(p%Width));
     ++shape.pixels;
     shape.aboveGround+=float(p/Width)<anchor.y-24;
    }
    shape.width=std::max(0,right-left+1);
    return shape;
   };
   const auto early=smokeShape(.18f),peak=smokeShape(.33f),late=smokeShape(.61f);
   const auto peakMask=smokeAt(.33f,BLANK);
   const auto endedMask=smokeAt(enemy_death::endTime(type),BLANK);
   std::uint32_t peakAlpha=0;
   for(const auto pixel:peakMask)peakAlpha=std::max(peakAlpha,alpha(pixel));
   report<<"Guardian smoke, flank "<<side<<": "<<early.pixels<<'/'<<peak.pixels<<'/'<<late.pixels
         <<" visible px, width "<<early.width<<'/'<<peak.width<<" px, alpha "<<peakAlpha<<"/255\n";
   check(early.pixels>200&&peak.pixels>650&&peak.aboveGround>350&&peak.width>early.width+12,
         type,direction,"stone smoke is visible during the collapse and expands over the fallen blocks");
   check(late.pixels>120&&late.pixels<peak.pixels&&peakAlpha>0&&peakAlpha<=165&&
         std::none_of(endedMask.begin(),endedMask.end(),[&](auto pixel){return alpha(pixel)!=0;}),
         type,direction,"the translucent smoke disperses after the stone blocks stop");
  }
  if(type==VolcanoBossType&&direction==int(Direction::SE)){
   const auto alphaMass=[&](const auto& pixels){
    std::uint64_t sum=0;
    for(const auto pixel:pixels)sum+=alpha(pixel);
    return double(sum)/255.0;
   };
   // Measure only the actual armor, so a dust puff cannot pass for the head.
   constexpr float phases[]{.19f,.32f,.48f,.74f};
   std::array<double,std::size(phases)> armor{};
   const auto liveMass=std::max(1.0,alphaMass(living));
   for(std::size_t i=0;i<armor.size();i++){
    const auto pose=varkhor_death_art::sample(phases[i]);
    const auto pixels=capturePixels([&]{
     varkhor_death_art::cutout(fallen,anchor.x,anchor.y,pose,BLANK,nullptr,false,1);
    });
    armor[i]=alphaMass(pixels)/liveMass;
   }
   report<<"Varkhor armor coverage at four phases, flank "<<side<<":";
   for(const auto amount:armor)report<<' '<<amount;
   report<<'\n';
   check(armor[0]>=.65&&armor[1]>=.18&&armor[1]<=.85&&armor[1]<armor[0]&&
         armor[2]>=.035&&armor[2]<armor[1]&&armor[3]<=.005,type,direction,
         "Varkhor's armor remains visible across successive breakup stages");
   constexpr float torsoPhases[]{0.f,.24f,.34f,.46f,.54f};
   std::array<double,std::size(torsoPhases)> torso{};
   for(std::size_t i=0;i<torso.size();i++){
    const auto pose=varkhor_death_art::sample(torsoPhases[i]);
    const auto pixels=capturePixels([&]{
     varkhor_death_art::cutout(fallen,anchor.x,anchor.y,pose,BLANK,nullptr,false,1,varkhor_death_art::TorsoParts);
    });
    torso[i]=alphaMass(pixels);
   }
   const double intactTorso=std::max(1.,torso[0]);
   report<<"Varkhor torso coverage, flank "<<side<<":";
   for(const double amount:torso)report<<' '<<amount/intactTorso;
   report<<'\n';
   check(torso[0]>50&&torso[1]>intactTorso*.55&&torso[1]<intactTorso*.995&&
         torso[2]>intactTorso*.08&&torso[2]<intactTorso*.65&&torso[3]<intactTorso*.12&&torso[4]<intactTorso*.01,
         type,direction,"Varkhor's torso crumbles during the early cloud instead of lingering to the end");

   constexpr int AshWidth=560;
   const Vector2 ashAnchor{AshWidth*.5f,anchor.y};
   auto ashSample=LoadRenderTexture(AshWidth,Height);
   const auto captureAsh=[&](float elapsed,Color background){
    if(!ashSample.id)return std::vector<std::uint32_t>{};
    Enemy ash=fallen;
    ash.deathElapsed=elapsed;
    BeginTextureMode(ashSample);ClearBackground(background);native_world::beginOpaqueFrame();
    enemy_death_fx::draw(ash,corpseDrawPoint(ash,ashAnchor));
    native_world::finishOpaqueBackground();EndTextureMode();
    Image image=LoadImageFromTexture(ashSample.texture);
    if(!image.data)return std::vector<std::uint32_t>{};
    ImageFlipVertical(&image);
    const auto* data=static_cast<const std::uint32_t*>(image.data);
    std::vector<std::uint32_t> result(data,data+AshWidth*Height);
    UnloadImage(image);return result;
   };
   // Measure the actual blend against lit basalt so a large, dim alpha mask
   // cannot pass while disappearing into the terrain.
   constexpr Color basalt{96,83,77,255};
   struct VisibleAsh {int pixels=0,span=0,height=0;double spread=0,y=0,verticalContinuity=0;};
   const auto ashShape=[&](const auto& pixels){
    VisibleAsh shape;
    std::array<int,AshWidth> columns{};
    std::array<int,Height> rows{};
    int mass=0;
    for(std::size_t p=0;p<pixels.size();p++){
     const auto pixel=pixels[p];
     const int contrast=std::max({std::abs(int(pixel&255)-basalt.r),
                                  std::abs(int((pixel>>8)&255)-basalt.g),
                                  std::abs(int((pixel>>16)&255)-basalt.b)});
     if(contrast<11)continue;
     columns[p%AshWidth]+=contrast;
     rows[p/AshWidth]+=contrast;
     shape.y+=(p/AshWidth+.5)*contrast;
     mass+=contrast;
     ++shape.pixels;
    }
    if(!mass)return shape;
    shape.y/=mass;
    // Ignore a few stray grains when measuring the visible width of the cloud.
    const auto percentile=[&](const auto& bins,double fraction){
     int total=0;
     for(int x=0;x<int(bins.size());x++){
      total+=bins[x];
      if(total>=fraction*mass)return x;
     }
     return int(bins.size())-1;
    };
    const int left=percentile(columns,.05),right=percentile(columns,.95);
    shape.span=right-left+1;
    shape.height=percentile(rows,.90)-percentile(rows,.10)+1;
    shape.spread=std::min(ashAnchor.x-left,right-ashAnchor.x);
    const int top=percentile(rows,.20),bottom=percentile(rows,.80);
    constexpr int RowWindow=5;
    if(bottom-top+1>=RowWindow){
     int total=0,lowest=mass;
     for(int row=top;row<=bottom;row++){
      total+=rows[row];
      if(row+RowWindow-1>bottom)continue;
      int window=0;
      for(int offset=0;offset<RowWindow;offset++)window+=rows[row+offset];
      lowest=std::min(lowest,window);
     }
     shape.verticalContinuity=double(lowest)*(bottom-top+1)/(double(total)*RowWindow);
    }
    return shape;
   };
   constexpr float ashTimes[]{.29f,.47f,.66f,.79f};
   std::array<VisibleAsh,std::size(ashTimes)> cloud{};
   std::uint32_t peakAlpha=0;
   const int bossWidth=liveBounds[1]-liveBounds[0]+1;
   for(std::size_t i=0;i<cloud.size();i++){
    cloud[i]=ashShape(captureAsh(ashTimes[i],basalt));
    for(auto pixel:captureAsh(ashTimes[i],BLANK))peakAlpha=std::max(peakAlpha,alpha(pixel));
    report<<"Varkhor ash over basalt, flank "<<side<<" at "<<ashTimes[i]<<" s: "
          <<cloud[i].pixels<<" visible px, span "<<cloud[i].span<<'/'<<bossWidth
          <<", height "<<cloud[i].height
          <<", center "<<ashAnchor.y-cloud[i].y<<" px above ground, smallest flank "<<cloud[i].spread
          <<" px, vertical continuity "<<cloud[i].verticalContinuity<<'\n';
   }
   check(cloud[0].pixels>=7100&&cloud[1].pixels>=5300&&cloud[2].pixels>=680,type,direction,
         "the airborne cloud has ample visible dust against basalt while the armor dissolves");
   check(cloud[1].span>=bossWidth*.72f&&cloud[1].spread>=bossWidth*.18f&&
         std::all_of(cloud.begin(),cloud.end(),[&](const auto& pose){return pose.span<=bossWidth*1.28f;}),
         type,direction,"the main cloud spreads on both sides to about the standing boss's width");
   check(cloud[0].height>=60&&cloud[1].height>=34&&cloud[3].height<=22,
         type,direction,"the body-sized cloud flattens as it falls onto the ground");
   check(cloud[0].verticalContinuity>=.30&&cloud[1].verticalContinuity>=.30,
         type,direction,"the main cloud has no horizontal gaps separating it into bands");
   const double firstDrop=cloud[1].y-cloud[0].y,secondDrop=cloud[2].y-cloud[1].y;
   check(cloud[0].y<=ashAnchor.y-24&&firstDrop>=5&&secondDrop>=5&&
         firstDrop<bossWidth*.30f&&secondDrop<bossWidth*.30f,type,direction,
         "the visible ash descends gradually toward the ground");
   check(cloud[3].pixels>=35&&std::abs(cloud[3].y-ashAnchor.y)<=20&&cloud[3].y>cloud[2].y,
         type,direction,"a fading layer remains visible close to the ground before dissipating");
   report<<"Varkhor ash peak alpha: "<<peakAlpha<<"/255\n";
   check(peakAlpha>0&&peakAlpha<=175,type,direction,"the airborne ash remains translucent");
   const auto settledDust=captureAsh(enemy_death::endTime(type),BLANK);
   check(!settledDust.empty()&&alphaMass(settledDust)==0,type,direction,
         "the airborne ash dissipates by the end of the settling interval");
   if(ashSample.id)UnloadRenderTexture(ashSample);
  }
  const auto restBounds=bounds(last);
  if(isCaldera(type)){
   const auto upright=bodyProjection(living,liveBounds),prone=bodyProjection(last,restBounds);
   const double area=prone.area/std::max(1.0,upright.area);
   const double thickness=prone.thickness/std::max(1.0,upright.thickness);
   const double span=prone.span/std::max(1.0,upright.span);
   report<<"Caldera silhouette "<<enemyInfo(type).name<<" / "<<direction<<'/'<<side
         <<": area "<<area<<", thickness "<<thickness<<", length "<<span<<'\n';
   const int height=restBounds[3]-restBounds[2]+1;
   const int width=restBounds[1]-restBounds[0]+1;
   const int liveHeight=liveBounds[3]-liveBounds[2]+1;
   if(type==AshWraithType)
    check(height<=liveHeight*.62f&&width>=40&&area>=.14&&area<=.90,type,direction,
          "the empty robe deflates into a broad, low pile of cloth");
   else if(type==ObsidianSentinelType)
    check(detachedPieces(last)>=5&&width>=55&&area>=.20&&area<=1.20,type,direction,
          "the shattered armor leaves recognizable, visibly separate fragments");
   else if(type==VolcanoBossType)
    check(height<=liveHeight*.36f&&width>=109&&area>=.061&&area<=.52&&span>=.80,type,direction,
          "the dust leaves a broad, low ash deposit close to the standing boss's width");
   else check(area>=.61&&area<=1.43&&thickness>=.56&&thickness<=1.64&&span>=.67&&span<=1.48,type,direction,
              "the prone silhouette keeps the living model's bulk and proportions");
  }
  const auto [left,right,top,bottom]=restBounds;
  standingHeight[type]=std::max(standingHeight[type],liveBounds[3]-liveBounds[2]+1);
  restingSpan[type]=std::max({restingSpan[type],right-left+1,bottom-top+1});
  if(type==3||type==7||type>=8)
   check(left>1&&top>1&&right<Width-2&&bottom<Height-2,
         type,direction,"the complete large corpse fits inside the captured frame");
  if(type==CinderhornType){
   const auto support=regional_enemy_art::cinderhornFootprint(fallen);
   const int startX=int(std::ceil(atRest.x+support.x)),endX=int(std::floor(atRest.x+support.x+support.width));
   const int ground=int(std::floor(atRest.y));
   int contactWidth=0;
   for(int x=startX;x<=endX;x++){
    bool covered=false;
    for(int y=ground-11;y<=ground+3;y++)covered|=alpha(last[y*Width+x])>=128;
    contactWidth+=covered;
   }
   report<<"Cinderhorn ground coverage "<<direction<<'/'<<side<<": "<<contactWidth<<'/'<<(endX-startX+1)<<'\n';
   check(endX-startX>=42&&contactWidth>=.65f*(endX-startX+1)&&contactWidth>=35,type,direction,
         "a wide strip of the fallen cinderhorn's flank reaches the floor");
  }
  if(type==3&&side==1)guardianStoneIslands=std::max(guardianStoneIslands,detachedPieces(last));
  const auto approaching=snapshot(samplePose(fallen,contact-.04f),false);
  const auto approachBounds=bounds(approaching);
  const float lateOverlap=overlap(approaching,last);
  const bool fitted=lateOverlap>=.83f&&std::equal(restBounds.begin(),restBounds.end(),approachBounds.begin(),
      [](int rest,int active){return std::abs(rest-active)<=4;});
  if(!fitted){
   report<<"Late fit "<<type<<'/'<<direction<<" overlap "<<lateOverlap<<" edge shifts";
   for(int i=0;i<4;i++)report<<' '<<approachBounds[i]-restBounds[i];
   report<<'\n';
  }
  check(fitted,type,direction,"the last part of the movement is fitted to the corpse size and ground anchor");
  {
   Enemy grounded=fallen;
   grounded.deathElapsed=enemy_death::groundContactTime(type);
   const auto contactPixels=snapshot(grounded,false);
   const auto contactOutline=snapshot(grounded,false,false,Gold);
   check(contactPixels==last&&contactOutline==snapshot(samplePose(fallen,1),false,false,Gold),
         type,direction,"ground contact already shows the final RGBA silhouette with or without hover");
   arena.enemies.assign(1,grounded);
   bool fixed=true;
   while(arena.enemies.front().dying()){
    arena.advanceEnemyDeaths(1.f/60);
    const auto& pose=arena.enemies.front();
    const auto point=corpseDrawPoint(pose,anchor);
    const auto pixels=snapshot(pose,false);
    fixed=fixed&&point.x==atRest.x&&point.y==atRest.y&&pixels==contactPixels;
   }
   check(fixed,type,direction,"every grounded 60 Hz sample preserves the contact pixels and anchor");
  }
  for(float rate:{30.f,60.f,120.f}){
   const float finalPose=1-1/(rate*enemy_death::duration(type));
   const auto predecessor=snapshot(samplePose(fallen,finalPose),false);
   check(predecessor==last,type,direction,rate==30.f?"last 30 Hz pose equals the resting corpse pixel for pixel":
         rate==60.f?"last 60 Hz pose equals the resting corpse pixel for pixel":"last 120 Hz pose equals the resting corpse pixel for pixel");
  }
  // The static endpoint alone misses a one-frame outline or color change in
  // the overlap. Probe both old outline switch weights and contact.
  const float oldMeshOutlineSwitch=(3-std::sqrt(5.f))*.5f;
  for(float phase:{blendPhase(type,.00000001f),blendPhase(type,oldMeshOutlineSwitch),blendPhase(type,.5f),contact}){
   const auto before=samplePose(fallen,phase-PhaseProbe),after=samplePose(fallen,phase+PhaseProbe);
   const auto plain=imageChange(snapshot(before,false),snapshot(after,false));
   const auto outlined=imageChange(snapshot(before,false,false,Gold),snapshot(after,false,false,Gold));
   const Change jump{std::max(plain.rgba,outlined.rgba),std::max(plain.coverage,outlined.coverage)};
   peakSwitch.rgba=std::max(peakSwitch.rgba,jump.rgba);peakSwitch.coverage=std::max(peakSwitch.coverage,jump.coverage);
   const bool continuous=jump.rgba<=.035f&&jump.coverage<=.065f;
   if(!continuous)report<<"Body discontinuity at phase "<<phase<<" flank "<<side
      <<": normal RGBA "<<plain.rgba<<" alpha "<<plain.coverage<<" outline RGBA "<<outlined.rgba<<" alpha "<<outlined.coverage<<'\n';
   check(continuous,type,direction,"body and highlight remain continuous across the handoff");
  }
  if(direction==int(Direction::SE)||(isCaldera(type)&&direction==int(Direction::N)))for(float rate:{30.f,60.f}){
   // Measure adjacent display timestamps through first ground contact.
   const float frames=rate*enemy_death::duration(type);
   const int firstFrame=isCaldera(type)?0:int(std::floor(blendPhase(type,.5f)*frames));
   const int lastFrame=int(std::ceil(contact*frames));
   auto before=snapshot(samplePose(fallen,firstFrame/frames),false,false,Gold);
   Change peakForFall{};
   int largestShiftFrame=firstFrame;
   for(int frame=firstFrame+1;frame<=lastFrame;frame++){
    auto after=snapshot(samplePose(fallen,frame/frames),false,false,Gold);
    const auto jump=imageChange(before,after);
    if(jump.rgba>peakFrame.rgba)peakFrameRgba={type,side,frame,rate};
    if(jump.coverage>peakFrame.coverage)peakFrameAlpha={type,side,frame,rate};
    peakFrame.rgba=std::max(peakFrame.rgba,jump.rgba);peakFrame.coverage=std::max(peakFrame.coverage,jump.coverage);
    peakForFall.rgba=std::max(peakForFall.rgba,jump.rgba);
    if(jump.coverage>peakForFall.coverage){
     peakForFall.coverage=jump.coverage;
     largestShiftFrame=frame;
    }
    before=std::move(after);
   }
   if(isCaldera(type)){
    report<<"Caldera motion "<<enemyInfo(type).name<<" / "<<direction<<'/'<<side<<'/'<<rate
          <<": peak RGBA "<<peakForFall.rgba<<", alpha "<<peakForFall.coverage
          <<" at "<<(largestShiftFrame-1)/rate<<"–"<<largestShiftFrame/rate<<" s\n";
    const bool steady=rate==60?(peakForFall.rgba<=.42f&&peakForFall.coverage<=.60f):
                                (peakForFall.rgba<=.57f&&peakForFall.coverage<=.74f);
    check(steady,type,direction,"adjacent pictures of the complete Caldera fall stay continuous at the sampled display rate");
   }
  }
  int solid=0,picked=0;
  for(int y=2;y<Height-2;y+=3)for(int x=2;x<Width-2;x+=3){
   const int i=y*Width+x;
   if(alpha(middle[i])<195||alpha(middle[i-1])<170||alpha(middle[i+1])<170||alpha(middle[i-Width])<170||alpha(middle[i+Width])<170)continue;
   const Vector2 point{x+.5f,y+.5f};solid++;
   picked+=corpseArt(moving,anchor.x,anchor.y,BLANK,&point);
   if(solid>=7)break;
  }
  if(type==VolcanoBossType)report<<"Varkhor fragment/ash picks, flank "<<side<<": "<<picked<<'/'<<solid<<'\n';
  check(solid>=3&&picked>=solid*.73f,type,direction,"the rendered body remains pickable during its fall");
  if(volcano_actors::specialDeathArt(type)){
   int visibleSamples=0,clickableSamples=0;
   const Enemy settled=samplePose(fallen,1);
   for(int y=top+1;y<bottom;y+=3)for(int x=left+1;x<right;x+=3)if(alpha(last[y*Width+x])>=220){
    ++visibleSamples;
    const Vector2 point{x+.5f,y+.5f};
    clickableSamples+=corpseArt(settled,anchor.x,anchor.y,BLANK,&point);
   }
   check(visibleSamples>=8&&clickableSamples>=visibleSamples*.78f,type,direction,
         "opaque cloth, fragments or settled ash remain clickable after touchdown");
  }
 }
 check(guardianStoneIslands>=6,3,0,"the finished guardian shows at least six clearly detached blocks");
 report<<"Detached guardian blocks in the clearest direction: "<<guardianStoneIslands<<'\n';
 for(int type=0;type<EnemyTypeCount;type++)if((type==3||type>=7)&&!volcano_actors::specialDeathArt(type)){
  const int standing=standingHeight[type],span=restingSpan[type];
  report<<"Scale "<<enemyInfo(type).name<<": standing "<<standing<<" px; corpse span "<<span<<" px\n";
  const float minimum=type==3||type==7?.88f:.55f;
  check(standing>20&&span>=standing*minimum,type,0,"the prone body keeps the scale of the standing creature");
 }
 report<<"Peak near-instant body change, normalized to visible pixels: RGBA "<<peakSwitch.rgba
       <<", alpha "<<peakSwitch.coverage<<'\n';
 report<<"Peak adjacent 30/60 Hz body change including motion: RGBA "<<peakFrame.rgba
       <<", alpha "<<peakFrame.coverage<<'\n';
 auto reportPeak=[&](const char* channel,const AdjacentProbe& peak){
  if(peak.type<0)return;
  report<<channel<<" peak: "<<enemyInfo(peak.type).name<<", flank "<<peak.side<<", "<<peak.rate
        <<" Hz, elapsed "<<(peak.frame-1)/peak.rate<<" to "<<peak.frame/peak.rate<<" s\n";
 };
 reportPeak("RGBA",peakFrameRgba);reportPeak("Alpha",peakFrameAlpha);

 // Exercise the actual world shadow pass around the point where the dying
 // mesh gives way to the baked ground shadow. The fixture positions the
 // character well away from the hero so that the crop contains its shadow.
 auto previousGame=std::move(game);
 game=Game{};game.player={20,20};game.developerSession=true;
 game.props.clear();game.forage.clear();game.ground.clear();game.enemies.resize(1);
 auto shadowSnapshot=[&](const Enemy& e,bool* bearsWeight=nullptr){
  game.enemies.front()=e;
  world_shadows::prepare();
  const auto p=screenFlat(e.pos);
  const auto offset=world_shadows::maskOffset;
  const float scale=world_shadows::maskScale;
  Image image=LoadImageFromTexture(world_shadows::mask.texture);ImageFlipVertical(&image);
  const int left=std::clamp(int((p.x+offset.x-121)*scale),0,image.width),right=std::clamp(int((p.x+offset.x+121)*scale),0,image.width);
  const int top=std::clamp(int((p.y+offset.y-50)*scale),0,image.height),bottom=std::clamp(int((p.y+offset.y+64)*scale),0,image.height);
  std::vector<unsigned char> rgba;
  rgba.reserve(std::size_t(right-left)*(bottom-top));
  const auto* colors=static_cast<const Color*>(image.data);
  if(bearsWeight){
   const auto anchor=corpseDrawPoint(e,p);
   const auto foot=regionalEnemy(e.type)?world_shadows::paintedGroundFootprint(e):volcano_actors::groundFootprint(e);
   *bearsWeight=foot.width>0;
   for(float across:{.25f,.5f,.75f}){
    const int x=int((anchor.x+offset.x+foot.x+foot.width*across)*scale);
    const int y=int((anchor.y+offset.y+foot.y+foot.height*.5f)*scale);
    *bearsWeight&=x>=0&&x<image.width&&y>=0&&y<image.height&&colors[y*image.width+x].a>=210;
   }
  }
  for(int y=top;y<bottom;y++)for(int x=left;x<right;x++)rgba.push_back(colors[y*image.width+x].a);
  UnloadImage(image);actor_art::finishShadowFrame();
  return rgba;
 };
 float peakShadow=0;
 for(int type=0;type<EnemyTypeCount;type++)for(int side:{1,-1})for(int facing=0;facing<8;facing++){
  const bool contactShadow=isCaldera(type)||type==SporekinType;
  if(type!=CinderhornType&&type!=SporekinType&&facing!=int(Direction::SE))continue;
  auto fallen=creature(type,facing);fallen.pos={23,18};
  arena.hit(fallen,10000,DamageType::Physical,false,flankImpulse(fallen.animation,side));
  const float contact=enemy_death::groundContact(type);
  const auto before=shadowSnapshot(samplePose(fallen,contact-PhaseProbe));
  fallen.deathElapsed=enemy_death::groundContactTime(type);
  bool bearsWeight=false;
  const auto after=shadowSnapshot(fallen,contactShadow?&bearsWeight:nullptr);
  std::uint64_t total=0,difference=0;
  for(std::size_t i=0;i<before.size();i++){
   difference+=std::abs(int(before[i])-int(after[i]));total+=std::max(before[i],after[i]);
  }
  const float jump=float(difference)/std::max<std::uint64_t>(1,total);
  peakShadow=std::max(peakShadow,jump);
  if(total==0||jump>.06f)report<<"Ground shadow discontinuity flank "<<side<<": alpha "<<jump<<" mass "<<total<<'\n';
  check(total>0&&jump<=.06f,type,facing,"the ground shadow does not jump at the resting pose");
  if(contactShadow)check(bearsWeight,type,facing,"the contact shadow actually lies underneath the fallen body");
  {
   fallen.deathElapsed=-1;
   check(total>0&&after==shadowSnapshot(fallen),type,facing,"the real ground shadow stays pixel-identical after contact");
  }
 }
 const bool oldStats=statsOpen,oldSkills=skillsOpen,oldWelcome=welcome,oldPaused=paused;
 const float oldScale=renderScale;
 const auto oldMouse=mouse;
 const auto oldIntro=intro;const auto oldStory=towerStory;
 statsOpen=skillsOpen=welcome=paused=false;intro={};towerStory={};mouse={0,0};
 prepareCanvas(1);
 // Capture the gameplay compositor with forest lighting, terrain and shadows.
 // Keep the camera and ambient clock still. The explicit dust mask excludes
 // only particles that continue moving after the visible body has landed.
 const int facing=int(Direction::SE);
 V fallSite{23,18};
 Vector2 screenSite=screen(fallSite);
 Rectangle crop{screenSite.x-Width*.5f,screenSite.y-anchor.y,Width,Height};
 const auto colors=[](const Image& image){
  const auto* data=static_cast<const std::uint32_t*>(image.data);
  return std::vector<std::uint32_t>(data,data+image.width*image.height);
 };
 auto worldCrop=[&](){
  renderScene();
  Image world=LoadImageFromTexture(canvas.texture);ImageFlipVertical(&world);
  Image result=ImageFromImage(world,crop);
  UnloadImage(world);return result;
 };
 auto dustMask=[&](const Enemy& e){
  BeginTextureMode(sample);ClearBackground(BLANK);native_world::beginOpaqueFrame();
  BeginMode2D({{-crop.x,-crop.y},{0,0},0,1});
  if(e.dying())enemy_death_fx::draw(e,corpseDrawPoint(e,screenSite));
  EndMode2D();native_world::finishOpaqueBackground();EndTextureMode();
  Image art=LoadImageFromTexture(sample.texture);ImageFlipVertical(&art);
  auto pixels=colors(art);UnloadImage(art);return pixels;
 };
 game.enemies.clear();
 Image emptyWorld=worldCrop();
 const auto backgroundPixels=colors(emptyWorld);UnloadImage(emptyWorld);
 const char* captions[]={"Chwila przed", "Kontakt", "Po 1 / 60 s", "Spoczynek"};
 constexpr int Header=35,SideLabel=145;
 Image gallery=GenImageColor(SideLabel+Width*4,Header+Height*4,rgb(19,30,32));
 for(int col=0;col<4;col++)ImageDrawText(&gallery,captions[col],SideLabel+Width*col+9,9,18,Gold);
 int galleryRow=0;
 for(int type:{1,BoarType})for(int side:{1,-1}){
  auto live=creature(type,facing);live.pos=fallSite;
  game.enemies.assign(1,live);
  auto& falling=game.enemies.front();
  game.hit(falling,10000,DamageType::Physical,false,flankImpulse(falling.animation,side));
  game.effects.clear();game.combatImpacts.clear();
  auto advanceTo=[&](float tick){
   while(falling.deathElapsed+.05f<tick)game.advanceEnemyDeaths(.05f);
   game.advanceEnemyDeaths(tick-falling.deathElapsed);
  };
  const float contact=enemy_death::groundContactTime(type);
  const int screenY=Header+galleryRow++*Height;
  ImageDrawText(&gallery,type==1?"Wilk":"Dzik",10,screenY+Height/2-22,21,Text);
  ImageDrawText(&gallery,side==1?"Prawy bok":"Lewy bok",10,screenY+Height/2+7,17,Muted);
  auto capture=[&](int col){
   Image frame=worldCrop();
   ImageDraw(&gallery,frame,{0,0,Width,Height},{float(SideLabel+col*Width),float(screenY),Width,Height},WHITE);
   auto pixels=colors(frame);UnloadImage(frame);return pixels;
  };
  advanceTo(contact-.05f);
  const auto before=capture(0);
  advanceTo(contact);
  check(falling.deathElapsed==contact&&falling.deathSide()==side,type,facing,"the gameplay clock reaches exact ground contact on the assigned flank");
  const auto atContact=capture(1),dustAtContact=dustMask(falling);
  game.advanceEnemyDeaths(1.f/60);
  const auto afterOne=capture(2),dustAfterOne=dustMask(falling);
  while(falling.dying())game.advanceEnemyDeaths(.05f);
  const auto settled=capture(3);
  int visible=0,changedBefore=0,changedGrounded=0,dustPixels=0;
  for(std::size_t i=0;i<atContact.size();i++){
   if(alpha(dustAtContact[i])||alpha(dustAfterOne[i])){++dustPixels;continue;}
   visible+=atContact[i]!=backgroundPixels[i];
   changedBefore+=atContact[i]!=before[i];
   changedGrounded+=atContact[i]!=afterOne[i]||atContact[i]!=settled[i];
  }
  report<<"Gameplay crop "<<type<<'/'<<side<<": visible "<<visible<<", in-flight change "<<changedBefore
        <<", grounded change "<<changedGrounded<<", dust excluded "<<dustPixels<<'\n';
  check(visible>100&&changedBefore>40&&dustPixels>0,type,facing,"the world capture resolves the falling body and isolates the moving dust");
  check(changedGrounded==0,type,facing,"the gameplay compositor retains the same visible body and shadow after touchdown");
 }
 check(ExportImage(gallery,"qa-deaths/grounded-wolf-and-boar.png"),1,facing,"export the ground-contact frames from the game renderer");
 UnloadImage(gallery);

 const char* regionCaptions[]={"Przed ciosem","Upadek","Kontakt","Spoczynek"};
 const char* calderaCaptions[]={"Przed ciosem","Ugięcie","Przechył","Opadanie","Przed ziemią","Kontakt","Spoczynek"};
 constexpr std::array<float,4> calderaFallTimes{.19f,.42f,.68f,.91f};
 constexpr int RegionLabel=215;
 std::vector<int> glyphs;
 for(int code=32;code<384;code++)glyphs.push_back(code);
 const Font labelsFont=LoadFontEx("assets/Lato-Bold.ttf",17,glyphs.data(),int(glyphs.size()));
 auto regionalGallery=[&](const std::vector<int>& types,const char* path,const std::vector<int>& flanks={1},bool timeline=false){
  const auto captions=timeline?calderaCaptions:regionCaptions;
  const int columns=timeline?int(std::size(calderaCaptions)):int(std::size(regionCaptions));
  Image regional=GenImageColor(RegionLabel+Width*columns,Header+Height*types.size()*flanks.size(),rgb(19,30,32));
  for(int col=0;col<columns;col++)
   ImageDrawTextEx(&regional,labelsFont,captions[col],{float(RegionLabel+Width*col+9),9},17,1,Gold);
  game.enemies.clear();
  Image emptyField=worldCrop();const auto empty=colors(emptyField);UnloadImage(emptyField);
  int y=Header;
  for(int type:types)for(int flank:flanks){
   game.allocationRequired=false;game.messageTime=0;
   auto live=creature(type,facing);live.pos=fallSite;
   game.enemies.assign(1,live);
   auto& falling=game.enemies.front();
   const bool showFlank=timeline||flanks.size()>1;
   ImageDrawTextEx(&regional,labelsFont,enemyInfo(type).name,{10,float(y+Height/2-(showFlank?21:9))},17,1,Text);
   if(showFlank)ImageDrawTextEx(&regional,labelsFont,flank>0?"Prawy bok":"Lewy bok",{10,float(y+Height/2+4)},17,1,Muted);
   auto capture=[&](int col){
    Image frame=worldCrop();
    if(col>=0)ImageDraw(&regional,frame,{0,0,Width,Height},{float(RegionLabel+col*Width),float(y),Width,Height},WHITE);
    auto pixels=colors(frame);UnloadImage(frame);return pixels;
   };
   capture(0);
   game.hit(falling,10000,DamageType::Physical,false,flankImpulse(falling.animation,flank));
   game.effects.clear();game.combatImpacts.clear();
   const float contact=enemy_death::groundContactTime(type);
   auto advanceTo=[&](float at){
    while(falling.deathElapsed+.05f<at)game.advanceEnemyDeaths(.05f);
    game.advanceEnemyDeaths(at-falling.deathElapsed);
   };
   if(timeline){
    for(std::size_t i=0;i<calderaFallTimes.size();i++){
     advanceTo(contact*calderaFallTimes[i]);capture(int(i)+1);
    }
   }else{
    advanceTo(contact*(type==3?.89f:.68f));capture(1);
   }
   advanceTo(contact);
   const auto impact=capture(columns-2),groundDust=dustMask(falling);
   game.advanceEnemyDeaths(1.f/60);
   const auto after=capture(-1),laterDust=dustMask(falling);
   while(falling.dying())game.advanceEnemyDeaths(.05f);
   const auto final=capture(columns-1);
   int stableChanges=0,visible=0;
   for(std::size_t pixel=0;pixel<impact.size();pixel++){
    if(alpha(groundDust[pixel])||alpha(laterDust[pixel]))continue;
    stableChanges+=impact[pixel]!=after[pixel]||impact[pixel]!=final[pixel];
    visible+=impact[pixel]!=empty[pixel];
   }
   report<<"Region contact "<<enemyInfo(type).name<<": "<<stableChanges<<" moved pixels outside dust; "<<visible<<" visible pixels (flank "<<flank<<")\n";
   check(stableChanges==0&&visible>100,type,facing,"the actual rendered body and shadow stay put from the contact frame");
   y+=Height;
  }
  check(ExportImage(regional,path),types.front(),facing,"export the region's ordinary gameplay rendering");
  UnloadImage(regional);
 };
 regionalGallery({2,6,7,SporekinType,3},"qa-deaths/forest-fall-details.png",{1},true);

 const auto previousChests=game.chests;
 game.enemies.clear();
 game.props={{{19,24},0,1.04f},{{25,17},4,1.f},{{26,22},5,1.f},
             {{20.7f,22.7f},ForestDecorationStart,1.f},{{21,17},ForestDecorationStart+3,1.07f}};
 game.chests.front().pos={23,22};
 game.messageTime=0;
 renderScene();saveTexture(canvas,"qa-deaths/forest-wood-in-game.png");
 game.props.clear();game.chests=previousChests;
 auto setField=[&](WorldLocation field,V player,V site){
  game=Game{};
  game.activateField(field);
  game.player=player;game.developerSession=true;
  game.props.clear();game.enemies.clear();game.ground.clear();game.forage.clear();
  fallSite=site;screenSite=screen(fallSite);
  crop={screenSite.x-Width*.5f,screenSite.y-anchor.y,Width,Height};
 };
 setField(WorldLocation::Desert,{24,23},{26,21});
 regionalGallery({8,9,10,DesertBossType,DuneEyeType},"qa-deaths/desert-ground-contact.png");
 setField(WorldLocation::Volcano,VolcanoArrival,VolcanoArrival+V{2,-1});
 const std::vector<int> calderaTypes{12,13,14,15,16,17,VolcanoBossType,CinderhornType};
 regionalGallery(calderaTypes,"qa-deaths/caldera-ground-contact.png",{1},true);
 regionalGallery(calderaTypes,"qa-deaths/caldera-ground-contact-left.png",{-1},true);
 regionalGallery({CinderhornType},"qa-deaths/cinderhorn-both-flanks.png",{1,-1});
 setField(WorldLocation::Forest,{20,20},{23,18});
 UnloadFont(labelsFont);
 game.enemies.resize(1);
 game.enemies.front()=creature(1,int(Direction::SE));
 game.enemies.front().pos={22,19};arena.hit(game.enemies.front(),10000);
 game.allocationRequired=true;statsOpen=true;skillsOpen=false;welcome=false;intro={};towerStory={};
 invalidatePausedWorld();renderScene();
 const unsigned firstBuild=pausedWorldCache.builds;
 game.advanceEnemyDeaths(.05f);renderScene();
 check(pausedWorldCache.builds==firstBuild+1,1,int(Direction::SE),"mandatory level allocation refreshes the fall even when gameplay is paused");
 while(game.enemies.front().dying())game.advanceEnemyDeaths(.05f);
 renderScene();const unsigned bodyBuild=pausedWorldCache.builds;renderScene();
 check(bodyBuild==firstBuild+2&&pausedWorldCache.builds==bodyBuild,1,int(Direction::SE),"the paused scene captures the settled pose once and resumes caching");
 invalidatePausedWorld();statsOpen=oldStats;skillsOpen=oldSkills;welcome=oldWelcome;paused=oldPaused;
 intro=oldIntro;towerStory=oldStory;mouse=oldMouse;prepareCanvas(oldScale);
 game=std::move(previousGame);
 report<<"Peak near-instant world ground shadow change: alpha "<<peakShadow<<'\n';
 auto drawTile=[&](RenderTexture2D sheet,const Enemy& pose,float x,float y,float scale,float shadowWidth,float shadowHeight){
  BeginTextureMode(sample);ClearBackground(BLANK);native_world::beginOpaqueFrame();
  corpseArt(pose,anchor.x,anchor.y);
  if(pose.dying())enemy_death_fx::draw(pose,corpseDrawPoint(pose,anchor));
  native_world::finishOpaqueBackground();EndTextureMode();

  BeginTextureMode(sheet);
  ellipse(x,y+scale,shadowWidth*scale,shadowHeight*scale,rgb(26,42,44));
  // Keep atlas and shader passes on their ordinary game-space transform.
  // Scale only their completed, premultiplied render for the preview grid.
  BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
  DrawTexturePro(sample.texture,{0,0,Width,-Height},{x-anchor.x*scale,y-anchor.y*scale,Width*scale,Height*scale},{},0,WHITE);
  EndBlendMode();EndTextureMode();
 };
 constexpr int CellW=161,CellH=164,LabelW=224;
 const std::array<std::vector<int>,3> realms{{{0,1,2,3,4,5,6,7,19},{8,9,10,11,20},{12,13,14,15,16,17,18,21}}};
 const char* files[]={"forest","desert","caldera"};
 for(std::size_t realm=0;realm<realms.size();realm++){
  RenderTexture2D sheet=LoadRenderTexture(LabelW+CellW*times.size(),28+CellH*realms[realm].size());
  BeginTextureMode(sheet);ClearBackground(rgb(19,30,32));
  for(std::size_t col=0;col<times.size();col++)txt(TextFormat("%ld %%",std::lround(times[col]*100)),LabelW+CellW*col+24,5,15,Gold);
  EndTextureMode();
  int row=0;
  for(int type:realms[realm]){
   const float y=28+CellH*row++;
   BeginTextureMode(sheet);
   line(9,y,sheet.texture.width-9,y,rgb(43,65,66));
   txt(enemyInfo(type).name,12,y+64,16,Text);
   EndTextureMode();
   auto e=creature(type,1);arena.hit(e,10000,DamageType::Physical,false,flankImpulse(e.animation,1));
   for(std::size_t col=0;col<times.size();col++){
    const auto pose=samplePose(e,times[col]);
    const float x=LabelW+CellW*col+CellW*.5f;
    const float scale=enemyBoss(type)?.68f:.83f;
    drawTile(sheet,pose,x,y+139,scale,35,12);
   }
  }
  saveTexture(sheet,TextFormat("qa-deaths/deaths-%s.png",files[realm]));UnloadRenderTexture(sheet);
 }
 RenderTexture2D closeup=LoadRenderTexture(1360,642);
 BeginTextureMode(closeup);ClearBackground(rgb(19,30,32));
 for(std::size_t col=0;col<times.size();col++)txt(TextFormat("%ld %%",std::lround(times[col]*100)),174+168*col,19,15,Gold);
 EndTextureMode();
 int row=0;
 for(int type:{1,BoarType})for(int side:{1,-1}){
  const float y=146+154*row++;
  BeginTextureMode(closeup);
  line(10,y-103,1350,y-103,rgb(43,65,66));
  txt(enemyInfo(type).name,19,y-65,21,Text);
  txt(side==1?"Prawy bok":"Lewy bok",19,y-35,15,Muted);
  EndTextureMode();
  auto e=creature(type,int(Direction::SE));arena.hit(e,10000,DamageType::Physical,false,flankImpulse(e.animation,side));
  for(std::size_t col=0;col<times.size();col++){
   const float x=199+168*col;
   const auto pose=samplePose(e,times[col]);
   drawTile(closeup,pose,x,y,1.42f,39,11);
  }
 }
 saveTexture(closeup,"qa-deaths/deaths-wolf-and-boar.png");UnloadRenderTexture(closeup);UnloadRenderTexture(sample);
 report<<"Render, readback and export seconds: "<<GetTime()-start<<'\n';
 report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';
 return failures==0;
}
