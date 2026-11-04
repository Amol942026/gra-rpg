// Compiled beside the existing Win32 presentation integration. No rendered
// pointer, cursor capture/warp, system preference change or global ShowCursor
// loop; the display-count probe below stays balanced when already visible.
#include "native_cursor.hpp"
#include <cstring>
namespace native_cursor {
namespace detail {
HWND window=nullptr;
WNDPROC previous=nullptr;
HCURSOR arrow=nullptr;
LONG_PTR previousClassCursor=0;
void showArrow(){
 if(!arrow)arrow=LoadCursorW(nullptr,MAKEINTRESOURCEW(32512)); // IDC_ARROW, user's Windows scheme
 SetCursor(arrow);
 // raylib also exports a C symbol named ShowCursor. Resolve the actual
 // user32 entry explicitly rather than linking its colliding import thunk.
 using AdjustCount=int (WINAPI*)(BOOL);
 static AdjustCount adjust=[](){
  FARPROC proc=GetProcAddress(GetModuleHandleW(L"user32.dll"),"ShowCursor");AdjustCount result=nullptr;
  static_assert(sizeof(result)==sizeof(proc),"Windows function pointer size");
  std::memcpy(&result,&proc,sizeof(result));return result;
 }();
 if(!adjust)return;
 int count=adjust(TRUE);
 if(count>0)adjust(FALSE);
 else for(int tries=0;count<0&&tries<63;tries++)count=adjust(TRUE);
}
bool overClient(){
 if(!window||!IsWindowVisible(window)||IsIconic(window)||GetForegroundWindow()!=window)return false;
 POINT p{};RECT client{};
 if(!GetCursorPos(&p)||WindowFromPoint(p)!=window||!GetClientRect(window,&client)||!ScreenToClient(window,&p))return false;
 return PtInRect(&client,p)!=FALSE;
}
LRESULT CALLBACK procedure(HWND hwnd,UINT message,WPARAM wp,LPARAM lp){
 if(message==WM_SETCURSOR&&LOWORD(lp)==HTCLIENT){showArrow();return TRUE;}
 LRESULT result=CallWindowProcW(previous,hwnd,message,wp,lp);
 if(message==WM_SETFOCUS||message==WM_ACTIVATE||message==WM_SIZE||message==WM_STYLECHANGED||message==WM_DISPLAYCHANGE){
  if(overClient())showArrow();
 }
 return result;
}
}
void uninstall(){
 using namespace detail;
 if(window&&IsWindow(window)){
  if(reinterpret_cast<WNDPROC>(GetWindowLongPtrW(window,GWLP_WNDPROC))==procedure)SetWindowLongPtrW(window,GWLP_WNDPROC,reinterpret_cast<LONG_PTR>(previous));
  SetClassLongPtrW(window,GCLP_HCURSOR,previousClassCursor);
 }
 window=nullptr;previous=nullptr;arrow=nullptr;previousClassCursor=0;
}
void install(void* handle){
 using namespace detail;
 HWND target=static_cast<HWND>(handle);if(window==target)return;
 uninstall();if(!target||!IsWindow(target))return;
 arrow=LoadCursorW(nullptr,MAKEINTRESOURCEW(32512));
 WNDPROC old=reinterpret_cast<WNDPROC>(GetWindowLongPtrW(target,GWLP_WNDPROC));
 SetLastError(0);LONG_PTR prior=SetWindowLongPtrW(target,GWLP_WNDPROC,reinterpret_cast<LONG_PTR>(procedure));
 if(!prior&&GetLastError()!=0){arrow=nullptr;return;}
 previous=prior?reinterpret_cast<WNDPROC>(prior):old;window=target;
 previousClassCursor=SetClassLongPtrW(target,GCLP_HCURSOR,reinterpret_cast<LONG_PTR>(arrow));
 refresh();
}
void refresh(){if(detail::overClient())detail::showArrow();}
}
