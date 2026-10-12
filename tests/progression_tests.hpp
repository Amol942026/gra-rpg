#pragma once

void earnLevel(Game& g){g.xp=79;Enemy foe{90,0,g.player,g.player,1,1};g.hit(foe,10);}
void progressionTests(){
    Game start=emptyArena();
    check(start.mana==100&&start.maxMana()==100&&start.maxHp()==100,"new hero starts with 100 mana and 100 HP");
    check(start.magicDamage()>start.damage(),"initial fireball damage is greater than sword damage");
    earnLevel(start);
    check(start.level==2&&start.attributePoints==5&&start.allocationRequired,"one level grants five points and requires the allocation menu");
    start.enemies.push_back({0,0,{17,15},{17,15},100,100});start.enemies.back().windup=.01f;
    start.fireballs.push_back({{16,15},{8,0},8,28});start.mana=50;start.manaRegenTime=4.99f;
    V position=start.player,enemy=start.enemies.back().pos,ball=start.fireballs[0].pos;
    float time=start.time,health=start.hp;
    for(int i=0;i<120;i++)start.update(1.f/60,{1,0},true);
    check(distance(start.player,position)==0&&distance(start.enemies.back().pos,enemy)==0&&distance(start.fireballs[0].pos,ball)==0,
        "level pause freezes player, monsters and flying projectiles");
    check(start.time==time&&start.hp==health&&start.mana==50&&start.enemies.back().windup==.01f,
        "level pause freezes clocks, enemy strikes and mana regeneration");
    check(!start.attack()&&!start.spell()&&!start.startRoll({1,0})&&!start.use({Place::Bag,1}),"no combat or potion action can bypass mandatory level pause");
    Attributes partial{1,1,1,0,0,0},negative{-1,0,0,0,0,6},extra{6,0,0,0,0,0};
    check(!start.distribute(partial)&&!start.distribute(negative)&&!start.distribute(extra)&&start.attributePoints==5,
        "incomplete, negative and oversized allocations do not spend points");
    check(start.distribute({1,1,0,0,2,1})&&start.skillAllocationPending()&&start.attributePoints==0,"valid attributes advance to mandatory skill allocation");
    start.update(.05f,{1,0},true);check(start.time==time,"world stays frozen between attribute and skill confirmations");
    check(start.trainSkills({1,0})&&!start.allocationRequired,"skill confirmation resumes the game atomically");
    check(start.damage()==16&&start.magicDamage()==31&&start.maxHp()==120&&start.maxMana()==105,"allocated attributes change physical damage, magic damage, HP and mana separately");
    check(!start.distribute({5,0,0,0,0,0}),"a confirmed level cannot spend the same points again");
    start.update(.05f,{},false);check(start.time>time&&(start.fireballs.empty()||distance(start.fireballs[0].pos,ball)>0),"projectiles and clocks resume after confirmation");

    Game multi=emptyArena();multi.xp=250;Enemy foe{90,0,multi.player,multi.player,1,1};multi.hit(foe,10);
    check(multi.level==3&&multi.attributePoints==10&&multi.allocationRequired,"two simultaneous level gains provide ten points");
    check(multi.distribute({0,0,0,0,5,5})&&multi.maxHp()==150&&multi.maxMana()==125,"multiple levels preserve exact 10 HP and 5 mana per point");
    for(int index=0;index<AttributeCount;index++){
        Game build=emptyArena();earnLevel(build);Attributes allocation{};allocation[index]=5;build.distribute(allocation);
        check(build.damage()==(index==0?24:14)&&build.magicDamage()==(index==1?43:28),"strength and magic power affect only their own attack type");
        check(build.maxHp()==(index==4?150:100)&&build.maxMana()==(index==5?125:100),"HP and mana growth comes only from the corresponding attribute");
        check(build.incomingDamage(25,DamageType::Physical)==(index==2?21.4f:23.65f),"physical defense reduces physical damage only");
        check(build.incomingDamage(25,DamageType::Magic)==(index==3?22.75f:25),"magic defense reduces magical damage only");
    }
    for(int fps:{30,60,120}){
        Game mana=emptyArena();mana.mana=0;
        for(int i=0;i<fps*5-1;i++)mana.update(1.f/fps,{},false);
        check(mana.mana==0,"mana is not restored before five seconds");
        mana.update(1.f/fps,{},false);check(mana.mana==1,"one mana point is restored at five seconds at any FPS");
        for(int i=0;i<fps*10;i++)mana.update(1.f/fps,{},false);
        check(mana.mana==3,"mana restores three points in fifteen seconds at any FPS");
    }
    Game full=emptyArena();full.manaRegenTime=4.9f;full.update(.05f,{},false);full.spell({1,0});
    for(int i=0;i<98;i++)full.update(.05f,{},false);
    check(full.mana==90,"full mana cannot bank regeneration time before a spell");
    for(int i=0;i<2;i++)full.update(.05f,{},false);
    check(full.mana==91,"regeneration restarts normally after spending mana");
    full.mana=100;for(int i=0;i<200;i++)full.update(.05f,{},false);check(full.mana==100,"mana regeneration respects maximum capacity");

    Game manaCost=emptyArena();manaCost.mana=10;
    check(manaCost.spell({1,0})&&manaCost.mana==0&&manaCost.fireballs.size()==1,"fireball consumes exactly ten mana and creates one projectile");
    check(!manaCost.spell({1,0})&&manaCost.mana==0&&manaCost.fireballs.size()==1,"cooldown cannot create a duplicate fireball");
    manaCost.spellCd=0;manaCost.mana=9;
    check(!manaCost.spell({1,0})&&manaCost.mana==9&&manaCost.fireballs.size()==1,"insufficient mana does not consume resources or create a projectile");
    manaCost.chooseSpell(Spell::None);manaCost.mana=100;
    check(!manaCost.spell({1,0})&&manaCost.mana==100,"casting requires an equipped spell");
    check(manaCost.chooseSpell(Spell::Fireball)&&manaCost.spell({1,0}),"choosing fireball enables casting again");
    for(int type=0;type<EnemyTypeCount;type++){
        Game magic=emptyArena();V target{19,15};magic.enemies.push_back({0,type,target,target,100,100});magic.enemies[0].windup=100;
        magic.spell({1,0});check(magic.enemies[0].hp==100,"fireball must travel before dealing damage");
        for(int i=0;i<30;i++)magic.update(.05f,{},false);
        float expected=100-damageAfterArmor(28,magic.enemies[0].magicDefense());
        check(std::abs(magic.enemies[0].hp-expected)<.001f&&magic.fireballs.empty(),"fireball uses magical resistance and damages its victim once");
    }
    Game blocked=emptyArena();blocked.props.push_back({{17,15},0,1});V target{21,15};
    blocked.enemies.push_back({0,0,target,target,100,100});blocked.enemies[0].windup=100;blocked.spell({1,0});
    for(int i=0;i<30;i++)blocked.update(.05f,{},false);
    check(blocked.enemies[0].hp==100&&blocked.fireballs.empty(),"scenery stops a fireball before a distant enemy");
    Game far=emptyArena();target={26,15};far.enemies.push_back({0,0,target,target,100,100});far.enemies[0].windup=100;far.spell({1,0});
    for(int i=0;i<30;i++)far.update(.05f,{},false);
    check(far.enemies[0].hp==100&&far.fireballs.empty(),"fireball expires instead of reaching targets beyond its range");
    Game group=emptyArena();group.xp=75;
    group.enemies.push_back({0,0,{18,15},{18,15},1,36});group.enemies.push_back({1,0,{18.1f,15.5f},{18.1f,15.5f},1,36});
    group.enemies[0].windup=group.enemies[1].windup=100;group.spell({1,0});
    for(int i=0;i<30&&!group.allocationRequired;i++)group.update(.05f,{},false);
    check(group.kills==2&&group.attributePoints==5&&group.allocationRequired&&group.ground.empty()&&!group.enemies[0].loot[0].empty()&&!group.enemies[1].loot[0].empty(),
        "fireball explosion finishes all nearby hits and corpse loot before level pause");
}

