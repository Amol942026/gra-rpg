#pragma once
#include <array>
namespace ember {
// The first two IDs are retained for every existing save.
enum class Spell {None,Fireball,IceShard,LightningOrb,Count};
struct SpellDefinition {
    const char* name;const char* description;int price;float mana,cooldown,speed,range,radius,multiplier,impactLife;
};
inline constexpr std::array<SpellDefinition,3> Spells={{
    {"Kula ognia","Płomień wybucha przy kontakcie i parzy pobliskich wrogów.",600,10,.8f,8,8,1.05f,1,.35f},
    {"Sopel lodu","Szybki lodowy grot rozpryskuje się w ostrych kryształach.",1500,20,1.8f,11,9,.55f,1.7f,.55f},
    {"Kulisty piorun","Potężna kula burzy razi przeciwników wokół miejsca trafienia.",3000,35,3.5f,6.5f,9,1.5f,3,.7f}
}};
inline constexpr bool validSpell(Spell spell){return spell>=Spell::Fireball&&spell<Spell::Count;}
inline constexpr const SpellDefinition& spellDefinition(Spell spell){return Spells[validSpell(spell)?int(spell)-1:0];}
inline constexpr unsigned spellBit(Spell spell){return validSpell(spell)?1u<<(int(spell)-1):0;}
enum class LearnSpellResult {Success,Unavailable,AlreadyKnown,NotEnoughGold,Invalid};
inline const char* learnSpellMessage(LearnSpellResult result){switch(result){
    case LearnSpellResult::Success:return "Lekcja ukończona. Zaklęcie zapisano w twojej księdze.";
    case LearnSpellResult::Unavailable:return "Po pokonaniu Kamiennego Strażnika porozmawiaj z Eldrinem w wieży.";
    case LearnSpellResult::AlreadyKnown:return "Znasz już to zaklęcie. Eldrin nie pobierze złota ponownie.";
    case LearnSpellResult::NotEnoughGold:return "Brakuje ci złota na tę lekcję.";
    case LearnSpellResult::Invalid:return "Wybierz jedno z zaklęć Eldrina.";
}return "";}
}
