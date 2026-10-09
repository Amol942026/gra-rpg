#pragma once

// Static fallen rigs use the same geometry for drawing and pointer picking.
bool corpseArt(const Enemy& e,float x,float y,Color outline=BLANK,const Vector2* probe=nullptr){
    actor_art::Rig rig(x,y,1,e.animation.direction);
    const Color bone=rgb(160,159,130),fur=rgb(106,113,108),stone=rgb(79,98,95);
    if(e.type==0){
        rig.blob(26,8,rgb(74,112,66));
        rig.form({-8,13,5},{7,2,2},rgb(32,49,35));rig.form({8,13,5},{7,2,2},rgb(32,49,35));
    }else if(e.type==1){
        rig.form({0,0,7},{20,37,12},fur);rig.form({0,22,5},{15,17,8},rgb(129,133,117));
        rig.form({0,32,4},{11,12,6},rgb(80,87,82));
        rig.form({-6,19,12},{5,9,5},fur);rig.form({6,19,12},{5,9,5},fur);
        for(float side:{-1.f,1.f})for(float f:{-12.f,10.f})rig.beam({side*8,f,5},{side*21,f-7,3},5,5,fur);
        rig.beam({0,-18,6},{7,-36,3},7,6,fur);
    }else if(e.type==2){
        rig.form({0,22,6},{15,16,11},bone);rig.form({-4,29,7},{4,2,4},rgb(43,52,48));rig.form({4,29,7},{4,2,4},rgb(43,52,48));
        rig.beam({0,15,5},{0,-12,4},4,4,bone);
        for(float f:{0.f,5.f,10.f})rig.beam({-9,f,5},{9,f,5},3,3,bone);
        for(float side:{-1.f,1.f}){
            rig.beam({side*8,11,4},{side*20,-3,3},4,4,bone);
            rig.beam({side*4,-12,4},{side*13,-32,3},4,4,bone);
        }
        rig.beam({-23,-13,3},{-17,27,3},3,3,rgb(109,131,129));
    }else if(e.type==3){
        rig.form({0,0,9},{29,28,17},stone);rig.form({0,25,7},{19,20,13},rgb(118,129,110));
        rig.form({-26,3,5},{18,19,9},stone);rig.form({26,-2,6},{18,23,11},stone);
        rig.form({-12,-25,5},{16,17,9},stone);rig.form({14,-29,4},{16,17,7},stone);
        rig.form({0,13,17},{6,7,3},rgb(85,132,129));
    }else if(e.type==4){
        Color hide=rgb(112,82,56);rig.form({0,0,9},{29,39,17},hide);rig.form({0,25,7},{24,18,12},hide);
        for(float side:{-1.f,1.f}){
            rig.beam({side*8,30,6},{side*14,35,12},4,4,bone);
            for(float f:{-12.f,12.f})rig.beam({side*10,f,6},{side*24,f-4,3},7,7,shade(hide,.7f));
        }
    }else if(e.type==5){
        Color chitin=rgb(55,68,59);rig.form({0,-8,7},{25,27,13},chitin);rig.form({0,14,5},{20,18,10},chitin);
        for(int i=0;i<4;i++)for(float side:{-1.f,1.f})rig.beam({side*9,-15+i*10.f,6},{side*28,-20+i*10.f,2},3,3,chitin);
        rig.form({0,-8,14},{7,15,2},rgb(107,132,60));
    }else if(e.type==6){
        Color cloth=rgb(103,58,44);rig.form({0,2,6},{25,30,12},cloth);rig.form({0,26,6},{17,17,11},rgb(186,146,109));
        rig.form({0,23,12},{20,20,6},cloth);
        for(float side:{-1.f,1.f}){rig.beam({side*10,12,5},{side*21,-2,3},7,7,cloth);rig.beam({side*6,-12,4},{side*13,-31,3},7,7,rgb(55,61,53));}
        rig.beam({-22,0,3},{-16,21,3},3,3,rgb(166,180,169));
    }else{
        Color bark=rgb(90,74,51),leaf=rgb(55,89,53);rig.form({0,0,10},{29,38,19},bark);rig.form({0,29,9},{23,22,16},bark);
        for(float side:{-1.f,1.f}){
            rig.beam({side*12,7,8},{side*35,12,3},9,9,bark);rig.beam({side*8,-17,6},{side*19,-34,3},9,9,bark);
            rig.form({side*25,8,9},{22,20,11},leaf);
        }
        rig.form({0,26,18},{31,27,12},leaf);
    }
    if(probe)return rig.contains(*probe);
    ellipse(x,y+3,e.type==3?52:39,e.type==3?19:14,rgb(8,17,17,105));rig.draw(outline);return false;
}
