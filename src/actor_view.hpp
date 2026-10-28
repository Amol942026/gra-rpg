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
    if(e.type==BoarType&&e.boarWindup>0){
        float progress=1-e.boarWindup/BoarChargeWindup;
        Color warning=Fade(rgb(244,154,73),.4f+.5f*progress);
        V direction=normalized(e.boarAim-e.pos),side{-direction.y,direction.x};
        V end=e.pos+direction*std::min(distance(e.pos,e.boarAim),BoarChargeSpeed*BoarChargeDuration);
        auto a=screen(e.pos+side*.55f),b=screen(end+side*.55f),c=screen(end-side*.55f),d=screen(e.pos-side*.55f);
        poly({a,b,c,d},Fade(warning,.1f+.08f*progress));
        line(a.x,a.y,b.x,b.y,warning,2);line(d.x,d.y,c.x,c.y,warning,2);
        auto tip=screen(end),left=screen(end-direction*.45f+side*.4f),right=screen(end-direction*.45f-side*.4f);
        line(left.x,left.y,tip.x,tip.y,warning,2);line(right.x,right.y,tip.x,tip.y,warning,2);
        ellipseOutline(x,y,34+progress*5,17+progress*2,warning);
    }
    if(e.type==BoarType&&e.boarRush>0){
        // Bounded ground dust; existing creature gait still follows actual travel.
        for(int i=0;i<3;i++){
            auto p=screen(e.pos-e.boarDirection*(.5f+i*.35f));
            float pulse=.5f+.5f*std::sin(game.time*26-i*1.7f);
            ellipse(p.x,p.y,5+i*2+pulse*2,2+i,Fade(rgb(161,139,99),.27f-i*.06f));
        }
    }
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

