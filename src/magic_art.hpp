#pragma once

void fireIcon(float x,float y,float size=1,float phase=0){
    if(spell_art::icon(Spell::Fireball,x,y,size))return;
    const Color orange=rgb(243,125,48),hot=rgb(255,217,132);
    DrawCircleGradient(int(x),int(y),21*size,Fade(orange,.25f),BLANK);
    poly({{x-13*size,y+13*size},{x-16*size,y-2*size},{x-6*size,y-13*size},
          {x-4*size,y-3*size},{x+3*size,y-(24+std::sin(phase)*3)*size},
          {x+13*size,y-6*size},{x+15*size,y+8*size},{x+7*size,y+17*size}},orange);
    poly({{x-7*size,y+12*size},{x-5*size,y+1*size},{x+2*size,y-10*size},
          {x+8*size,y+6*size},{x+4*size,y+15*size}},hot);
}
void drawFireball(const Fireball& ball){
    Vector2 p=screen(ball.pos),direction{(ball.velocity.x-ball.velocity.y)*40,(ball.velocity.x+ball.velocity.y)*20};
    float size=std::max(.001f,std::hypot(direction.x,direction.y));direction.x/=size;direction.y/=size;
    Color color=spell_art::tint(ball.spell);auto motion=spell_motion::sample(int(ball.spell),ball.age);
    float angle=std::atan2(direction.y,direction.x)*RAD2DEG;
    ellipse(p.x,p.y,ball.spell==Spell::LightningOrb?19:14,6,Fade(color,.18f));
    for(int i=6;i>0;i--){float distance=i*(ball.spell==Spell::IceShard?8.f:6.f);Vector2 at{p.x-direction.x*distance,p.y-30-direction.y*distance};DrawCircleV(at,(ball.spell==Spell::LightningOrb?9.f:6.f)-i*.65f,Fade(color,.38f-i*.045f));}
    float extent=ball.spell==Spell::LightningOrb?80:ball.spell==Spell::IceShard?88:72;
    if(spell_art::draw(ball.spell,1,{p.x,p.y-30},extent*motion.scale,extent*motion.scale,angle+motion.rotation,motion.alpha))return;
    ellipse(p.x,p.y,14,6,rgb(226,111,38,42));
    for(int i=5;i>0;i--)DrawCircleV({p.x-direction.x*i*6,p.y-30-direction.y*i*6},7-i*.8f,Fade(rgb(253,149,62),.48f-i*.06f));
    fireIcon(p.x,p.y-30,.65f,game.time*17);
    DrawCircleV({p.x,p.y-25},3,rgb(255,240,180));
    for(int n=0;n<4;n++){float age=std::fmod(game.time*3+n*.25f,1.f),side=std::sin(n*7.f+game.time*5)*5;
        Vector2 spark{p.x-direction.x*(8+age*23)-direction.y*side,p.y-30-direction.y*(8+age*23)+direction.x*side-age*6};
        DrawCircleV(spark,1.4f-age*.7f,Fade(rgb(255,207,112),(1-age)*.7f));}
}
void drawExplosion(const Explosion& blast){
    Vector2 p=screen(blast.pos);float age=1-blast.life/spellDefinition(blast.spell).impactLife;
    float alpha=std::clamp(1-age,0.f,1.f);
    Color color=spell_art::tint(blast.spell);float radius=(blast.spell==Spell::LightningOrb?125:blast.spell==Spell::IceShard?92:100)*(0.5f+age);
    if(spell_art::draw(blast.spell,2,{p.x,p.y-24},radius,radius,age*23,alpha)){
        ellipseOutline(p.x,p.y,radius*.42f,radius*.21f,Fade(color,alpha*.7f));
        for(int i=0;i<12;i++){float a=i*PI/6+age;float r=12+age*(blast.spell==Spell::LightningOrb?72:50);Vector2 at{p.x+std::cos(a)*r,p.y-25+std::sin(a)*r*.5f-age*14};
            if(blast.spell==Spell::IceShard)diamond(at.x,at.y,2.5f,6,Fade(color,alpha));
            else DrawCircleV(at,2.2f,Fade(color,alpha));}
        return;
    }
    DrawCircleGradient(int(p.x),int(p.y-22),28+age*38,Fade(rgb(254,162,63),alpha*.48f),BLANK);
    ellipseOutline(p.x,p.y,20+age*40,10+age*20,Fade(rgb(255,174,83),alpha));
    for(int n=0;n<5;n++){float a=n*2.39996f;Vector2 smoke{p.x+std::cos(a)*age*31,p.y-28+std::sin(a)*age*16-age*27};
        ellipse(smoke.x,smoke.y,5+age*8,4+age*6,Fade(rgb(98,86,65),alpha*age*.45f));}
    for(int i=0;i<12;i++){
        float angle=i*PI/6,radius=12+age*52;
        diamond(p.x+std::cos(angle)*radius,p.y-24+std::sin(angle)*radius*.5f-age*14,2.5f,4,Fade(Gold,alpha));
    }
}
