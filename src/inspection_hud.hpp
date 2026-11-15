#pragma once

struct Inspection {
    HoverKind source=HoverKind::None;
    std::string name,kind,detail,secondary;
    Color nameColor=Text;
};
Inspection inspectWorld(WorldHover hit) {
    Inspection info;
    if(hit.kind==HoverKind::Fountain&&game.hasFountains()&&hit.index>=0&&hit.index<FountainCount){
        info.source=hit.kind;info.name=fountainName(hit.index);info.nameColor=hit.index==0?Blue:rgb(238,123,123);info.kind=game.inVolcano()?"Jednorazowa kapliczka kaldery":"Jednorazowa kapliczka pustyni";
        info.detail=game.fountainSpent(hit.index)?"Wyczerpane źródło · sucha misa.":hit.index==0?"Odnawia całą manę. Jedno użycie.":"Odnawia całe zdrowie. Jedno użycie.";
        info.secondary=game.fountainSpent(hit.index)?"Moc źródła została wykorzystana.":"Podejdź i użyj E / LPM / PPM.";
    }else if(hit.kind==HoverKind::TowerStair&&game.inTower()){
        info.source=hit.kind;info.name=hit.index==1?"Schody w górę":"Schody w dół";info.kind="Przejście między piętrami";
        info.detail=std::string("Cel: ")+TowerFloors[std::clamp(game.towerFloor+hit.index,0,TowerFloorCount-1)].name;
        info.secondary="Podejdź i użyj E / LPM / PPM.";
    }else if(hit.kind==HoverKind::TowerObject&&game.inTower()&&hit.index>=0&&hit.index<int(towerFurnishings(game.towerFloor).size())){
        int type=towerFurnishings(game.towerFloor)[hit.index].type;info.source=hit.kind;info.name=towerObjectName(type);info.kind=type==MerchantObjectType?"Handlarz / przyjazny NPC":TowerFloors[game.towerFloor].name;
        info.detail=type==MerchantObjectType?"Kupuje sprzęt i materiały z bestii.":type==4?"Odnawia całe HP i manę.":type==7?"Warzenie mikstur z materiałów.":type==9?"Dotknij, aby zmienić blask runy.":"Wyposażenie wieży Eldrina.";
        info.secondary=towerInteractive(type)?"Podejdź i użyj E / LPM / PPM.":"Blokuje ruch; element wystroju.";
    }else if(hit.kind==HoverKind::Mage&&game.inTower()&&game.towerFloor==0){
        info.source=hit.kind;info.name="Eldrin";info.kind="Mag / przyjazny NPC";
        info.detail="Strażnik Pieczęci Popiołu.";
        info.secondary=game.mageReachable()?"E / PPM: porozmawiaj.":"Podejdź bliżej, aby porozmawiać.";
    }else if(hit.kind==HoverKind::Corpse){
        const Enemy* e=game.findCorpse(hit.index);if(!e)return info;
        info.source=hit.kind;info.name=std::string("Ciało: ")+enemyInfo(e->type).name;info.kind="Pokonany potwór";
        int count=0;for(const auto& value:e->loot)if(!value.empty())count++;
        info.detail=count?"Otwórz łup w panelu W zasięgu.":"Ciało jest puste.";
        info.secondary=game.corpseReachable(e->id)?"Przeciągnij przedmioty do plecaka.":"Podejdź do ciała (maks. 2,5 pola).";
    }else if(hit.kind==HoverKind::Chest){
        const auto* c=game.findChest(hit.index);if(!c)return info;
        info.source=hit.kind;info.name="Skrzynia skarbów";info.kind="Pojemnik z wyposażeniem";
        int count=0;for(const auto& value:c->loot)count+=!value.empty();
        info.detail=c->opened&&!count?"Skrzynia jest pusta.":"3 przedmioty: pospolite lub rzadkie.";
        info.secondary=game.chestReachable(c->id)?"E / LPM / PPM: otwórz łup.":"Podejdź bliżej (maks. 2,5 pola).";
    }else if(hit.kind==HoverKind::Prop&&hit.index>=0&&hit.index<int(game.props.size())){
        const auto& prop=propInfo(game.props[hit.index].type);
        info.source=hit.kind;info.name=prop.name;info.kind=prop.kind;
        info.detail=game.props[hit.index].type==13?"Powrót do wieży Eldrina.":"Przeszkoda terenowa.";info.secondary=game.props[hit.index].type==13?"Podejdź i użyj E / LPM / PPM.":"Blokuje ruch i przeturlanie.";
    }else if(hit.kind==HoverKind::Forage){
        const Item* value=game.get({Place::Forage,hit.index});if(!value||value->empty())return info;
        info.source=hit.kind;info.name=itemName(*value);info.nameColor=rarity(*value);info.kind="Grzyb jadalny";info.detail="Pospolity · +10 HP.";
        info.secondary=game.reachable({Place::Forage,hit.index})?"Przeciągnij do plecaka; PPM, aby zjeść.":"Podejdź bliżej, aby zebrać.";
    }else if(hit.kind==HoverKind::Ground){
        const Item* value=game.get({Place::Ground,hit.index});if(!value||value->empty())return info;
        info.source=hit.kind;info.name=itemName(*value);info.nameColor=rarity(*value);
        info.kind=value->kind==Kind::Mushroom?"Grzyb jadalny":gearSlot(value->kind)>=0?"Wyposażenie":material(value->kind)?"Materiał / trofeum":value->kind==Kind::Health||value->kind==Kind::Mana?"Mikstura":"Skarb";
        info.detail=std::string(rarityName(itemRarity(*value)))+"  ·  Ilość: "+std::to_string(value->count);
        info.secondary=game.reachable({Place::Ground,hit.index})?"Przeciągnij łup do plecaka.":"Podejdź bliżej, aby podnieść.";
    }
    return info;
}
void drawInspectionHud() {
    if(hovered.kind==HoverKind::Enemy)return;
    const WorldHover nearbyTarget=nearbyUnderPointer().target;
    const Inspection info=inspectWorld(nearbyTarget.kind==HoverKind::None?hovered:nearbyTarget);
    panel({1078,705,326,169},rgb(16,26,28));
    if(info.source==HoverKind::None){
        txt("Najedź na przeciwnika lub obiekt.",1090,750,16,Text);
        txt("Tutaj zobaczysz informacje o nim.",1090,777,14,Muted);
        return;
    }
    txt(info.name,1090,735,info.name.size()>24?16:22,info.nameColor);
    txt("Rodzaj: "+info.kind,1090,760,14,Muted);
    txt(info.detail,1090,796,14,Gold);txt(info.secondary,1090,826,14,Muted);
}
