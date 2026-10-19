#pragma once

std::array<std::vector<unsigned char>,PropTypeCount> propMasks;
std::array<Texture2D,PropTypeCount> propOutlines{};

void initHoverArt(){
 for(int type=0;type<PropTypeCount;type++){
  Image source=LoadImageFromTexture(propArt[type].texture);ImageFlipVertical(&source);Color* pixels=LoadImageColors(source);int w=source.width,h=source.height,radius=3*w/180;
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
    objects.reserve(game.props.size()+game.enemies.size()+1);
    for(int i=0;i<int(game.props.size());i++)objects.push_back({game.props[i].pos.x+game.props[i].pos.y,0,i});
    for(int i=0;i<int(game.enemies.size());i++)
        objects.push_back({game.enemies[i].pos.x+game.enemies[i].pos.y+(game.enemies[i].alive?0.f:-.03f),game.enemies[i].alive?1:4,i});
    objects.push_back({game.player.x+game.player.y,2,0});
    for(int i=0;i<TreasureCount;i++)if(game.chests[i].location==game.location)objects.push_back({game.chests[i].pos.x+game.chests[i].pos.y,9,i});
    for(int i=0;i<int(game.fireballs.size());i++)objects.push_back({game.fireballs[i].pos.x+game.fireballs[i].pos.y,3,i});
    for(int i=0;i<int(game.arrows.size());i++)objects.push_back({game.arrows[i].pos.x+game.arrows[i].pos.y,5,i});
    std::stable_sort(objects.begin(),objects.end(),[](const auto& a,const auto& b){return a.depth<b.depth;});
    return objects;
}

bool worldPointerAllowed(Vector2 p) {
    if(developerMenuOpen||towerPanelOpen||intro.active()||towerStory.active()||game.storyBlocked()||weaponWheelOpen||welcome||help||paused||audioOpen||statsOpen||skillsOpen||spellBookOpen||game.allocationRequired||game.dead())return false;
    if(!CheckCollisionPointRec(p,{0,116,WorldW,678}))return false;
    if(CheckCollisionPointRec(p,weaponWheelButton())||CheckCollisionPointRec(p,developerButton()))return false;
    if(archetype(game.equipment[0].kind)==Kind::Bow&&CheckCollisionPointRec(p,{195,112,515,36}))return false;
    if(CheckCollisionPointRec(p,{842,133,180,155}))return false; // minimap
    if(CheckCollisionPointRec(p,{22,675,310,119}))return false; // journal
    if(game.messageTime>0){
        float width=std::min(760.f,textWidth(game.message,13)+32);
        if(CheckCollisionPointRec(p,{WorldW/2.f-width/2,744,width,29}))return false;
    }
    return true;
}

bool propContains(const Prop& prop,Vector2 point) {
    Vector2 base=screen(prop.pos);
    float sourceY=(point.y-base.y)/prop.size+208;
    float shift=treeProp(prop.type)?treeShift(prop,sourceY,game.time):0;
    int w=propArt[prop.type].texture.width,h=propArt[prop.type].texture.height;
    int x=int(std::floor(((point.x-base.x)/prop.size+90-shift)*w/180)),y=int(std::floor(sourceY*h/230));
    return x>=0&&x<w&&y>=0&&y<h&&!propMasks[prop.type].empty()&&propMasks[prop.type][y*w+x];
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
        if(it->type==3||it->type==5)continue; // Projectiles do not obstruct mouse inspection.
        V position{};bool touched=false;HoverKind kind=HoverKind::None;int id=it->index;
        if(it->type==9){
            const auto& chest=game.chests[it->index];position=chest.pos;id=chest.id;kind=HoverKind::Chest;
            touched=CheckCollisionPointRec(point,treasureBounds(screen(position)));
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
            float half=enemyBoss(e.type)?125:75;
            if(CheckCollisionPointRec(point,{p.x-half,p.y-enemyInfo(e.type).visualHeight,half*2,enemyInfo(e.type).visualHeight+14})){
                float attack=e.attackPose();
                touched=actor_art::draw(e.type+1,p.x,p.y,1,e.animation,attack,false,false,Kind::Sword,false,BLANK,&point);
            }
        }else if(it->type==4){
            const Enemy& e=game.enemies[it->index];position=e.pos;kind=HoverKind::Corpse;id=e.id;
            Vector2 p=screen(position);
            float half=enemyBoss(e.type)?110:68;
            if(CheckCollisionPointRec(point,{p.x-half,p.y-65,half*2,109}))touched=corpseArt(e,p.x,p.y,BLANK,&point);
        }else{
            position=game.player;Vector2 p=screen(position);
            if(CheckCollisionPointRec(point,{p.x-112,p.y-140,224,182})){
                float progress=game.swingProgress();float attack=progress>=0?std::sin(progress*PI):0;
                touched=actor_art::draw(0,p.x,p.y,1,game.animation,attack,!game.equipment[1].empty(),!game.equipment[2].empty(),archetype(game.equipment[0].kind),false,BLANK,&point,game.rollProgress(),progress,game.bowDrawing?game.bowCharge():game.bowReleaseTime>0?game.lastBowCharge*game.bowReleaseTime/.18f:-1,game.ammoCount(),game.empoweredPose());
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
        if(!it->value.empty()&&CheckCollisionPointRec(point,{p.x-15,p.y-27,30,30})){
            hit.kind=HoverKind::Forage;hit.index=it->id;hit.tile={std::round(it->pos.x),std::round(it->pos.y)};hit.tileValid=true;return hit;
        }
    }
    return hit;
}

void drawHoveredTile() {
    if(!hovered.tileValid)return;
    Vector2 p=screen(hovered.tile);
    Color color=hovered.kind==HoverKind::Enemy?rgb(255,140,104):Gold;
    diamond(p.x,p.y,40,20,Fade(color,.20f));
    Vector2 corners[]={{p.x,p.y-20},{p.x+40,p.y},{p.x,p.y+20},{p.x-40,p.y}};
    for(int i=0;i<4;i++)DrawLineEx(corners[i],corners[(i+1)%4],2,Fade(color,.95f));
}

void drawHoveredName() {
    if(hovered.kind!=HoverKind::Enemy&&hovered.kind!=HoverKind::Prop&&hovered.kind!=HoverKind::Corpse&&hovered.kind!=HoverKind::Forage&&hovered.kind!=HoverKind::Mage&&hovered.kind!=HoverKind::TowerObject&&hovered.kind!=HoverKind::TowerStair)return;
    std::string name;Color color=Gold;
    if(hovered.kind==HoverKind::TowerStair){name=hovered.index==1?"Schody w górę · E / PPM":"Schody w dół · E / PPM";}
    else if(hovered.kind==HoverKind::Chest){name="Skrzynia skarbów · E / LPM / PPM";}
    else if(hovered.kind==HoverKind::TowerObject){const auto& object=towerFurnishings(game.towerFloor)[hovered.index];name=towerObjectName(object.type);if(towerInteractive(object.type))name+=" · E / PPM";}
    else if(hovered.kind==HoverKind::Mage){name=game.mageReachable()?"Eldrin · E / PPM: porozmawiaj":"Eldrin · podejdź bliżej";}
    else if(hovered.kind==HoverKind::Enemy){
        Enemy* e=game.findEnemy(hovered.index);if(!e)return;
        name=std::string(enemyInfo(e->type).name)+"  "+std::to_string(int(std::ceil(e->hp)))+" / "+std::to_string(int(e->maxHp))+" HP";
        color=rgb(255,166,132);
    }else if(hovered.kind==HoverKind::Corpse){
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

