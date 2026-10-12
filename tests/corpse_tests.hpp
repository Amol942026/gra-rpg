#pragma once

int bodyLootCount(const Enemy& e){int count=0;for(const auto& i:e.loot)if(!i.empty())count++;return count;}
void corpseTests(){
    for(int type=0;type<EnemyTypeCount;type++){
        Game g=emptyArena();V p=g.player+V{1,0};g.enemies.push_back({71,type,p,p,1,100});
        g.enemies[0].windup=.3f;g.hit(g.enemies[0],20);
        Enemy& body=g.enemies[0];
        check(!body.alive&&body.hp==0&&distance(body.pos,p)==0&&body.windup==0,"death keeps a stationary corpse at the actual monster location");
        int generated=bodyLootCount(body);
        check(g.ground.empty()&&generated>=1&&generated<=CorpseLootSize,"each species keeps its variable loot inside its corpse without overflowing slots");
        check(body.loot[0].kind==Kind::Gold&&body.loot[0].count>=enemyLootTable(type)[0].minCount&&body.loot[0].count<=enemyLootTable(type)[0].maxCount,"corpse preserves the coin range configured for its species");
        if(type==3)check(body.loot[1].kind==Kind::Sword&&body.loot[1].tier==2&&body.loot[2].kind==Kind::Ring&&body.loot[2].tier==2,"boss keeps both legendary rewards in its body");
        int before=total(g),kills=g.kills;g.hit(body,200);
        check(g.kills==kills&&total(g)==before&&g.ground.empty(),"hitting a corpse cannot duplicate loot or XP");
        check(!g.transfer({Place::Corpse,0,71},{Place::Bag,8}),"corpse loot requires opening that exact body first");
        if(g.allocationRequired){
            check(!g.openCorpse(71),"a boss level-up requires allocating points before opening its body");
            Attributes points{};points[0]=g.attributePoints;check(g.distribute(points),"attribute allocation continues to skills");SkillRanks learned{};learned[0]=g.skillPoints;check(g.trainSkills(learned),"skill allocation resumes corpse interaction");
        }
        check(g.openCorpse(71)&&g.openedCorpse==71,"nearby corpse can be opened by stable enemy ID");
        check(g.transfer({Place::Corpse,0,71},{Place::Bag,8})&&body.loot[0].empty()&&g.bag[8].kind==Kind::Gold&&total(g)==before,"taking corpse loot into backpack conserves items exactly");
        check(!g.transfer({Place::Corpse,0,71},{Place::Bag,9}),"an already emptied slot cannot be taken twice");
        for(int i=0;i<120;i++)g.update(1.f/60,{},false);
        check(g.findCorpse(71)&&distance(body.pos,p)==0&&bodyLootCount(body)==generated-1,"corpse and remaining loot persist while the world runs");
    }
    Game g=emptyArena();
    for(int id:{41,83}){V p=g.player+V{float(id==41?1:-1),0};g.enemies.push_back({id,0,p,p,1,36});g.hit(g.enemies.back(),10);}
    g.enemies[0].loot={};g.enemies[0].loot[0]=item(Kind::Health,0,8);g.enemies[0].loot[1]=item(Kind::Sword,1);
    g.bag[10]=item(Kind::Health,0,98);g.openCorpse(41);int before=total(g);
    check(g.transfer({Place::Corpse,0,41},{Place::Bag,10})&&g.bag[10].count==99&&g.enemies[0].loot[0].count==7&&total(g)==before,"partial stack keeps excess in the corpse without duplication");
    check(!g.transfer({Place::Corpse,0,41},{Place::Bag,10})&&total(g)==before,"a full destination stack cannot consume corpse loot");
    check(!g.transfer({Place::Corpse,1,41},{Place::Bag,0})&&g.bag[0].kind==Kind::Health&&total(g)==before,"different occupied backpack item is preserved instead of swapped into a corpse");
    check(!g.transfer({Place::Corpse,1,41},{Place::Equip,0})&&!g.drop({Place::Corpse,1,41},g.player),"loot must enter the backpack before equipping or dropping it");
    g.hp=10;check(!g.use({Place::Corpse,0,41})&&g.hp==10,"corpse potions cannot be consumed before collection");
    check(!g.transfer({Place::Bag,2},{Place::Corpse,4,41})&&total(g)==before,"backpack items cannot be inserted into corpse loot slots");
    g.openCorpse(83);
    check(!g.transfer({Place::Corpse,1,41},{Place::Bag,12}),"switching bodies invalidates a previous body source");
    g.closeCorpse();check(g.openedCorpse==-1&&bodyLootCount(g.enemies[0])==2,"closing a body keeps its contents intact");
    g.openCorpse(41);g.player={25,25};
    check(!g.transfer({Place::Corpse,1,41},{Place::Bag,12})&&!g.openCorpse(41),"distant bodies cannot be opened or looted");
    g.update(.01f,{},false);check(g.openedCorpse==-1,"walking out of range closes the corpse panel");
    g.player={15,15};g.openCorpse(41);g.allocationRequired=true;
    check(!g.openCorpse(83)&&!g.transfer({Place::Corpse,1,41},{Place::Bag,12}),"mandatory advancement blocks corpse actions");
    g.allocationRequired=false;g.hp=0;
    check(!g.openCorpse(83)&&!g.transfer({Place::Corpse,1,41},{Place::Bag,12}),"dead player cannot loot corpses");
    g.hp=100;
    check(!g.openCorpse(999)&&g.get({Place::Corpse,-1,41})==nullptr&&g.get({Place::Corpse,CorpseLootSize,41})==nullptr&&g.get({Place::Corpse,0,999})==nullptr,"invalid body IDs and slot bounds are safely rejected");
    g.openCorpse(41);g.transfer({Place::Corpse,0,41},{Place::Bag,12});g.transfer({Place::Corpse,1,41},{Place::Bag,13});g.closeCorpse();
    check(g.openCorpse(41)&&bodyLootCount(g.enemies[0])==0&&g.findCorpse(41),"empty bodies remain on the map and reopen without regenerating rewards");
}
