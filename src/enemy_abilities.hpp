#pragma once
// Included inside ember after V and realm IDs. Existing attack clips supply
// the poses; these profiles only schedule their anticipation/action/recovery.
enum class EnemyAbility {None,VenomFan,CopperWave,ObsidianCleave,EmberBreath};
struct EnemyAbilityProfile {const char* name;float windup,active,recovery,cooldown,minRange,maxRange,damageScale;DamageType damageType;float duration()const{return windup+active+recovery;}};
inline EnemyAbility enemyAbility(int type){return type==10?EnemyAbility::VenomFan:type==9?EnemyAbility::CopperWave:type==16?EnemyAbility::ObsidianCleave:type==17?EnemyAbility::EmberBreath:EnemyAbility::None;}
inline const EnemyAbilityProfile& enemyAbilityProfile(int type){
 static constexpr std::array<EnemyAbilityProfile,5> values={{{"",0,0,0,3,0,0,0,DamageType::Physical},
  {"Jadowity wachlarz",.75f,.15f,.55f,6.2f,.75f,6.f,1.1f,DamageType::Magic},
  {"Miedziana fala",.85f,.7f,.7f,7.2f,.75f,3.3f,1.25f,DamageType::Physical},
  {"Rozłupujący cios",1.05f,.18f,.85f,7.8f,.75f,5.2f,1.55f,DamageType::Physical},
  {"Żarowy oddech",.95f,1.05f,.65f,7.5f,.75f,5.2f,1.05f,DamageType::Magic}}};
 return values[int(enemyAbility(type))];
}
inline bool enemyAbilityRealm(int type,WorldLocation realm){return (type==9||type==10)?realm==WorldLocation::Desert:(type==16||type==17)&&realm==WorldLocation::Volcano;}
inline V rotateAbilityDirection(V p,float angle){float c=std::cos(angle),s=std::sin(angle);return {p.x*c-p.y*s,p.x*s+p.y*c};}
inline bool insideEnemyCleave(V relative,V direction){float along=relative.x*direction.x+relative.y*direction.y,side=relative.x*direction.y-relative.y*direction.x;return along>=.3f&&along<=5.2f&&std::abs(side)<=.72f;}
inline bool insideEmberBreath(V relative,V direction){float radius=length(relative);return radius<=4.8f&&(radius<.15f||relative.x*direction.x+relative.y*direction.y>=radius*.819152f);}
// Earliest contact with an expanding annulus, including a moving hero/caster.
// Both circle boundaries have quadratic roots; endpoint-only tests tunnel.
inline float expandingWaveContact(V relative,V delta,float radius,float growth,float margin){
 std::array<float,6> candidates{};int count=0;candidates[count++]=0;candidates[count++]=1;
 for(float side:{-1.f,1.f}){
  float r=radius+side*margin,a=delta.x*delta.x+delta.y*delta.y-growth*growth,b=2*(relative.x*delta.x+relative.y*delta.y-r*growth),c=relative.x*relative.x+relative.y*relative.y-r*r;
  auto add=[&](float t){if(t>=0&&t<=1&&count<int(candidates.size()))candidates[count++]=t;};
  if(std::abs(a)<1e-7f){if(std::abs(b)>1e-7f)add(-c/b);}
  else{float d=b*b-4*a*c;if(d>=0){float root=std::sqrt(d);add((-b-root)/(2*a));add((-b+root)/(2*a));}}
 }
 for(int i=1;i<count;i++){float value=candidates[i];int j=i;while(j>0&&candidates[j-1]>value){candidates[j]=candidates[j-1];j--;}candidates[j]=value;}
 for(int i=0;i<count;i++){float t=candidates[i];if(std::abs(length(relative+delta*t)-(radius+growth*t))<=margin+.0001f)return t;}
 return 2;
}
