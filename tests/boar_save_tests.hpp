#pragma once
void boarSaveTests(){
 auto old=saves::read("art/gameplay/boar-charge/fixtures/legacy-v7.efs");
 for(const auto& e:old.game.enemies)check(!e.boarBusy()&&e.boarChargeCd==BoarChargeFirstDelay&&e.boarCharges==0,"real v7 save receives safe charge defaults");
 roundtrip(old);
 saves::Snapshot s;s.game.props.clear();s.game.enemies.clear();s.game.forage.clear();s.game.player={20,20};
 s.game.enemies.push_back({91,BoarType,{16,20},{16,20},88,88});s.game.enemies[0].boarChargeCd=0;
 std::array<bool,5> seen{};
 for(int frame=0;frame<120;frame++){
  s.game.update(1.f/60,{},false);const auto& e=s.game.enemies[0];
  int phase=e.boarWindup>0?0:e.boarRush>0?(e.boarHit?2:1):e.boarRecovery>0?3:4;
  if(seen[phase])continue;seen[phase]=true;roundtrip(s);
  auto original=s,restored=saves::decode(saves::encode(s));
  for(int i=0;i<90;i++){original.game.update(1.f/60,{},false);restored.game.update(1.f/60,{},false);}
  check(saves::encode(original)==saves::encode(restored),"v8 resumes windup, rush before/after contact, recovery and cooldown exactly");
 }
 for(bool phase:seen)check(phase,"all five real charge phases were reached and persisted");
 auto invalid=s;invalid.game.enemies[0].boarChargeCd=-.01f;rejects([&]{saves::encode(invalid);},"negative charge cooldown refused");
 invalid=s;invalid.game.enemies[0].boarRush=BoarChargeDuration+.1f;rejects([&]{saves::encode(invalid);},"oversized rush clock refused");
 invalid=s;invalid.game.enemies[0].boarRush=.2f;invalid.game.enemies[0].boarDirection={3,0};rejects([&]{saves::encode(invalid);},"nonunit charge direction refused");
 invalid=s;invalid.game.enemies[0].boarRush=.2f;invalid.game.enemies[0].boarDirection={1,0};invalid.game.enemies[0].boarWindup=.1f;rejects([&]{saves::encode(invalid);},"simultaneous charge phases refused");
 invalid=s;invalid.game.enemies[0].type=1;invalid.game.enemies[0].boarWindup=.1f;rejects([&]{saves::encode(invalid);},"charge on another species refused");
 invalid=s;invalid.game.enemies[0].alive=false;invalid.game.enemies[0].boarWindup=.1f;rejects([&]{saves::encode(invalid);},"charge on a corpse refused");
 invalid=s;invalid.game.enemies[0].boarHit=true;rejects([&]{saves::encode(invalid);},"orphaned charge contact refused");
 invalid=s;invalid.game.enemies[0].boarAim.x=std::numeric_limits<float>::quiet_NaN();rejects([&]{saves::encode(invalid);},"nonfinite charge target refused");
}
