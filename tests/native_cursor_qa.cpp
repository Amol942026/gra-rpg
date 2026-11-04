#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <fstream>
#include <string>
#include <filesystem>
#include "../src/native_cursor_win32.inl"
namespace fs=std::filesystem;
int originalMessages=0;
LRESULT CALLBACK fixture(HWND w,UINT m,WPARAM p,LPARAM l){
 if(m==WM_SETCURSOR){originalMessages++;SetCursor(LOWORD(l)==HTCAPTION?LoadCursorW(nullptr,MAKEINTRESOURCEW(32649)):nullptr);return TRUE;}
 return DefWindowProcW(w,m,p,l);
}
int cursorCounter(){int n=ShowCursor(TRUE);ShowCursor(FALSE);return n-1;}
std::wstring desktopName(HDESK desktop){wchar_t name[256]{};DWORD needed=0;GetUserObjectInformationW(desktop,UOI_NAME,name,sizeof(name),&needed);return name;}
int main(int argc,char** argv){
 if(argc!=4||std::string(argv[1])!="--no-audio"||std::string(argv[3])!="--background-test")return 3;
 fs::path root=argv[2];fs::create_directories(root);std::ofstream log(root/"native-checks.txt");int checks=0,failures=0;
 auto check=[&](bool b,const char* label){checks++;failures+=!b;log<<(b?"PASS ":"FAIL ")<<label<<'\n';log.flush();};
 // An isolated, non-input desktop protects the user's real cursor while we
 // deliberately exercise NULL handles and negative Win32 display counters.
 HDESK previous=GetThreadDesktop(GetCurrentThreadId());std::wstring name=L"EmberfallCursorQA_"+std::to_wstring(GetCurrentProcessId());
 HDESK isolated=CreateDesktopW(name.c_str(),nullptr,nullptr,0,GENERIC_ALL,nullptr);
 if(!isolated||!SetThreadDesktop(isolated)){check(false,"private desktop creation");return 4;}
 HDESK input=OpenInputDesktop(0,FALSE,DESKTOP_READOBJECTS);check(input&&desktopName(input)!=desktopName(GetThreadDesktop(GetCurrentThreadId())),"native failure probes run away from the user's input desktop");if(input)CloseDesktop(input);
 WNDCLASSW cls{};cls.lpfnWndProc=fixture;cls.hInstance=GetModuleHandleW(nullptr);cls.lpszClassName=L"EmberfallCursorFixture";RegisterClassW(&cls);
 HWND window=CreateWindowW(cls.lpszClassName,L"Hidden native cursor fixture",WS_POPUP,0,0,200,150,nullptr,nullptr,cls.hInstance,nullptr);
 check(window&&!IsWindowVisible(window),"fixture window remains hidden on the private desktop");
 for(int i=0;i<64&&cursorCounter()!=0;i++){if(cursorCounter()<0)ShowCursor(TRUE);else ShowCursor(FALSE);}
 HCURSOR systemArrow=LoadCursorW(nullptr,MAKEINTRESOURCEW(32512));
 SendMessageW(window,WM_SETCURSOR,reinterpret_cast<WPARAM>(window),MAKELPARAM(HTCLIENT,WM_MOUSEMOVE));
 check(GetCursor()==nullptr,"fixture reproduces the missing native HCURSOR");
 native_cursor::install(window);
 check(reinterpret_cast<HCURSOR>(GetClassLongPtrW(window,GCLP_HCURSOR))==systemArrow,"fallback class cursor is the user's Windows IDC_ARROW");
 SendMessageW(window,WM_SETCURSOR,reinterpret_cast<WPARAM>(window),MAKELPARAM(HTCLIENT,WM_MOUSEMOVE));
 check(GetCursor()==systemArrow&&cursorCounter()==0,"client cursor event repairs NULL with a visible native Windows arrow");
 ICONINFO icon{};bool nativeIcon=GetIconInfo(GetCursor(),&icon)!=FALSE;
 check(nativeIcon&&!icon.fIcon&&icon.hbmMask,"selected handle is a real Windows cursor, not game-rendered graphics");if(icon.hbmMask)DeleteObject(icon.hbmMask);if(icon.hbmColor)DeleteObject(icon.hbmColor);
 ShowCursor(FALSE);ShowCursor(FALSE);ShowCursor(FALSE);check(cursorCounter()==-3,"negative Win32 visibility counter is reproduced in isolation");
 SendMessageW(window,WM_SETCURSOR,reinterpret_cast<WPARAM>(window),MAKELPARAM(HTCLIENT,WM_MOUSEMOVE));
 check(cursorCounter()==0&&GetCursor()==systemArrow,"native cursor repair restores a negative visibility counter");
 int count=cursorCounter();for(int i=0;i<500;i++)SendMessageW(window,WM_SETCURSOR,reinterpret_cast<WPARAM>(window),MAKELPARAM(HTCLIENT,WM_MOUSEMOVE));
 check(cursorCounter()==count,"500 repeated updates do not inflate the Windows display counter");
 for(UINT event:{WM_SETFOCUS,WM_ACTIVATE,WM_SIZE,WM_DISPLAYCHANGE}){
  SetCursor(nullptr);native_cursor::refresh();SendMessageW(window,event,0,0);
  check(GetCursor()==nullptr&&!IsWindowVisible(window),"hidden or unfocused window never takes ownership of the desktop cursor");
  SendMessageW(window,WM_SETCURSOR,reinterpret_cast<WPARAM>(window),MAKELPARAM(HTCLIENT,WM_MOUSEMOVE));check(GetCursor()==systemArrow,"next client event restores the native arrow after window state change");
 }
 int forwarded=originalMessages;SendMessageW(window,WM_SETCURSOR,reinterpret_cast<WPARAM>(window),MAKELPARAM(HTCAPTION,WM_MOUSEMOVE));
 check(originalMessages==forwarded+1&&GetCursor()!=systemArrow,"non-client cursor handling stays with the original window procedure");
 native_cursor::install(window);native_cursor::uninstall();
 check(reinterpret_cast<WNDPROC>(GetWindowLongPtrW(window,GWLP_WNDPROC))==fixture&&GetClassLongPtrW(window,GCLP_HCURSOR)==0,"shutdown restores the original procedure and class cursor after repeated install");
 native_cursor::install(window);SendMessageW(window,WM_SETCURSOR,reinterpret_cast<WPARAM>(window),MAKELPARAM(HTCLIENT,WM_MOUSEMOVE));check(GetCursor()==systemArrow,"native handling can be installed again without stale state");native_cursor::uninstall();
 DestroyWindow(window);UnregisterClassW(cls.lpszClassName,cls.hInstance);check(SetThreadDesktop(previous)!=FALSE,"original test-thread desktop restored without switching the user's input desktop");CloseDesktop(isolated);
 std::ofstream out(root/"native-summary.json");out<<"{\"checks\":"<<checks<<",\"failures\":"<<failures<<",\"native_windows_cursor\":true,\"private_desktop\":true,\"hidden\":true,\"audio_opened\":false}";
 return failures?2:0;
}
