#pragma once
void drawArrow(const Arrow& arrow){
 Vector2 p=screen(arrow.pos),direction{(arrow.velocity.x-arrow.velocity.y)*40,(arrow.velocity.x+arrow.velocity.y)*20};
 float n=std::hypot(direction.x,direction.y);if(n<.001f)return;direction.x/=n;direction.y/=n;
 Vector2 tip{p.x,p.y-35},tail{tip.x-direction.x*27,tip.y-direction.y*27};
 ellipse(p.x,p.y,8,3,rgb(8,16,14,60));line(tail.x,tail.y,tip.x,tip.y,rgb(212,177,111),2);
 Vector2 cross{-direction.y,direction.x};poly({tip,{tip.x-direction.x*8+cross.x*3,tip.y-direction.y*8+cross.y*3},{tip.x-direction.x*8-cross.x*3,tip.y-direction.y*8-cross.y*3}},Text);
 for(float side:{-1.f,1.f})line(tail.x+direction.x*5,tail.y+direction.y*5,tail.x+cross.x*side*4,tail.y+cross.y*side*4,Gold,2);
}
