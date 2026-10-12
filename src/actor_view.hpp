#pragma once
void hero(float x,float y,float s,bool portrait) {
    AnimationState state=game.animation;
    if(portrait){state.direction=Direction::SE;state.moving=false;state.phase=0;}
    float progress=portrait?-1:game.swingProgress();
    float attack=progress>=0?std::sin(progress*PI):0;
    float roll=portrait?-1:game.rollProgress();
    if(roll>=0){
        Vector2 heading{(game.rollDirection.x-game.rollDirection.y)*40,(game.rollDirection.x+game.rollDirection.y)*20};
        float len=std::hypot(heading.x,heading.y);if(len>0){heading.x/=len;heading.y/=len;}
        for(int i=0;i<4;i++){
            float trail=(i*10+12)*s;
            ellipse(x-heading.x*trail,y-heading.y*trail+3*s,(6-i)*s,(3-i*.4f)*s,
                    Fade(Gold,std::sin(roll*PI)*(.22f-i*.04f)));
        }
        if(roll*RollDuration<RollProtection)ellipseOutline(x,y+2*s,28*s,13*s,Fade(Gold,.8f));
    }
    actor_art::draw(0,x,y,s,state,attack,!game.equipment[1].empty(),!game.equipment[2].empty(),game.equipment[0].kind,false,BLANK,nullptr,roll,progress,portrait?-1:game.bowDrawing?game.bowCharge():game.bowReleaseTime>0?game.lastBowCharge*game.bowReleaseTime/.18f:-1,game.ammoCount(),!portrait&&game.empoweredPose());
}
void monster(Enemy& e,float x,float y) {
    if(e.windup>0){
        Color warning=e.attackType()==DamageType::Magic?Blue:rgb(215,109,84);
        ellipseOutline(x,y,36,18,warning);
        ellipseOutline(x,y,32,16,warning);
    }
    float attack=e.windup>0?-std::clamp(1-e.windup/.42f,0.f,1.f):e.strike>0?std::sin((1-e.strike/.22f)*PI):0;
    bool underMouse=hovered.kind==HoverKind::Enemy&&hovered.index==e.id;
    actor_art::draw(e.type+1,x,y,1,e.animation,attack,false,false,Kind::Sword,e.flash>0,underMouse?rgb(255,140,104):BLANK);
    if(e.hp<e.maxHp||underMouse){
        float w=e.type==3||e.type==7?78:48,yy=y-enemyInfo(e.type).visualHeight;
        rect(x-w/2,yy,w,5,Ink);rect(x-w/2,yy,w*std::max(0.f,e.hp)/e.maxHp,5,rgb(175,86,75));
    }
}

