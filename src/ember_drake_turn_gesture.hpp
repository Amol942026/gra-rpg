#pragma once
// Included inside volcano_actors, after Part. The five Synfig poses range
// from a full counterclockwise bank through neutral to a clockwise bank.
inline void applyDrakeTurnGesture(Part& part,int row,int baseDirection,float turnLean){
    if(std::abs(turnLean)<.001f)return;
    const auto& clip=ember_drake_motion::bank;
    const float frame=(std::clamp(turnLean,-1.f,1.f)+1)*.5f*(clip.count-1);
    const int first=std::min(int(frame),clip.count-2);
    const auto& a=clip.frames[first][baseDirection][row];
    const auto& b=clip.frames[first+1][baseDirection][row];
    const float weight=frame-first;
    const auto mix=[weight](short left,short right){return left+(right-left)*weight;};
    part.shift.x+=mix(a.x,b.x)/128.f;
    part.shift.y+=mix(a.y,b.y)/128.f;
    part.angle+=mix(a.turn,b.turn)/128.f;
    part.scaleX*=1+mix(a.scaleX,b.scaleX)/4096.f;
    part.scaleY*=1+mix(a.scaleY,b.scaleY)/4096.f;
}
