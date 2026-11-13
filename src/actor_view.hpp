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
    actor_art::draw(0,x,y,s,state,attack,!game.equipment[1].empty(),!game.equipment[2].empty(),archetype(game.equipment[0].kind),false,BLANK,nullptr,roll,progress,portrait?-1:game.visibleBowCharge(),game.ammoCount(),!portrait&&game.empoweredPose(),&game.equipment,-1,fall,fallR,fallF,portrait?PowerStyle::Count:game.activePowerVisual(),game.powerVisualAge(),portrait?-1:game.powerWindupProgress());
    actor_art::shadowCacheKey=-1;
}
void monster(Enemy& e,float x,float y) {
    sand_prison::drawCaster(e,x,y);
    if(e.eruptionWindup>0)ellipseOutline(x,y,62,30,Fade(rgb(255,159,88),.7f));
    if(chargingSpecies(e.type)&&e.boarWindup>0){
        const auto& charge=chargeProfile(e.type);
        float progress=1-e.boarWindup/charge.windup;
        Color warning=Fade(e.type==CinderhornType?rgb(255,125,70):rgb(244,154,73),.4f+.5f*progress);
        V direction=normalized(e.boarAim-e.pos),side{-direction.y,direction.x};
        V delta=direction*(e.type==CinderhornType?charge.speed*charge.duration:std::min(distance(e.pos,e.boarAim),charge.speed*charge.duration));
        V end=e.pos+delta*std::min(1.f,game.chargeObstacleTime(e,delta));
        const float width=e.type==CinderhornType?.74f:.55f;
        auto a=screen(e.pos+side*width),b=screen(end+side*width),c=screen(end-side*width),d=screen(e.pos-side*width);
        poly({a,b,c,d},Fade(warning,.1f+.08f*progress));
        line(a.x,a.y,b.x,b.y,warning,2);line(d.x,d.y,c.x,c.y,warning,2);
        auto tip=screen(end),left=screen(end-direction*.45f+side*.4f),right=screen(end-direction*.45f-side*.4f);
        line(left.x,left.y,tip.x,tip.y,warning,2);line(right.x,right.y,tip.x,tip.y,warning,2);
        ellipseOutline(x,y,34+progress*5,17+progress*2,warning);
    }
    if(chargingSpecies(e.type)&&e.boarRush>0){
        // Bounded ground dust; existing creature gait still follows actual travel.
        for(int i=0;i<3;i++){
            auto p=screen(e.pos-e.boarDirection*(.5f+i*.35f));
            float pulse=.5f+.5f*std::sin(game.time*26-i*1.7f);
            const Color ash=e.type==CinderhornType?rgb(177,125,106):rgb(161,139,99);
            ellipse(p.x,p.y,5+i*2+pulse*2,2+i,Fade(ash,.27f-i*.06f));
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
    if(regionalEnemy(e.type)&&e.strike>0){
        float age=std::clamp(1-e.strike/.22f,0.f,1.f);
        const bool spores=e.type==SporekinType;
        const bool sand=e.type==DuneEyeType;
        Color dust=spores?rgb(218,183,106):sand?rgb(223,179,106):rgb(253,111,62);
        float heading=int(e.animation.direction)*PI/4;
        Vector2 forward{std::cos(heading),std::sin(heading)};
        Vector2 origin{x+forward.x*22,y+forward.y*12-(sand?33:spores?20:27)};
        float radius=(sand?32:spores?21:17)*age;
        for(int i=0;i<9;i++){
            float a=heading+(i-4)*.25f;
            float travel=radius*(.65f+(i%3)*.24f);
            Vector2 at{origin.x+std::cos(a)*travel,origin.y+std::sin(a)*travel*.55f-(i%3)*5*age};
            ellipse(at.x,at.y,(spores?3.5f:2.f)*(1-.5f*age),1.7f,Fade(dust,(1-age)*.72f));
        }
        if(sand)ellipseOutline(origin.x,origin.y,6+radius,(6+radius)*.34f,Fade(rgb(98,225,207),(1-age)*.73f));
    }
    guardian_art::held(e);
    if(e.hp<e.maxHp||underMouse){
          float w=enemyBoss(e.type)||e.type==7?98:48,yy=y-enemyInfo(e.type).visualHeight;
          if(e.type==3&&e.boulderWindup>0){auto socket=Game::boulderHand(e,e.boulderProgress());yy=std::min(yy,screen(socket.ground).y-socket.height-BoulderSpriteSize*.5f-10);}
        rect(x-w/2,yy,w,5,Ink);rect(x-w/2,yy,w*std::max(0.f,e.hp)/e.maxHp,5,rgb(175,86,75));
    }
}

