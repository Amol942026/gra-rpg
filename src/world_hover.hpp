#pragma once

std::array<std::vector<unsigned char>,PropTypeCount> propMasks;
std::array<Texture2D,PropTypeCount> propOutlines{};

void initHoverArt(){
 for(int type=0;type<PropTypeCount;type++){
  Image source=LoadImageFromTexture(propArt[type].texture);ImageFlipVertical(&source);Color* pixels=LoadImageColors(source);int w=source.width,h=source.height,radius=int(3*w/(180*propArtScale(type)));
  auto& mask=propMasks[type];mask.resize(w*h);for(int i=0;i<w*h;i++)mask[i]=pixels[i].a>=160?255:0;UnloadImageColors(pixels);UnloadImage(source);
  Image border=GenImageColor(w,h,BLANK);auto* out=static_cast<Color*>(border.data);
  for(int y=0;y<h;y++)for(int x=0;x<w;x++)if(!mask[y*w+x]){bool edge=false;for(int dy=-radius;dy<=radius&&!edge;dy++)for(int dx=-radius;dx<=radius;dx++){int xx=x+dx,yy=y+dy;if(dx*dx+dy*dy<=radius*radius+1&&xx>=0&&xx<w&&yy>=0&&yy<h&&mask[yy*w+xx]){edge=true;break;}}if(edge)out[y*w+x]=WHITE;}
  propOutlines[type]=LoadTextureFromImage(border);UnloadImage(border);SetTextureFilter(propArt[type].texture,TEXTURE_FILTER_BILINEAR);SetTextureFilter(propOutlines[type],TEXTURE_FILTER_BILINEAR);
 }
}
void unloadHoverArt(){for(auto texture:propOutlines)if(texture.id)UnloadTexture(texture);}

std::vector<DrawObject> worldDrawOrder() {
    std::vector<DrawObject> objects;
    if(game.inTower()){
        for(int i=0;i<int(towerFurnishings(game.towerFloor).size());i++){V p=towerFurnishings(game.towerFloor)[i].pos;objects.push_back({p.x+p.y,7,i});}
        if(game.towerFloor==0)objects.push_back({MagePosition.x+MagePosition.y,6,0});
        if(game.stairPresent(1))objects.push_back({TowerStairUp.x+TowerStairUp.y,8,1});
        if(game.stairPresent(-1))objects.push_back({TowerStairDown.x+TowerStairDown.y,8,-1});objects.push_back({game.player.x+game.player.y,2,0});
        std::stable_sort(objects.begin(),objects.end(),[](const auto& a,const auto& b){return a.depth<b.depth;});return objects;
    }
    objects.reserve(game.props.size()+game.enemies.size()+1+game.duneDust.size()+game.boulderImpacts.size()*guardian_break::Pieces);
    for(int i=0;i<int(game.props.size());i++)objects.push_back({game.props[i].pos.x+game.props[i].pos.y,0,i});
    for(int i=0;i<int(game.enemies.size());i++)
        objects.push_back({game.enemies[i].pos.x+game.enemies[i].pos.y+(game.enemies[i].alive?0.f:-.03f),game.enemies[i].alive?1:4,i});
    objects.push_back({game.player.x+game.player.y,2,0});
    for(int i=0;i<TreasureCount;i++)if(game.chests[i].location==game.location)objects.push_back({game.chests[i].pos.x+game.chests[i].pos.y,9,i});
    if(game.hasFountains())for(int i=0;i<FountainCount;i++)objects.push_back({game.fountains()[i].x+game.fountains()[i].y,12,i});
    for(int i=0;i<int(game.fireballs.size());i++)objects.push_back({game.fireballs[i].pos.x+game.fireballs[i].pos.y,3,i});
    for(int i=0;i<int(game.cinders.size());i++)objects.push_back({game.cinders[i].pos.x+game.cinders[i].pos.y,14,i});
    for(int i=0;i<int(game.venomGlobs.size());i++)objects.push_back({game.venomGlobs[i].pos.x+game.venomGlobs[i].pos.y,15,i});
    for(int i=0;i<int(game.duneDust.size());i++){
        const V at=game.duneDust[i].center();
        objects.push_back({at.x+at.y+game.elevation(at)*.8f,16,i});
    }
    for(int i=0;i<int(game.arrows.size());i++)objects.push_back({game.arrows[i].pos.x+game.arrows[i].pos.y,5,i});
    for(int i=0;i<int(game.boulders.size());i++)objects.push_back({game.boulders[i].pos.x+game.boulders[i].pos.y,10,i});
    for(int i=0;i<int(game.boulderImpacts.size());i++)for(int piece=0;piece<guardian_break::Pieces;piece++){
        const auto& impact=game.boulderImpacts[i];auto q=guardian_break::sample(piece,impact.progress());
        if(q.alpha>0)objects.push_back({impact.pos.x+impact.pos.y+q.y/20,11,i*guardian_break::Pieces+piece});
    }
    for(auto& object:objects){V p{};bool grounded=true;
        switch(object.type){case 0:p=game.props[object.index].pos;break;case 1:case 4:p=game.enemies[object.index].pos;break;case 2:p=game.player;break;
        case 9:p=game.chests[object.index].pos;break;case 12:p=game.fountains()[object.index];break;
        case 3:p=game.fireballs[object.index].pos;break;case 5:p=game.arrows[object.index].pos;break;case 14:p=game.cinders[object.index].pos;break;
        case 15:p=game.venomGlobs[object.index].pos;break;
        case 10:p=game.boulders[object.index].pos;break;case 11:p=game.boulderImpacts[object.index/guardian_break::Pieces].pos;break;default:grounded=false;}
        if(grounded)object.depth+=game.elevation(p)*.8f;
    }
    std::stable_sort(objects.begin(),objects.end(),[](const auto& a,const auto& b){return a.depth<b.depth;});
    return objects;
}

bool worldPointerAllowed(Vector2 p) {
    if(developerMenuOpen||towerPanelOpen||intro.active()||towerStory.active()||game.storyBlocked()||weaponWheelOpen||welcome||help||paused||audioOpen||statsOpen||skillsOpen||spellBookOpen||quickPickerSlot>=0||game.allocationRequired||game.dead())return false;
    if(!CheckCollisionPointRec(p,{0,116,WorldW,678}))return false;
    if(archetype(game.equipment[0].kind)==Kind::Bow&&CheckCollisionPointRec(p,{195,112,515,36}))return false;
    if(CheckCollisionPointRec(p,minimapRect()))return false; // minimap
    if(developerSummonHudHit(p))return false;
    if(CheckCollisionPointRec(p,journalRect()))return false;
    if(game.messageTime>0){
        float width=std::min(760.f,textWidth(game.message,13)+32);
        if(CheckCollisionPointRec(p,{WorldW/2.f-width/2,744,width,29}))return false;
    }
    return true;
}

struct PropMaskRow {
    const unsigned char* pixels=nullptr;
    float left=0,pixelsPerPoint=0;
    int width=0;
    bool contains(float x)const{
        const float local=(x-left)*pixelsPerPoint;
        return pixels&&local>=0&&local<width&&pixels[int(local)];
    }
};
PropMaskRow propMaskRow(const Prop& prop,Vector2 base,float worldY){
    const float size=propArtSize(prop);
    const float sourceY=(worldY-base.y)/size+208;
    const auto texture=propArt[prop.type].texture;
    const auto& pixels=propMasks[prop.type];
    if(sourceY<0||sourceY>=230||pixels.empty())return {};
    const int row=int(sourceY*texture.height/230);
    const float shift=tree_wind::moves(prop.type)?treeShift(prop,sourceY,game.time):0;
    return {pixels.data()+row*texture.width,base.x+(shift-90)*size,texture.width/(180*size),texture.width};
}
bool propContains(const Prop& prop,Vector2 point) {
    if(prop.type==13&&volcano_art::gateFrame().id)return volcano_art::portalContains(prop,point);
    if(volcanoSceneryProp(prop.type))return volcano_art::propContains(prop,point);
    return propMaskRow(prop,screen(prop.pos),point.y).contains(point.x);
}
float propOpacity(const Prop& prop,Vector2 base,Vector2 player,bool inFront,actor_art::Silhouette& silhouette){
    if(!inFront)return 1.f;
    const float size=propArtSize(prop);
    const Rectangle art={base.x-110*size,base.y-208*size,220*size,230*size};
    const Rectangle heroBounds={player.x-actor_art::Silhouette::Width,player.y-156,
                                actor_art::Silhouette::Width*2,actor_art::Silhouette::Height*2};
    if(!CheckCollisionRecs(art,heroBounds))return 1.f;
    if(!silhouette.ready){
        const auto heroRig=std::find_if(actor_art::frameRigs.begin(),actor_art::frameRigs.end(),
                                       [](const auto& entry){return entry.first==0;});
        if(heroRig==actor_art::frameRigs.end())return 1.f;
        heroRig->second.rasterize(silhouette);
    }
    const bool atlas=(prop.type==13&&volcano_art::gateFrame().id)||volcanoSceneryProp(prop.type);
    // Incidental contact with a leaf or a weapon tip does not hide the hero.
    const int hidden=std::max(12,(silhouette.count+9)/10);
    int covered=0;
    for(int row=silhouette.top;row<=silhouette.bottom;row++){
        const float y=silhouette.point(0,row).y;
        if(y<art.y||y>=art.y+art.height)continue;
        const PropMaskRow mask=atlas?PropMaskRow{}:propMaskRow(prop,base,y);
        if(!atlas&&!mask.pixels)continue;
        for(int col=silhouette.left;col<=silhouette.right;col++){
            if(!silhouette.has(col,row))continue;
            const Vector2 point=silhouette.point(col,row);
            const bool blocks=atlas?propContains(prop,point):mask.contains(point.x);
            if(blocks&&++covered>=hidden)return .38f;
        }
    }
    return 1.f;
}

WorldHover pickWorld(Vector2 point) {
    WorldHover hit;
    if(!worldPointerAllowed(point))return hit;
    V world=toWorld(point);
    hit.tile={std::round(world.x),std::round(world.y)};
    hit.tileValid=hit.tile.x>=0&&hit.tile.x<MapSize&&hit.tile.y>=0&&hit.tile.y<MapSize;
    if(game.inTower())hit.tileValid=game.walkable(hit.tile);
    // Reverse of the actual drawing order: never select an enemy through a tree.
    auto objects=worldDrawOrder();
    for(auto it=objects.rbegin();it!=objects.rend();++it){
        if(it->type==10||it->type==11||it->type==16)continue; // Scattered particles do not obstruct inspection.
        if(it->type==3||it->type==5||it->type==14||it->type==15)continue; // Projectiles do not obstruct mouse inspection.
        V position{};bool touched=false;HoverKind kind=HoverKind::None;int id=it->index;
        if(it->type==12){
            position=game.fountains()[it->index];kind=HoverKind::Fountain;touched=fountain_art::contains(it->index,point);
        }else if(it->type==9){
            const auto& chest=game.chests[it->index];position=chest.pos;id=chest.id;kind=HoverKind::Chest;
            touched=treasureContains(chest,point);
        }else if(it->type==6){
            position=MagePosition;Vector2 p=screen(position);kind=HoverKind::Mage;
            float dx=(point.x-p.x+4)/30,dy=(point.y-p.y+62)/62;
            touched=dx*dx+dy*dy<=1||CheckCollisionPointRec(point,{p.x-47,p.y-112,12,112});
        }else if(it->type==7){
            position=towerFurnishings(game.towerFloor)[it->index].pos;Vector2 p=screen(position);int type=towerFurnishings(game.towerFloor)[it->index].type;
            kind=HoverKind::TowerObject;
            touched=tower_furniture::contains(type,p,point,game.time,game.towerFloor);
        }else if(it->type==8){
            position=it->index==1?TowerStairUp:TowerStairDown;Vector2 p=screen(position);kind=HoverKind::TowerStair;
            touched=tower_stairs::contains(it->index,p,point);
        }else if(it->type==0){
            const Prop& prop=game.props[it->index];position=prop.pos;
            touched=propContains(prop,point);kind=HoverKind::Prop;
        }else if(it->type==1){
            const Enemy& e=game.enemies[it->index];position=e.pos;
            Vector2 p=screen(e.pos);kind=HoverKind::Enemy;id=e.id;
            float half=enemyBoss(e.type)?125:e.type==EmberDrakeType?90:75;
            if(e.type==VolcanoBossType&&(e.windup>0||e.strike>0))half=163;
            const bool raisedBlade=e.type==ObsidianSentinelType&&(e.volcanoAction()>=0||e.abilityBusy());
            if(raisedBlade)half=110;
            const bool newRig=caldera_quadruped::handles(e.type);
            const float height=newRig?caldera_quadruped::hoverAbove(e.type):enemyInfo(e.type).visualHeight+(raisedBlade?46.f:0.f);
            const float low=newRig?caldera_quadruped::HoverBelow:raisedBlade?45.f:14.f;
            if(regionalEnemy(e.type)||CheckCollisionPointRec(point,{p.x-half,p.y-height,half*2,height+low})){
                touched=actor_art::drawEnemy(e,p.x,p.y,BLANK,&point);
            }
        }else if(it->type==4){
            const Enemy& e=game.enemies[it->index];position=e.pos;kind=HoverKind::Corpse;id=e.id;
            Vector2 p=screen(position);
            float half=e.type==VolcanoBossType?170:enemyBoss(e.type)?120:e.type==EmberDrakeType?110:96;
            float height=!e.corpseGrounded()?enemyInfo(e.type).visualHeight+28:enemyBoss(e.type)?115.f:92.f;
            const Rectangle area=e.corpseGrounded()&&fallen_art::cachedBody(e.type)?cachedCorpseBounds(e,p):
                Rectangle{p.x-half,p.y-height,half*2,height+52};
            if(regionalEnemy(e.type)||CheckCollisionPointRec(point,area))touched=corpseArt(e,p.x,p.y,BLANK,&point);
        }else{
            position=game.player;Vector2 p=screen(position);
            if(CheckCollisionPointRec(point,{p.x-112,p.y-140,224,182})){
                touched=actor_art::drawPlayer(game,p.x,p.y,1,nullptr,&point);
            }
        }
        if(touched){
            hit.tile={std::round(position.x),std::round(position.y)};hit.tileValid=true;
            hit.kind=kind;hit.index=kind==HoverKind::None?-1:id;return hit;
        }
    }
    for(auto it=game.ground.rbegin();it!=game.ground.rend();++it){
        if(!game.groundVisible(*it))continue;
        Vector2 p=screen(it->pos);
        if(!it->value.empty()&&CheckCollisionPointRec(point,{p.x-16,p.y-30,32,34})){
            hit.kind=HoverKind::Ground;hit.index=it->id;
            hit.tile={std::round(it->pos.x),std::round(it->pos.y)};hit.tileValid=true;return hit;
        }
    }
    for(auto it=game.forage.rbegin();it!=game.forage.rend();++it){
        if(game.inTower())break;
        Vector2 p=screen(it->pos);
        if(!it->value.empty()&&wildMushroomContains(p,point)){
            hit.kind=HoverKind::Forage;hit.index=it->id;hit.tile={std::round(it->pos.x),std::round(it->pos.y)};hit.tileValid=true;return hit;
        }
    }
    return hit;
}

void drawHoveredTile() {
    if(!hovered.tileValid||hovered.kind==HoverKind::Enemy)return;
    Color color=Gold;
    Vector2 corners[]={screen(hovered.tile+V{-.5f,-.5f}),screen(hovered.tile+V{.5f,-.5f}),screen(hovered.tile+V{.5f,.5f}),screen(hovered.tile+V{-.5f,.5f})};
    poly({corners[0],corners[1],corners[2],corners[3]},Fade(color,.20f));
    for(int i=0;i<4;i++)DrawLineEx(corners[i],corners[(i+1)%4],2,Fade(color,.95f));
}

void drawHoveredName() {
    if(hovered.kind!=HoverKind::Prop&&hovered.kind!=HoverKind::Corpse&&hovered.kind!=HoverKind::Forage&&hovered.kind!=HoverKind::Mage&&hovered.kind!=HoverKind::TowerObject&&hovered.kind!=HoverKind::TowerStair&&hovered.kind!=HoverKind::Chest&&hovered.kind!=HoverKind::Fountain)return;
    std::string name;Color color=Gold;
    if(hovered.kind==HoverKind::Fountain){name=std::string(fountainName(hovered.index))+(game.fountainUsed[hovered.index]?" · wyczerpane":" · E / LPM / PPM");color=hovered.index==0?Blue:rgb(238,123,123);}
    else if(hovered.kind==HoverKind::TowerStair){name=hovered.index==1?"Schody w górę · E / PPM":"Schody w dół · E / PPM";}
    else if(hovered.kind==HoverKind::Chest){name="Skrzynia skarbów · E / LPM / PPM";}
    else if(hovered.kind==HoverKind::TowerObject){const auto& object=towerFurnishings(game.towerFloor)[hovered.index];name=towerObjectName(object.type);if(towerInteractive(object.type))name+=" · E / PPM";}
    else if(hovered.kind==HoverKind::Mage){name=game.mageReachable()?"Eldrin · E / PPM: porozmawiaj":"Eldrin · podejdź bliżej";}
    else if(hovered.kind==HoverKind::Corpse){
        Enemy* e=game.findCorpse(hovered.index);if(!e)return;
        name=std::string("Ciało: ")+enemyInfo(e->type).name+" · PPM: otwórz";
    }else if(hovered.kind==HoverKind::Forage){
        name="Borowik · pospolity · +10 HP";color=WHITE;
    }else{
        name=propInfo(game.props[hovered.index].type).name;if(game.props[hovered.index].type==13)name+=" · E / PPM";
    }
    float width=textWidth(name,14)+24;
    float x=std::clamp(mouse.x+18,8.f,WorldW-width-8),y=std::clamp(mouse.y+18,118.f,728.f);
    panel({x,y,width,30},rgb(12,22,25,248));txt(name,x+12,y+7,14,color);
}

