#pragma once
inline Game enemyAbilityArena(int type,float range=3){
 Game game;game.activateField(type<12?WorldLocation::Desert:WorldLocation::Volcano);game.props.clear();game.enemies.clear();game.forage.clear();game.messageTime=0;
 V origin=type<12?V{36,34}:V{60,40};game.player=origin+V{range,0};float hp=enemyInfo(type).health;game.enemies.push_back({8000,type,origin,origin,hp,hp});game.enemies[0].abilityCd=0;return game;
}
