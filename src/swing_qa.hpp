#pragma once

void exportSwingQA() {
    fs::create_directories("qa-v07");fs::create_directories("assets/generated/v07");
    renderScale=1;typography.setScale(1);
    const char* names[]={"E","SE","S","SW","W","NW","N","NE"};
    for(bool labelled:{false,true}){
        RenderTexture2D sheet=LoadRenderTexture(labelled?1728:1664,labelled?1344:1280);
        BeginTextureMode(sheet);ClearBackground(labelled?Ink:BLANK);
        if(labelled)txt("CIĘCIE PRZED BOHATEREM — OSIEM KIERUNKÓW",24,19,24,Text);
        for(int d=0;d<8;d++)for(int frame=0;frame<8;frame++){
            float x=frame*208.f+104+(labelled?64:0),y=d*160.f+135+(labelled?64:0);
            if(labelled){diamond(x,y,46,22,rgb(30,46,44));if(frame==0)txt(names[d],15,y-62,15,Gold);}
            AnimationState state;state.direction=static_cast<Direction>(d);
            float progress=frame/7.f;
            actor_art::draw(0,x,y,1,state,std::sin(progress*PI),true,true,Kind::Sword,false,BLANK,nullptr,-1,progress);
        }
        EndTextureMode();saveTexture(sheet,labelled?"qa-v07/sword-directions.png":"assets/generated/v07/hero-attack.png");
        UnloadRenderTexture(sheet);
    }
    game=Game{};game.props.clear();game.enemies.clear();welcome=help=paused=audioOpen=false;mouse={0,0};
    V position=game.player+normalized(V{1,-1})*2.06f;
    game.enemies.push_back({0,1,position,position,54,54});game.enemies[0].animation.direction=Direction::W;
    game.animation.direction=Direction::E;game.attack();game.swing=SwingDuration*.5f;game.enemies[0].flash=0;
    prepareCanvas(1.2f);renderScene();saveTexture(canvas,"qa-v07/forward-swing.png");
}
