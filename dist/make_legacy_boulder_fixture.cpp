#include "../src/save_game.hpp"
int main(int argc,char**argv){
 if(argc!=2||std::string(argv[1])!="--no-audio")return 3;
 namespace fs=std::filesystem;
 fs::path root="art/gameplay/guardian/fixtures";fs::create_directories(root);
 ember::saves::Snapshot s;s.date="2026-10-23 12:00";
 s.game.player={8,19};s.game.hp=57;s.game.gold=123;s.game.bag[8]=ember::item(ember::Kind::GuardianSword,2);
 ember::saves::atomicWrite(root/"legacy-v1.efs",ember::saves::encode(s));
 return 0;
}
