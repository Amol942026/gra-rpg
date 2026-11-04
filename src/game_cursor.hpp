#pragma once

#include "native_cursor.hpp"
namespace game_cursor {
inline void initialize(){
 // Unlike EnableCursor, ShowCursor does not move the mouse to the window's
 // centre in raylib 5.5. DEFAULT uses the user's native Windows arrow.
 ShowCursor();SetMouseCursor(MOUSE_CURSOR_DEFAULT);
 native_cursor::install(GetWindowHandle());
}
inline void update(){if(IsCursorHidden())ShowCursor();native_cursor::refresh();}
inline void shutdown(){if(IsCursorHidden())ShowCursor();native_cursor::uninstall();}
}
