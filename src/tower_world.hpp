#pragma once
#include "tower_rooms_art.hpp"

void drawTowerFurniture(int index){
    const auto& object=towerFurnishings(game.towerFloor)[index];Vector2 p=screen(object.pos);float x=p.x,y=p.y;
    intro_art::glow(x,y,48,17,rgb(5,6,16,135));
    if(hovered.kind==HoverKind::TowerObject&&hovered.index==index)ellipseOutline(x,y,object.radius*40+8,object.radius*20+4,Gold);
    if(object.type>2){drawThemedFurniture(object,x,y);return;}
    if(object.type==0){
        ellipse(x,y,29,15,rgb(31,34,54));ellipse(x,y-5,28,13,rgb(120,117,143));
        DrawRectangleRounded({x-18,y-157,36,145},.28f,12,rgb(86,83,108));
        DrawRectangleRounded({x-14,y-155,11,141},.32f,12,rgb(139,131,158));
        for(int i=0;i<7;i++){float yy=y-22-i*19;line(x-15,yy,x+16,yy+1,rgb(65,64,89));line(x-13,yy+2,x+14,yy+3,rgb(148,136,163));}
        ellipse(x,y-154,25,11,rgb(74,72,99));ellipse(x,y-163,26,10,rgb(165,145,169));
        intro_art::glow(x,y-181,43,46,rgb(115,90,242,53));diamond(x,y-183,9,22,rgb(154,151,246));diamond(x-2,y-188,3,11,rgb(230,215,255));
    }else if(object.type==1){
        for(int side:{-1,1})DrawRectangleRounded({x+side*24-4,y-5,8,28},.4f,10,rgb(50,33,39));
        ellipse(x,y-13,49,25,rgb(59,38,43));ellipse(x,y-20,49,23,rgb(120,76,63));ellipseOutline(x,y-21,46,21,rgb(183,134,92));
        poly({{x-30,y-25},{x-8,y-34},{x+2,y-20},{x-20,y-11}},rgb(225,204,154));
        poly({{x+2,y-20},{x+23,y-31},{x+34,y-18},{x+11,y-8}},rgb(204,182,143));
        for(int i=0;i<4;i++){line(x-23,y-24+i*2.8f,x-7,y-29+i*2.8f,rgb(133,99,83));line(x+10,y-18+i*2.2f,x+25,y-25+i*2.2f,rgb(133,99,83));}
        ellipse(x+30,y-39,9,5,rgb(52,44,68));rect(x+27,y-55,6,17,rgb(208,188,133));
        intro_art::glow(x+30,y-61,32,30,rgb(255,155,76,42));ellipse(x+30,y-60,3,7,Gold);
    }else{
        ellipse(x,y-7,31,15,rgb(117,103,130));DrawRectangleRounded({x-19,y-62,38,52},.22f,10,rgb(73,63,94));
        ellipse(x,y-61,31,14,rgb(166,146,177));ellipse(x,y-65,27,12,rgb(76,70,115));
        float lift=std::sin(game.time*1.8f)*3;intro_art::glow(x,y-96+lift,57,56,rgb(125,131,247,44));
        ellipse(x,y-96+lift,21,23,rgb(101,125,211));ellipse(x-5,y-101+lift,12,14,rgb(170,183,249));
        intro_art::arc(x,y-96+lift,34,9,0,2*PI,Fade(Gold,.65f),1.2f);
        for(int i=0;i<5;i++){float a=i*2*PI/5+game.time*.4f;intro_art::star(x+std::cos(a)*31,y-96+lift+std::sin(a)*15,3,Gold);}
    }
}
void drawTowerWorld(){
    clip(0,0,WorldW,H);ClearBackground(rgb(14,15,30));Vector2 c=screen(TowerCenter);
    Color accent=towerAccent(game.towerFloor);
    DrawRectangleGradientV(0,0,WorldW,H,rgb(18,19,38),rgb(34,29,47));
    intro_art::glow(c.x,c.y-220,600,380,Fade(accent,.13f));
    for(int i=-2;i<=2;i++){
        float x=c.x+i*162,y=c.y-320+std::abs(i)*19;
        DrawRectangleRounded({x-65,y-133,130,258},.5f,20,rgb(64,56,88));
        DrawRectangleRounded({x-54,y-122,108,244},.48f,20,rgb(13,20,43));
        DrawRectangleRounded({x-48,y-116,96,236},.47f,20,shade(accent,.19f));
        clip(x-48,y-114,96,228);
        for(int star=0;star<18;star++){
            float xx=x-40+(Game::hash(i+9,star)%80),yy=y-106+(Game::hash(star,i+29)%185);
            if(game.towerFloor==4||game.towerFloor==3)intro_art::star(xx,yy,star%4?1.3f:2.5f,Fade(accent,.35f+.2f*std::sin(game.time+star)));
            else if(star<5){line(xx,yy,xx+18,yy-9,Fade(accent,.16f));ellipse(xx,yy-7,5,10,Fade(accent,.11f));}
        }
        EndScissorMode();clip(0,0,WorldW,H);
        rect(x-2,y-112,4,229,rgb(89,82,118));line(x-48,y+20,x+48,y+20,rgb(104,91,122),3);
        for(int j=0;j<6;j++)line(x-78,y-102+j*37,x-66,y-102+j*37,rgb(107,92,125));
    }
    const auto& floor=towerFloorArt[game.towerFloor];
    if(floor.id)DrawTexturePro(floor.texture,{0,0,1024,-600},{c.x-512,c.y-300,1024,600},{0,0},0,WHITE);else drawTowerGround(c,game.towerFloor);
    if(game.towerFloor==3){for(int i=0;i<3;i++)if(game.towerRuneMask&(1<<i)){Vector2 p=screen(towerFurnishings(3)[i].pos),q=screen({12,11.3f});line(p.x,p.y-90,q.x,q.y-90,Fade(towerAccent(3),.22f),2);}}
    drawHoveredTile();
    for(const auto& item:game.ground)if(game.groundVisible(item)&&!item.value.empty()){
        Vector2 p=screen(item.pos);ellipse(p.x,p.y,18,8,Fade(rarity(item.value.tier),.18f));icon(item.value,p.x,p.y-14,.58f);
    }
    for(auto object:worldDrawOrder()){
        if(object.type==7)drawTowerFurniture(object.index);
        else if(object.type==8)drawTowerStairs(object.index);
        else if(object.type==6){
            Vector2 p=screen(MagePosition);if(hovered.kind==HoverKind::Mage)ellipseOutline(p.x,p.y,31,14,Gold);
            intro_art::mage(p.x,p.y,1,game.time,0);
            center("ELDRIN",p.x,p.y-144,15,rgb(205,172,248),true);
            center(game.mageReachable()?"E / PPM · ROZMAWIAJ":"MAG · MENTOR",p.x,p.y-123,14,game.mageReachable()?Gold:Muted);
        }else if(object.type==2){Vector2 p=screen(game.player);ellipseOutline(p.x,p.y+2,25,12,Fade(Gold,.6f));hero(p.x,p.y,1);}
    }
    drawTowerStairLabels();drawHealingAura();drawCombatEffects();
    DrawRectangleGradientV(0,0,WorldW,160,rgb(9,12,26,238),rgb(9,12,26,0));
    DrawRectangleGradientV(0,680,WorldW,220,rgb(9,12,26,0),rgb(9,12,26,250));EndScissorMode();
}
