#pragma once

struct FieldSite {const char* id;const char* name;WorldLocation realm;V position;};
inline constexpr FieldSite FieldSites[]={
 {"forest","Przesieka Wędrowca",WorldLocation::Forest,ForestArrival},
 {"ruins","Ruiny Mchu",WorldLocation::Forest,{21,8}},
 {"birches","Brzozowa polana",WorldLocation::Forest,{14,43}},
 {"grove","Zaciszny zagajnik",WorldLocation::Forest,{14,61}},
 {"heart","Serce Kniei",WorldLocation::Forest,ForestRitualCircle},
 {"trail","Stary trakt",WorldLocation::Forest,{39,62}},
 {"wolves","Wilcza knieja",WorldLocation::Forest,{55,21}},
 {"spiders","Pajęcze ostępy",WorldLocation::Forest,{35,42}},
 {"desert","Brama Piasków",WorldLocation::Desert,DesertArrival},
 {"desert-oasis","Oaza Bursztynu",WorldLocation::Desert,{18,39}},
 {"desert-boss","Tron Żywej Wydmy",WorldLocation::Desert,{57,49}},
 {"volcano","Brama Popiołu",WorldLocation::Volcano,VolcanoArrival},
 {"volcano-forges","Obsydianowe Kuźnie",WorldLocation::Volcano,{91,56}},
 {"volcano-boss","Tron Serca Kaldery",WorldLocation::Volcano,{56,15}},
 {"forest-eastern-ruins","Wschodnie Ruiny",WorldLocation::Forest,{83,14}},
 {"forest-ford","Ruiny Kamiennego Brodu",WorldLocation::Forest,{107,15}},
 {"forest-deep","Głęboka Knieja",WorldLocation::Forest,{80,55}},
 {"forest-pines","Sosnowe Grzbiety",WorldLocation::Forest,{115,60}},
 {"forest-crossroads","Korzenne Rozdroża",WorldLocation::Forest,{39,84}},
 {"forest-root-hills","Korzenne Wzgórza",WorldLocation::Forest,{94,92}},
 {"forest-ferns","Wąwóz Paproci",WorldLocation::Forest,{37,129}},
 {"forest-old-growth","Prastara Knieja",WorldLocation::Forest,{115,103}},
 {"forest-old-road","Wzgórza Starego Traktu",WorldLocation::Forest,{88,121}},
 {"forest-watch","Północna Czata",WorldLocation::Forest,{131,11}},
 {"forest-south-grove","Południowy Zagajnik",WorldLocation::Forest,{18,82}},
 {"forest-pine-glade","Sosnowa Polana",WorldLocation::Forest,{128,79}},
 {"forest-fern-pass","Paprotna Przełęcz",WorldLocation::Forest,{62,135}},
 {"forest-cedars","Cisowe Uroczysko",WorldLocation::Forest,{133,135}},
 {"desert-sun-ruins","Ruiny Słońca",WorldLocation::Desert,{43,15}},
 {"desert-east-dunes","Wschodnie Wydmy",WorldLocation::Desert,{79,16}},
 {"desert-copper-wind","Wydmy Miedzianego Wiatru",WorldLocation::Desert,{128,16}},
 {"desert-cistern","Wyschnięta Cysterna",WorldLocation::Desert,{121,54}},
 {"desert-amber-hills","Wzgórza Bursztynu",WorldLocation::Desert,{88,49}},
 {"desert-southern-waste","Południowe Pustkowia",WorldLocation::Desert,{22,81}},
 {"desert-caravan-crossing","Karawanowe Rozdroże",WorldLocation::Desert,{65,85}},
 {"desert-stone-trail","Kamienny Szlak",WorldLocation::Desert,{130,91}},
 {"desert-caravan-valley","Dolina Karawan",WorldLocation::Desert,{22,123}},
 {"desert-amber-ridges","Bursztynowe Grzbiety",WorldLocation::Desert,{68,130}},
 {"desert-buried","Zatopione w Piasku",WorldLocation::Desert,{135,135}},
 {"desert-watch","Strażnica Piaskowego Szlaku",WorldLocation::Desert,{101,121}},
 {"desert-salt-pass","Słona Przełęcz",WorldLocation::Desert,{51,111}},
 {"volcano-cinderfields","Popielne Pogorzelisko",WorldLocation::Volcano,{17,23}},
 {"volcano-rift","Rozłam Żaru",WorldLocation::Volcano,{38,43}},
 {"volcano-glass-slopes","Szkliste Zbocza",WorldLocation::Volcano,{132,23}},
 {"volcano-black-smoke","Kuźnie Czarnego Dymu",WorldLocation::Volcano,{115,67}},
 {"volcano-basalt-hills","Bazaltowe Wzgórza",WorldLocation::Volcano,{92,91}},
 {"volcano-ember-pass","Przełęcz Żaru",WorldLocation::Volcano,{39,80}},
 {"volcano-ash-watch","Popielna Strażnica",WorldLocation::Volcano,{14,102}},
 {"volcano-lava-narrows","Przesmyki Lawy",WorldLocation::Volcano,{39,132}},
 {"volcano-pumice-ridges","Grzbiety Pumeksu",WorldLocation::Volcano,{80,129}},
 {"volcano-obsidian-waste","Obsydianowe Pustkowie",WorldLocation::Volcano,{131,133}},
 {"volcano-fissure-watch","Czata nad Szczeliną",WorldLocation::Volcano,{131,48}},
 {"volcano-cinder-terrace","Taras Żużlu",WorldLocation::Volcano,{129,104}},
 {"volcano-red-overlook","Czerwony Próg",WorldLocation::Volcano,{70,30}},
 {"volcano-southern-cinders","Południowe Pogorzelisko",WorldLocation::Volcano,{12,90}},
 {"volcano-slag-bridge","Most nad Żużlem",WorldLocation::Volcano,{65,114}},
};
inline const FieldSite& fieldRegion(WorldLocation realm,V p){
 const FieldSite* closest=&FieldSites[0];
 float best=1.e9f;
 for(const auto& site:FieldSites)if(site.realm==realm){
  const V delta=site.position-p;
  const float d=delta.x*delta.x+delta.y*delta.y;
  if(d<best){best=d;closest=&site;}
 }
 return *closest;
}
inline bool nearFieldSite(WorldLocation realm,V p,float range){
 const auto& site=fieldRegion(realm,p);
 const V delta=site.position-p;
 return delta.x*delta.x+delta.y*delta.y<range*range;
}
