#pragma once
#include "equipment_shapes.hpp"
#include "items.hpp"

namespace ember::weapon_profiles {
// Dimensions are measured from the grip. The same profiles are painted
// for the equipment slots and projected on the animated hero.
struct Blade {float length,width,bend,shoulder,point,guard,hook,pommel;};
inline constexpr Blade blade(Kind k){
 switch(k){
 case Kind::CryptSword:return {37,4.3f,0,.92f,.83f,5.2f,-.11f,2.3f};
 case Kind::GuardianSword:return {35,5.1f,.35f,.98f,.73f,8.4f,.15f,2.5f};
 case Kind::DuneBlade:return {36,3.8f,4.7f,.64f,.95f,5.5f,.26f,1.8f};
 case Kind::SlimeBlade:return {32,4.1f,1.4f,.76f,.69f,5.5f,-.12f,2.1f};
 case Kind::BanditSword:return {36,4.1f,4.1f,.51f,.79f,4.9f,-.27f,2};
 case Kind::TrailFalchion:return {34,5.3f,3.5f,.67f,.87f,5.8f,.18f,1.7f};
 case Kind::DewSword:return {36,3.0f,-.9f,.56f,.73f,5.4f,.22f,1.5f};
 case Kind::SunTrailSword:return {37,4.3f,.6f,.80f,.76f,7.4f,.36f,2.4f};
 case Kind::BriarSword:return {37,4.7f,-.3f,.47f,.65f,7.2f,.25f,2.1f};
 case Kind::MirageSword:return {37,3.5f,6.7f,.60f,.93f,6.5f,.27f,2.1f};
 case Kind::EmberSword:return {37,4.5f,0,.77f,.81f,7.3f,-.22f,2.5f};
 case Kind::Dagger:return {24,3.7f,0,.68f,.72f,4.8f,.22f,1.7f};
 case Kind::WolfKnife:return {23,4.4f,-2.1f,.79f,.60f,3.3f,-.19f,2.1f};
 case Kind::StingDagger:return {24,3.3f,4.7f,.62f,.75f,4.1f,.37f,1.7f};
 case Kind::SpiderDagger:return {22,2.9f,-1.5f,.89f,.67f,5.1f,-.35f,1.7f};
 case Kind::FlintDagger:return {21,4.7f,-.4f,.79f,.62f,2.5f,.03f,1.4f};
 case Kind::GlassDagger:return {24,3.4f,-1.8f,.91f,.76f,4.1f,-.20f,2.2f};
 case Kind::SporeDagger:return {24,4.7f,.1f,.87f,.61f,5.3f,.13f,2.3f};
 case Kind::SunDagger:return {24,3.9f,1.3f,.75f,.67f,4.6f,.32f,2.4f};
 default:return {37,4.2f,0,.73f,.77f,6.5f,.24f,2.05f};
 }
}

using Point=equipment_shape::Point;
// Coordinates are in the plane of the bit. Every head has its own cutting
// edge and poll. Negative x is the cutting side for carry and attack.
struct Axe {std::array<Point,10> bit;float haftEnd,butt,width;};
inline constexpr Axe axe(Kind k){
 switch(k){
 case Kind::TuskAxe:return {{{{0,.55f},{-.64f,.72f},{-1.43f,.38f},{-1.14f,.97f},{-1.41f,1.72f},{-.77f,1.58f},{-.16f,1.38f},{.35f,1.44f},{.49f,1.10f},{.29f,.69f}}},34,13,1.13f};
 case Kind::GuardianAxe:return {{{{0,.44f},{-.47f,.53f},{-1.25f,.26f},{-1.50f,.76f},{-1.40f,1.53f},{-.85f,1.76f},{-.26f,1.48f},{.67f,1.54f},{.74f,1.08f},{.43f,.62f}}},35,15,1.49f};
 case Kind::ScorpionAxe:return {{{{0,.49f},{-.63f,.62f},{-1.09f,.44f},{-1.37f,1.14f},{-1.50f,1.81f},{-.83f,1.51f},{-.20f,1.30f},{.57f,1.29f},{.62f,.97f},{.27f,.56f}}},33,12,1.14f};
 case Kind::ScarabAxe:return {{{{0,.37f},{-.58f,.35f},{-1.14f,.29f},{-1.52f,.81f},{-1.37f,1.55f},{-.78f,1.74f},{-.12f,1.47f},{.54f,1.58f},{.66f,.93f},{.39f,.54f}}},34,12,1.23f};
 case Kind::CopperHatchet:return {{{{0,.66f},{-.49f,.67f},{-1.13f,.56f},{-1.35f,.88f},{-1.25f,1.46f},{-.79f,1.57f},{-.23f,1.43f},{.54f,1.40f},{.57f,1.04f},{.31f,.69f}}},29,12,1.13f};
 case Kind::BasaltCleaver:return {{{{0,.40f},{-.49f,.48f},{-1.27f,.34f},{-1.43f,.75f},{-1.43f,1.47f},{-.92f,1.66f},{-.30f,1.54f},{.75f,1.43f},{.80f,.84f},{.43f,.49f}}},35,14,1.43f};
 case Kind::OathAxe:return {{{{0,.56f},{-.47f,.65f},{-1.19f,.28f},{-1.48f,.73f},{-1.51f,1.62f},{-.82f,1.94f},{-.18f,1.52f},{.65f,1.80f},{.56f,1.20f},{.34f,.68f}}},36,15,1.33f};
 case Kind::AshHookAxe:return {{{{0,.63f},{-.48f,.70f},{-1.20f,.29f},{-1.04f,.86f},{-1.51f,1.59f},{-.83f,1.86f},{-.16f,1.60f},{.49f,1.74f},{.54f,1.10f},{.28f,.70f}}},35,14,1.17f};
 default:return {equipment_shape::Axe,35,12,1.22f};
 }
}

struct Bow {float half,curve,recurve,width,shoulder,tip;};
inline constexpr Bow bow(Kind k){
 switch(k){
 case Kind::MoonBow:return {26.5f,10.6f,2.8f,1.68f,.91f,.56f};
 case Kind::BanditBow:return {23.8f,9.4f,.4f,1.45f,1.18f,.65f};
 case Kind::MirageBow:return {26.6f,11.9f,3.5f,1.44f,.85f,.54f};
 case Kind::GraveBow:return {25.4f,10.5f,4.3f,1.79f,1.25f,.79f};
 case Kind::GroveBow:return {26,12.7f,1.7f,1.77f,1.24f,.63f};
 case Kind::DawnBow:return {27,12.4f,2.9f,1.50f,.88f,.61f};
 case Kind::BirchBow:return {25.5f,9.5f,.6f,1.52f,1.13f,.48f};
 case Kind::OasisBow:return {24.8f,11.7f,3.1f,1.45f,1.19f,.46f};
 case Kind::AshBow:return {25.1f,10.5f,2.0f,1.52f,1.06f,.61f};
 case Kind::ReedBow:return {27,8.8f,3.6f,1.30f,1.05f,.42f};
 case Kind::CinderBow:return {26,12.7f,4.6f,1.64f,1.12f,.82f};
 case Kind::RiftBow:return {27,12.6f,4.2f,1.92f,1.32f,.72f};
 default:return {27,11,0,1.56f,1,.62f};
 }
}

inline constexpr float SpearLengthScale=1.15f;
inline constexpr float SpearHeel=-25.5f*SpearLengthScale,SpearSocket=34.3f*SpearLengthScale;
struct Spear {
 std::array<Point,7> point;
 float halfWidth,length,shaft,wrap;
 constexpr float bladeLength()const{return length*SpearLengthScale;}
 constexpr float tipOffset()const{return SpearSocket+point[3].y*bladeLength();}
};
inline constexpr Spear spear(Kind k){
 switch(k){
 case Kind::BriarSpear:return {{{{-.19f,0},{-.57f,.38f},{-.29f,.43f},{-.05f,1.10f},{.46f,.58f},{.61f,.25f},{.20f,0}}},4.7f,11.7f,1.19f,.86f};
 case Kind::OasisSpear:return {{{{-.19f,0},{-.70f,.26f},{-.37f,.40f},{0,1.17f},{.38f,.40f},{.69f,.24f},{.19f,0}}},5.2f,10.8f,1.08f,1.15f};
 case Kind::BasaltSpear:return {{{{-.25f,0},{-.66f,.38f},{-.41f,.72f},{-.11f,1.08f},{.44f,.67f},{.63f,.32f},{.21f,0}}},6.25f,10.5f,1.23f,1.01f};
 case Kind::DawnSpear:return {{{{-.20f,0},{-.74f,.31f},{-.37f,.51f},{0,1.25f},{.37f,.51f},{.74f,.31f},{.20f,0}}},5.3f,10.7f,1.13f,1.23f};
 default:return {{{{-.21f,0},{-.46f,.23f},{-.34f,.63f},{0,1.16f},{.33f,.63f},{.46f,.23f},{.21f,0}}},4.9f,10.8f,1.10f,1};
 }
}
}
