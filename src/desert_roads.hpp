#pragma once
// Baked once into terrain. The shared spline also reserves obstacle clearance.
void roadTriangle(Vector2 a,Vector2 b,Vector2 c,Color ca,Color cb,Color cc){
 if((b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x)>0){std::swap(b,c);std::swap(cb,cc);}
 rlCheckRenderBatchLimit(3);rlBegin(RL_TRIANGLES);
 for(auto v:{std::pair<Vector2,Color>{a,ca},{b,cb},{c,cc}}){rlColor4ub(v.second.r,v.second.g,v.second.b,v.second.a);rlTexCoord2f(0,0);rlVertex2f(v.first.x,v.first.y);}rlEnd();
}
void drawRoads(){
 auto project=[](V p){return Vector2{TerrainOriginX+(p.x-p.y)*40,90+(p.x+p.y)*20};};
 beginTransparentLayer();
 for(const auto& road:desertRoads()){
  const auto& points=road.points;std::vector<V> normals(points.size());std::vector<float> along(points.size());
  for(size_t i=0;i<points.size();i++){
   V tangent=normalized(points[std::min(i+2,points.size()-1)]-points[i>1?i-2:0]);normals[i]={-tangent.y,tangent.x};
   if(i)along[i]=along[i-1]+distance(points[i-1],points[i]);
  }
  auto width=[&](size_t i){return road.width*(1+.085f*std::sin(along[i]*.75f)+.045f*std::sin(along[i]*2.1f));};
  const float lateral[]={-1.65f,-1.3f,-.85f,0,.85f,1.3f,1.65f};
  const float opacity[]={0,24,68,100,68,24,0};
  auto color=[&](size_t i,int band){float fade=std::min({1.f,along[i]/1.2f,(along.back()-along[i])/1.2f});float noise=std::sin(along[i]*1.9f)*5;
   return rgb(169+noise,125+noise,76+noise,opacity[band]*std::max(0.f,fade));};
  rlSetTexture(rlGetTextureIdDefault());
  for(size_t i=1;i<points.size();i++)for(int band=0;band<6;band++){
   Vector2 a=project(points[i-1]+normals[i-1]*(lateral[band]*width(i-1))),b=project(points[i]+normals[i]*(lateral[band]*width(i)));
   Vector2 c=project(points[i]+normals[i]*(lateral[band+1]*width(i))),d=project(points[i-1]+normals[i-1]*(lateral[band+1]*width(i-1)));
   roadTriangle(a,b,c,color(i-1,band),color(i,band),color(i,band+1));roadTriangle(a,c,d,color(i-1,band),color(i,band+1),color(i-1,band+1));
  }rlSetTexture(0);
  // Interrupted wheel ruts, small stones and alternating caravan footprints.
  float nextPrint=.3f,nextStone=.2f;
  for(size_t i=1;i<points.size();i++){
   float fade=std::clamp(std::min(along[i],along.back()-along[i]),0.f,1.f);
   for(float side:{-1.f,1.f})if(int(along[i]*1.8f)%5!=0){
    auto a=project(points[i-1]+normals[i-1]*(side*.48f*width(i-1))),b=project(points[i]+normals[i]*(side*.48f*width(i)));
    line(a.x,a.y,b.x,b.y,rgb(137,104,66,fade*40),1.25f);
    line(a.x,a.y-.8f,b.x,b.y-.8f,rgb(247,210,153,fade*38),.65f);
   }
   if(along[i]>=nextPrint){
    unsigned h=Game::hash(int(nextPrint*31),int(points.front().x*97));float side=(int(nextPrint/.46f)%2?.17f:-.17f);
    auto q=project(points[i]+normals[i]*side);V d=normalized(points[i]-points[i-1]);auto toe=project(points[i]+normals[i]*side+d*.055f);
    line(q.x,q.y,toe.x,toe.y,rgb(119,93,63,fade*(57+h%24)),2.1f);
    oval(q.x,q.y,1.6f,.9f,rgb(136,102,65,fade*35));nextPrint+=.46f;
   }
   if(along[i]>=nextStone){
    unsigned h=Game::hash(int(nextStone*41),int(points.front().y*67));float side=(int(h%241)-120)*.01f*road.width;
    auto q=project(points[i]+normals[i]*side);
    if(h%4==0){oval(q.x,q.y+1,2.2f,.95f,rgb(117,91,59,60));oval(q.x,q.y,1.5f,.85f,rgb(188,155,111));line(q.x-1,q.y-.5f,q.x+.5f,q.y-.5f,rgb(237,204,151),.6f);}
    nextStone+=.27f;
   }
  }
 }EndBlendMode();
}
