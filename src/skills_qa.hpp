#pragma once
bool runSkillsQA(){
 fs::create_directories("qa-v27");std::ofstream report("qa-v27/skills-tests.txt");int checks=0,failures=0;
 auto check=[&](bool ok,const char* label){checks++;failures+=!ok;report<<(ok?"PASS: ":"FAIL: ")<<label<<'\n';report.flush();};
 auto reset=[](){game=Game{};game.props.clear();game.enemies.clear();game.forage.clear();game.ground.clear();game.player={15,15};game.messageTime=0;intro={};towerStory={};welcome=help=paused=audioOpen=statsOpen=skillsOpen=spellBookOpen=weaponWheelOpen=false;statDraft={};skillDraft={};draftForLevel=skillDraftLevel=-1;dragging={};mouse={0,0};};
 auto point=[](Rectangle r){mouse={r.x+r.width/2,r.y+r.height/2};};
 auto gain=[](){game.xp=game.level*80-1;Enemy victim{999,0,game.player,game.player,1,1};game.hit(victim,100);syncAdvancement();};
 reset();refreshDisplayLayout();check(!audioSystem.available,"skills diagnostic opens no audio device");
 game.hp=60;
 for(int i=0;i<SkillCount;i++){
  game.selectWeapon(i==1?1:0);point(skillActionRect(i));leftClickGameplay({});
  check(game.skillRank(Skill(i))==0&&game.hp==60&&game.mana==100&&game.swing==0&&!game.piercingArrowArmed&&game.healingPulse==0,"unlearned skill HUD clicks neither activate an effect nor consume resources");
  check(!activateGameplaySkill(Skill(i))&&game.hp==60&&game.mana==100,"the same hotkey handler refuses an unlearned skill");
 }
 game.selectWeapon(0);game.hp=game.maxHp();mouse={0,0};renderScene();saveTexture(canvas,"qa-v27/locked-starting-skills.png");
 gain();check(statsOpen&&!skillsOpen&&game.attributePoints==5&&game.skillPoints==1,"real level opens attributes first and earns one skill point");
 for(int i:{0,0,0,0,5}){point(statAdjust(i,true));clickProgression();}
 renderScene();saveTexture(canvas,"qa-v27/attributes-before-skills.png");point(statsConfirm());clickProgression();
 check(!statsOpen&&skillsOpen&&game.skillAllocationPending()&&game.skillRanks==SkillRanks{},"stats confirmation automatically opens mandatory skill selection without training early");
 float time=game.time;V pos=game.player;game.update(.05f,{1,0},true);
 check(game.time==time&&distance(pos,game.player)==0&&!gameplayInputAllowed()&&!worldPointerAllowed({600,430}),"skill allocation freezes simulation and mouse gameplay");
 check(!confirmSkills()&&skillsOpen&&game.skillPoints==1,"unallocated skill point cannot be skipped by confirmation");
 point(skillCard(0));clickProgression();check(skillDraft[0]==1&&game.skillRank(Skill::StrongCut)==0,"card selection changes draft only");
 point(skillCard(1));clickProgression();check(skillDraft==SkillRanks{0,1},"single-point selection can switch to other card before confirmation");
 point(skillAdjust(1,false));clickProgression();check(skillDraftRemaining()==1,"minus restores pending skill point");
 point(skillCard(0));clickProgression();
 for(float scale:{.8f,1.f,1.2f}){prepareCanvas(scale);mouse={0,0};renderScene();saveTexture(canvas,TextFormat("qa-v27/skills-menu-%d.png",int(scale*100)));}
 point(skillsConfirm());clickProgression();check(!skillsOpen&&!game.allocationRequired&&game.skillRank(Skill::StrongCut)==1&&game.skillPoints==0,"confirmation trains exactly chosen skill and resumes gameplay");
 game.effects.clear();game.messageTime=0;V foe=game.player+V{1,0};game.enemies.push_back({100,0,foe,foe,500,500});game.enemies.back().windup=100;float mana=game.mana,damage=game.strongCutDamage();
 point(skillActionRect(0));leftClickGameplay({});
 check(game.strongSwingActive()&&game.mana==mana-10&&std::abs(game.enemies[0].hp-(500-damageAfterArmor(damage,game.enemies[0].defense())))<.001f,"HUD strong-cut click uses learned sword skill for ten mana and greater damage");
 for(int i=0;i<3;i++)game.update(.05f,{},false);mouse={0,0};renderScene();saveTexture(canvas,"qa-v27/strong-cut.png");
 game.selectWeapon(1);mana=game.mana;point(skillActionRect(0));leftClickGameplay({});check(game.mana==mana&&!activateGameplaySkill(Skill::StrongCut),"sword skill is blocked when bow is equipped and cannot spend mana");
 game.selectWeapon(0);gain();for(int i=0;i<5;i++){point(statAdjust(5,true));clickProgression();}point(statsConfirm());clickProgression();point(skillCard(1));clickProgression();point(skillsConfirm());clickProgression();
 check(game.skillRanks==SkillRanks{1,1}&&!game.allocationRequired,"next level trains second skill through both mandatory menus");
 check(!activateGameplaySkill(Skill::PiercingArrow),"arrow skill cannot activate with sword equipped");game.selectWeapon(1);for(int i=0;i<13;i++)game.update(.05f,{},false);
 game.enemies.clear();for(int i=0;i<3;i++){V p=game.player+V{3.f+i*2,0};game.enemies.push_back({101+i,0,p,p,500,500});game.enemies.back().windup=100;}
 mana=game.mana;int ammo=game.ammoCount();point(skillActionRect(1));leftClickGameplay({});
 check(game.piercingArrowArmed&&game.mana==mana&&game.ammoCount()==ammo,"HUD piercing click arms shot without early mana or arrow cost");
 Vector2 aim=screen(game.enemies[0].pos);mouse={aim.x,aim.y-18};leftClickGameplay({});
 check(game.bowDrawing&&game.piercingArrowArmed,"normal LPM world input charges prepared piercing shot");
 for(int i=0;i<42;i++){game.aimBow(combatAimFromPointer());game.update(1.f/60,{},false);}
 renderScene();saveTexture(canvas,"qa-v27/piercing-draw.png");float expected=game.arrowDamage(game.bowCharge());
 check(releaseWorldBow()&&game.arrows.size()==1&&game.arrows[0].piercing&&game.arrows[0].damage==expected&&game.mana==mana-20&&game.ammoCount()==ammo-1,"release spends twenty mana and one arrow while retaining normal charged-shot damage");
 game.updateArrows(.04f);renderScene();saveTexture(canvas,"qa-v27/piercing-flight.png");game.updateArrows(.6f);
 bool all=true;for(const auto& e:game.enemies)all&=std::abs(e.hp-(500-damageAfterArmor(expected,e.defense())))<.001f;
 check(all&&game.pierceFlashes.size()==3,"one piercing shot damages three aligned enemies exactly once and animates each hit");
 mouse={0,0};renderScene();saveTexture(canvas,"qa-v27/piercing-impacts.png");
 game.piercingArrowCd=game.attackCd=0;game.preparePiercingArrow();mouse={aim.x,aim.y-18};leftClickGameplay({});openStats();
 check(!game.piercingArrowArmed&&!game.bowDrawing&&game.mana==mana-20&&game.ammoCount()==ammo-1,"opening character sheet cancels armed draw with no extra resource cost");confirmStats();
 game.preparePiercingArrow();mouse={aim.x,aim.y-18};leftClickGameplay({});point(bagRect(0));
 check(!releaseWorldBow()&&!game.piercingArrowArmed&&game.mana==mana-20&&game.ammoCount()==ammo-1,"release over inventory cancels piercing shot without resource cost");
 point(skillsButton());clickProgression();check(skillsOpen&&!game.allocationRequired,"HUD opens learned skill sheet outside advancement");confirmSkills();
 RenderTexture2D sample=LoadRenderTexture(280,260);RenderTexture2D sheet=LoadRenderTexture(1300,720);
 BeginTextureMode(sheet);ClearBackground(Ink);txt("MOCNE CIĘCIE — OSIEM KIERUNKÓW",24,16,24,Gold);EndTextureMode();
 for(int row=0;row<3;row++)for(int d=0;d<8;d++){
  float progress=.18f+row*.3f;AnimationState pose;pose.direction=static_cast<Direction>(d);pose.idleTime=.7f;
  BeginTextureMode(sample);ClearBackground(BLANK);actor_art::draw(0,140,220,1,pose,std::sin(progress*PI),false,false,Kind::Sword,false,BLANK,nullptr,-1,progress,-1,3,true);EndTextureMode();
  Image im=LoadImageFromTexture(sample.texture);ImageFlipVertical(&im);Color* pixels=LoadImageColors(im);int opaque=0,clipped=0,mismatch=0,interior=0;
  for(int y=0;y<260;y++)for(int x=0;x<280;x++)if(pixels[y*280+x].a>230){opaque++;clipped+=x<2||x>=278||y<2||y>=258;}
  for(int y=8;y<252;y+=8)for(int x=8;x<272;x+=8){bool solid=true,empty=true;for(int dy:{-3,0,3})for(int dx:{-3,0,3}){int a=pixels[(y+dy)*280+x+dx].a;solid&=a>230;empty&=a<8;}
   if(!solid&&!empty)continue;Vector2 point{float(x)+.5f,float(y)+.5f};bool hit=actor_art::draw(0,140,220,1,pose,std::sin(progress*PI),false,false,Kind::Sword,false,BLANK,&point,-1,progress,-1,3,true);interior+=solid;mismatch+=solid?!hit:hit;
  }
  check(opaque>130&&clipped==0,"strong-cut animation fits full silhouette in each phase and direction");
  check(interior>2&&mismatch==0,"empowered sword GPU silhouette agrees with pointer geometry");
  UnloadImageColors(pixels);UnloadImage(im);
  BeginTextureMode(sheet);actor_art::draw(0,145+d*144.f,210+row*210.f,1,pose,std::sin(progress*PI),false,false,Kind::Sword,false,BLANK,nullptr,-1,progress,-1,3,true);EndTextureMode();
 }
 saveTexture(sheet,"qa-v27/strong-cut-8-directions.png");UnloadRenderTexture(sheet);
 RenderTexture2D bowSheet=LoadRenderTexture(1300,270);BeginTextureMode(bowSheet);ClearBackground(Ink);txt("PRZESZYWAJĄCA STRZAŁA — NACIĄG W OŚMIU KIERUNKACH",24,16,24,Blue);EndTextureMode();
 for(int d=0;d<8;d++){
  AnimationState pose;pose.direction=static_cast<Direction>(d);pose.idleTime=.7f;
  BeginTextureMode(sample);ClearBackground(BLANK);actor_art::draw(0,140,220,1,pose,0,false,false,Kind::Bow,false,BLANK,nullptr,-1,-1,1,3,true);EndTextureMode();
  Image im=LoadImageFromTexture(sample.texture);ImageFlipVertical(&im);Color* pixels=LoadImageColors(im);int opaque=0,clipped=0,mismatch=0,interior=0;
  for(int y=0;y<260;y++)for(int x=0;x<280;x++)if(pixels[y*280+x].a>230){opaque++;clipped+=x<2||x>=278||y<2||y>=258;}
  for(int y=8;y<252;y+=8)for(int x=8;x<272;x+=8){bool solid=true,empty=true;for(int dy:{-3,0,3})for(int dx:{-3,0,3}){int a=pixels[(y+dy)*280+x+dx].a;solid&=a>230;empty&=a<8;}
   if(!solid&&!empty)continue;Vector2 point{float(x)+.5f,float(y)+.5f};bool hit=actor_art::draw(0,140,220,1,pose,0,false,false,Kind::Bow,false,BLANK,&point,-1,-1,1,3,true);interior+=solid;mismatch+=solid?!hit:hit;
  }
  check(opaque>130&&clipped==0,"empowered bow and quiver fit silhouette in all eight directions");check(interior>2&&mismatch==0,"empowered bow GPU silhouette agrees with pointer geometry");
  UnloadImageColors(pixels);UnloadImage(im);BeginTextureMode(bowSheet);actor_art::draw(0,145+d*144.f,220,1,pose,0,false,false,Kind::Bow,false,BLANK,nullptr,-1,-1,1,3,true);EndTextureMode();
 }
 saveTexture(bowSheet,"qa-v27/piercing-bow-8-directions.png");UnloadRenderTexture(bowSheet);UnloadRenderTexture(sample);
 reset();refreshDisplayLayout();report<<"Checks: "<<checks<<"; failures: "<<failures<<'\n';return failures==0;
}
