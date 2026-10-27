#pragma once
#include "tower_rooms_art.hpp"

void drawTowerFurniture(int index){
 const auto& object=towerFurnishings(game.towerFloor)[index];Vector2 p=screen(object.pos);
 tower_furniture::draw(object.type,p,game.time,game.towerFloor,index,hovered.kind==HoverKind::TowerObject&&hovered.index==index);
}
void drawTowerWorld(){
    clip(0,0,WorldW,H);ClearBackground(rgb(14,15,30));Vector2 c=screen(TowerCenter);
    Color accent=towerAccent(game.towerFloor);
    DrawRectangleGradientV(0,0,WorldW,H,rgb(18,19,38),rgb(34,29,47));
    bool balcony=game.towerFloor==TowerBalconyFloor;
    if(balcony)tower_balcony::panorama();
    intro_art::glow(c.x,c.y-220,600,380,Fade(accent,.13f));
    for(int i=-2;i<=2;i++){
        if(balcony&&i>0)continue; // The terrace side is open to the valley.
        float x=c.x+i*162,y=c.y-320+std::abs(i)*19;
        tower_furniture::drawSprite(tower_furniture::Window,{x,y+125});
        clip(x-48,y-114,96,228);
        for(int star=0;star<18;star++){
            float xx=x-40+(Game::hash(i+9,star)%80),yy=y-106+(Game::hash(star,i+29)%185);
            if(game.towerFloor==4||game.towerFloor==3)intro_art::star(xx,yy,star%4?1.3f:2.5f,Fade(accent,.35f+.2f*std::sin(game.time+star)));
            else if(star<5){line(xx,yy,xx+18,yy-9,Fade(accent,.16f));ellipse(xx,yy-7,5,10,Fade(accent,.11f));}
        }
        EndScissorMode();clip(0,0,WorldW,H);
    }
    const auto& floor=towerFloorArt[game.towerFloor];
    if(floor.id)DrawTexturePro(floor.texture,{0,0,1024,-600},{c.x-512,c.y-300,1024,600},{0,0},0,WHITE);else drawTowerGround(c,game.towerFloor);
    if(balcony)tower_balcony::terrace(false);
    world_shadows::draw();
    if(game.towerFloor==3){for(int i=0;i<3;i++)if(game.towerRuneMask&(1<<i)){Vector2 p=screen(towerFurnishings(3)[i].pos),q=screen({12,11.3f});line(p.x,p.y-90,q.x,q.y-90,Fade(towerAccent(3),.22f),2);}}
    drawHoveredTile();
    for(const auto& item:game.ground)if(game.groundVisible(item)&&!item.value.empty()){
        Vector2 p=screen(item.pos);ellipse(p.x,p.y,18,8,Fade(rarity(item.value),.18f));icon(item.value,p.x,p.y-14,.58f);
    }
    for(auto object:worldDrawOrder()){
        if(object.type==7)drawTowerFurniture(object.index);
        else if(object.type==8)drawTowerStairs(object.index);
        else if(object.type==6){
            Vector2 p=screen(MagePosition);if(hovered.kind==HoverKind::Mage)ellipseOutline(p.x,p.y,31,14,Gold);
            intro_art::mage(p.x,p.y,1,game.time,towerStory.phase==TowerPhase::TravelOut?storyEase(towerStory.age/.9f):0);
        }else if(object.type==2){Vector2 p=screen(game.player);ellipseOutline(p.x,p.y+2,25,12,Fade(Gold,.6f));hero(p.x,p.y,1);}
    }
    if(balcony){tower_balcony::terrace(true);tower_balcony::sign();}
    drawHealingAura();
    DrawRectangleGradientV(0,0,WorldW,160,rgb(9,12,26,238),rgb(9,12,26,0));
    DrawRectangleGradientV(0,680,WorldW,220,rgb(9,12,26,0),rgb(9,12,26,250));EndScissorMode();
}
