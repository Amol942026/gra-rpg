#pragma once
Rectangle enemyHealthBar(const Enemy& enemy,Vector2 p){
    const float w=enemyBoss(enemy.type)||enemy.type==7?98.f:48.f;
    float y=p.y-enemyInfo(enemy.type).visualHeight;
    if(enemy.type==3&&enemy.boulderWindup>0){
        const auto socket=Game::boulderHand(enemy,enemy.boulderProgress());
        y=std::min(y,screen(socket.ground).y-socket.height-BoulderSpriteSize*.5f-10);
    }
    return {p.x-w/2,y,w,5};
}
void enemyVitals(const Enemy& enemy,Vector2 p,bool name){
    const Rectangle bar=enemyHealthBar(enemy,p);
    if(name){
        const char* label=enemyInfo(enemy.type).name;
        const float width=textWidth(label,15);
        rect(p.x-width*.5f-5,bar.y-20,width+10,18,Fade(Ink,.86f));
        center(label,p.x,bar.y-19,15,Text);
    }
    rect(bar.x-1,bar.y-1,bar.width+2,7,Ink);
    rect(bar.x,bar.y,bar.width*std::clamp(enemy.hp/enemy.maxHp,0.f,1.f),bar.height,rgb(175,86,75));
}
void hero(float x,float y,float s,bool portrait,actor_art::Silhouette* fallbackSilhouette) {
    AnimationState portraitState{};
    portraitState.direction=Direction::SE;
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
    actor_art::shadowCacheKey=portrait?-1:0;
    actor_art::drawPlayer(game,x,y,s,portrait?&portraitState:nullptr,nullptr,fallbackSilhouette);
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
    actor_art::drawEnemy(e,x,y,underMouse?Gold:BLANK);
    actor_art::shadowCacheKey=-1;
    if(regionalEnemy(e.type)&&e.type!=GlassReaperType&&e.strike>0){
        float age=std::clamp(1-e.strike/meleeRecovery(e.type),0.f,1.f);
        const bool spores=e.type==SporekinType;
        const bool sand=e.type==DuneEyeType;
        const bool jackal=e.type==DuneJackalType;
        Color dust=spores?rgb(218,183,106):sand||jackal?rgb(223,179,106):rgb(253,111,62);
        float heading=jackal&&e.animation.facingInitialized?e.animation.facingAngle:int(e.animation.direction)*PI/4;
        Vector2 forward{std::cos(heading),std::sin(heading)};
        Vector2 origin{x+forward.x*22,y+forward.y*12-(sand?33:spores?20:jackal?11:27)};
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
    if(e.hp<e.maxHp&&!underMouse)enemyVitals(e,{x,y},false);
}

