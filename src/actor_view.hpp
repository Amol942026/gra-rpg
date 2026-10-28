#pragma once
void hero(float x,float y,float s,bool portrait) {
    AnimationState state=game.animation;
    if(portrait){state={};state.direction=Direction::SE;}
    float progress=portrait?-1:game.swingProgress();
    float attack=progress>=0?std::sin(progress*PI):0;
    float roll=portrait?-1:game.rollProgress();
    float fall=portrait?-1:game.knockdownProgress(),fallR=0,fallF=-1;
    if(fall>=0)actor_art::prepareKnockdown(game,state,fallR,fallF);
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
    actor_art::shadowCacheKey=portrait?-1:0;
    actor_art::draw(0,x,y,s,state,attack,!game.equipment[1].empty(),!game.equipment[2].empty(),archetype(game.equipment[0].kind),false,BLANK,nullptr,roll,progress,portrait?-1:game.bowDrawing?game.bowCharge():game.bowReleaseTime>0?game.lastBowCharge*game.bowReleaseTime/.18f:-1,game.ammoCount(),!portrait&&game.empoweredPose(),&game.equipment,-1,fall,fallR,fallF);
    actor_art::shadowCacheKey=-1;
}
void monster(Enemy& e,float x,float y) {
    sand_prison::drawCaster(e,x,y);
    if(e.windup>0){
        Color warning=e.attackType()==DamageType::Magic?Blue:rgb(215,109,84);
        ellipseOutline(x,y,36,18,warning);
        ellipseOutline(x,y,32,16,warning);
    }
    bool underMouse=hovered.kind==HoverKind::Enemy&&hovered.index==e.id;
    actor_art::shadowCacheKey=100+e.id;
    actor_art::drawEnemy(e,x,y,underMouse?rgb(255,140,104):BLANK);
    actor_art::shadowCacheKey=-1;
    guardian_art::held(e);
    if(e.hp<e.maxHp||underMouse){
          float w=enemyBoss(e.type)||e.type==7?98:48,yy=y-enemyInfo(e.type).visualHeight;
          if(e.type==3&&e.boulderWindup>0){auto socket=Game::boulderHand(e,e.boulderProgress());yy=std::min(yy,screen(socket.ground).y-socket.height-BoulderSpriteSize*.5f-10);}
        rect(x-w/2,yy,w,5,Ink);rect(x-w/2,yy,w*std::max(0.f,e.hp)/e.maxHp,5,rgb(175,86,75));
    }
}

