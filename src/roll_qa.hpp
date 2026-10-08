#pragma once

void exportRollQA() {
    fs::create_directories("qa-v04");fs::create_directories("assets/generated/v04");
    renderScale=1;typography.setScale(1);
    const char* compass[]={"E","SE","S","SW","W","NW","N","NE"};
    for(bool labelled:{false,true}){
        RenderTexture2D sheet=LoadRenderTexture(labelled?1632:1536,labelled?1216:1152);
        BeginTextureMode(sheet);ClearBackground(labelled?Ink:BLANK);
        if(labelled)txt("PRZETURLANIE — OSIEM KIERUNKÓW",24,16,24,Text);
        for(int d=0;d<8;d++){
            if(labelled)txt(compass[d],18,144.f*d+142,16,Gold);
            for(int frame=0;frame<12;frame++){
                float x=frame*128.f+64+(labelled?80:0),y=d*144.f+125+(labelled?56:0);
                if(labelled)diamond(x,y,48,24,rgb(30,46,44));
                AnimationState pose;pose.direction=static_cast<Direction>(d);
                actor_art::draw(0,x,y,1,pose,0,true,true,Kind::Sword,false,BLANK,nullptr,frame/11.f);
            }
        }
        EndTextureMode();saveTexture(sheet,labelled?"qa-v04/roll-directions.png":"assets/generated/v04/hero-roll.png");
        UnloadRenderTexture(sheet);
    }
    game=Game{};mouse={0,0};welcome=true;prepareCanvas(1.2f);
    renderScene();saveTexture(canvas,"qa-v04/welcome.png");
    welcome=false;game.startRoll({1,-1});
    for(int i=0;i<9;i++)game.update(1.f/60,{},false);
    renderScene();saveTexture(canvas,"qa-v04/rolling.png");
}
