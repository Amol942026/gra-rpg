#pragma once
template<class Check>void verifySprintArt(Check check){
 fs::create_directories("qa-v39");
 RenderTexture2D rt=LoadRenderTexture(280,270);
 for(Kind weapon:{Kind::Sword,Kind::Bow})for(int direction=0;direction<8;direction++){
  AnimationState state;state.direction=Direction(direction);state.moving=true;state.phase=.17f;
  auto render=[&](float run){state.runBlend=run;state.sprinting=run>0;BeginTextureMode(rt);ClearBackground(BLANK);
   actor_art::draw(0,140,225,1,state,0,true,true,weapon,false,BLANK,nullptr,-1,-1,-1,20);EndTextureMode();
   Image im=LoadImageFromTexture(rt.texture);ImageFlipVertical(&im);return im;};
  Image walk=render(0),sprint=render(1);Color* a=LoadImageColors(walk),*b=LoadImageColors(sprint);int difference=0,opaque=0,clipped=0,probes=0,mismatch=0;
  for(int y=0;y<270;y++)for(int x=0;x<280;x++){int n=y*280+x;difference+=std::abs(int(a[n].r)-b[n].r)+std::abs(int(a[n].g)-b[n].g)+std::abs(int(a[n].a)-b[n].a)>80;
   opaque+=b[n].a>240;clipped+=b[n].a>200&&(x<3||y<3||x>276||y>266);
  }
  for(int y=110;y<252;y+=13)for(int x=91;x<195;x+=13){
   bool solid=true,empty=true;for(int dy:{-2,0,2})for(int dx:{-2,0,2}){int alpha=b[(y+dy)*280+x+dx].a;solid&=alpha>240;empty&=alpha<8;}
   if(!solid&&!empty)continue;Vector2 point{x+.5f,y+.5f};
   bool hit=actor_art::draw(0,140,225,1,state,0,true,true,weapon,false,BLANK,&point,-1,-1,-1,20);
   mismatch+=solid?!hit:hit;probes++;
  }
  check(difference>300&&opaque>500&&clipped==0,"Shift sprint visibly changes the body, arms and legs in every facing and weapon pose");
  check(probes>30&&mismatch==0,"running sword/bow GPU silhouette and physical mouse geometry agree");
  UnloadImageColors(a);UnloadImageColors(b);UnloadImage(walk);UnloadImage(sprint);
 }
 UnloadRenderTexture(rt);
 RenderTexture2D sheet=LoadRenderTexture(1740,620);BeginTextureMode(sheet);ClearBackground(Ink);
 txt("BIEG / SHIFT — OSIEM KIERUNKÓW",25,18,25,Gold);
 for(int row=0;row<2;row++)for(int direction=0;direction<8;direction++){
  float x=direction*210.f+135,y=278+row*290;AnimationState pose;pose.direction=Direction(direction);pose.moving=pose.sprinting=true;pose.runBlend=1;pose.phase=.17f;
  actor_art::draw(0,x,y,2,pose,0,true,true,row?Kind::Bow:Kind::Sword,false,BLANK,nullptr,-1,-1,-1,20);
  center(row?"BIEG / ŁUK":"BIEG / MIECZ",x,y+18,14,Muted);
 }EndTextureMode();saveTexture(sheet,"qa-v39/sprint-8-directions.png");UnloadRenderTexture(sheet);
 sheet=LoadRenderTexture(1000,620);
 for(int frame=0;frame<24;frame++){
  BeginTextureMode(sheet);ClearBackground(Ink);txt("CHÓD",210,20,25,Gold);txt("BIEG / SHIFT",650,20,25,Gold);
  for(int row=0;row<2;row++)for(int run=0;run<2;run++){
   AnimationState pose;pose.direction=Direction::SE;pose.phase=frame/24.f;pose.moving=true;pose.sprinting=run;pose.runBlend=float(run);
   actor_art::draw(0,270+run*440.f,290+row*285.f,2.6f,pose,0,true,true,row?Kind::Bow:Kind::Sword,false,BLANK,nullptr,-1,-1,-1,20);
  }EndTextureMode();saveTexture(sheet,"qa-v39/sprint-frame-"+std::to_string(frame)+".png");
 }UnloadRenderTexture(sheet);
}
