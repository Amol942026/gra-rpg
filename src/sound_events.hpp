#pragma once

namespace ember {
enum class Cue {
    Step, Swing, SlimeHit, WolfHit, BoneHit, StoneHit, EnemyFall, Hurt, Death,
    Roll, Dodge, Spell, Potion, Pickup, Equip, Drop, LevelUp, Victory,
    EnemyAttack, Ui, Reject, BowShot, Count
};
struct SoundEvent { Cue cue; float x,y; };
}
