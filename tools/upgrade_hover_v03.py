from pathlib import Path

p=Path('src/main.cpp')
s=p.read_text(encoding='utf-8')
s=s.replace('else if(mouse.x<WorldW&&mouse.y>117&&mouse.y<741){if(!game.drop', 'else if(worldPointerAllowed(mouse)){if(!game.drop')
s=s.replace('txt("STRZAŁKI",','txt("WSAD",')
s=s.replace('const char* keys[]={"STRZAŁKI",','const char* keys[]={"W S A D",')
s=s.replace('"Ruszaj się w kierunkach ekranu."','"W — góra, S — dół, A — lewo, D — prawo."')
for old,new in [('KEY_RIGHT','KEY_D'),('KEY_LEFT','KEY_A'),('KEY_DOWN','KEY_S'),('KEY_UP','KEY_W')]:
    s=s.replace(old,new)
s=s.replace('txt("v0.2",','txt("v0.3",').replace('ruin | v0.2','ruin | v0.3')
start=s.index(' struct Obj {float depth;int type,index;};')
end=s.index('\n for(auto obj:objects)',start)
s=s[:start]+' auto objects=worldDrawOrder();'+s[end:]
needle='auto&t=propArt[p.type].texture;DrawTexturePro(t,'
assert needle in s
s=s.replace(needle,'auto&t=propArt[p.type].texture;if(hovered.kind==HoverKind::Prop&&hovered.index==obj.index)DrawTexturePro(propOutlines[p.type],{0,0,180,230},{q.x-90*p.size,q.y-208*p.size,180*p.size,230*p.size},{0,0},0,Gold);DrawTexturePro(t,')
needle='float pulse=.55f+std::sin(game.time*3+g.id)*.2f;'
assert needle in s
s=s.replace(needle,needle+'if(hovered.kind==HoverKind::Ground&&hovered.index==g.id){DrawEllipseLines(int(p.x),int(p.y),21,10,Gold);diamond(p.x,p.y,20,10,Fade(Gold,.25f));}')
start=s.index('   else if(mouse.x<WorldW&&mouse.y>117&&mouse.y<740){float best=55;')
end=s.index('\n',start)
s=s[:start]+'''   else if(worldPointerAllowed(mouse)){WorldHover hit=pickWorld(mouse);game.target=hit.kind==HoverKind::Enemy?hit.index:-1;if(game.target>=0)game.attack();}'''+s[end:]
# Keep the ground-pick regression test on a visible item, not underneath the player.
s=s.replace(' Vector2 start=screen(game.ground[0].pos);',' game.ground[0].pos=game.player+V{-1,.8f};\n Vector2 start=screen(game.ground[0].pos);')
s=s.replace('    makeArt();','    makeArt();initHoverArt();')
s=s.replace('    for(auto& t:propArt)UnloadRenderTexture(t);','    unloadHoverArt();\n    for(auto& t:propArt)UnloadRenderTexture(t);')
for name in ['interaction-test','benchmark','smoke-test','emberfall']:
    s=s.replace(name+'-v02',name+'-v03')
p.write_text(s,encoding='utf-8')

p=Path('src/visual_qa.hpp');s=p.read_text(encoding='utf-8')
s=s.replace('    drawWorld();drawUI();tooltip();','    hovered=pickWorld(mouse);\n    drawWorld();drawUI();drawHoveredName();tooltip();')
p.write_text(s,encoding='utf-8')

p=Path('resources/Emberfall.manifest');s=p.read_text(encoding='utf-8')
p.write_text(s.replace('version="0.2.0.0"','version="0.3.0.0"'),encoding='utf-8')
