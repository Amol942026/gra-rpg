#include "sync_present.hpp"
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/wglext.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
extern "C" void (*glfwGetProcAddress(const char*))(void);
extern "C" int glfwExtensionSupported(const char*);
extern "C" void glfwSwapInterval(int);
struct GLFWwindow;
extern "C" void __real_glfwSwapBuffers(GLFWwindow*);

namespace sync_present {
namespace {
struct Presenter {
    ID3D11Device* device=nullptr;
    ID3D11DeviceContext* context=nullptr;
    IDXGISwapChain1* chain=nullptr;
    ID3D11Texture2D* back=nullptr;
    ID3D11Texture2D* shared=nullptr;
    HANDLE bridge=nullptr,object=nullptr,frameReady=nullptr;
    HWND hiddenWindow=nullptr;
    HDC originalDc=nullptr,hiddenDc=nullptr;
    HGLRC originalContext=nullptr;
    GLuint renderbuffer=0,fbo=0;
    int width=0,height=0,interval=0;
    bool ready=false,disabled=false,locked=false;
    unsigned int frames=0,swaps=0;
    char reason[160]{};
    PFNWGLDXOPENDEVICENVPROC openDevice=nullptr;
    PFNWGLDXCLOSEDEVICENVPROC closeDevice=nullptr;
    PFNWGLDXREGISTEROBJECTNVPROC registerObject=nullptr;
    PFNWGLDXUNREGISTEROBJECTNVPROC unregisterObject=nullptr;
    PFNWGLDXLOCKOBJECTSNVPROC lockObjects=nullptr;
    PFNWGLDXUNLOCKOBJECTSNVPROC unlockObjects=nullptr;
    PFNGLGENFRAMEBUFFERSPROC genFramebuffers=nullptr;
    PFNGLDELETEFRAMEBUFFERSPROC deleteFramebuffers=nullptr;
    PFNGLBINDFRAMEBUFFERPROC bindFramebuffer=nullptr;
    PFNGLGENRENDERBUFFERSPROC genRenderbuffers=nullptr;
    PFNGLDELETERENDERBUFFERSPROC deleteRenderbuffers=nullptr;
    PFNGLFRAMEBUFFERRENDERBUFFERPROC attachRenderbuffer=nullptr;
    PFNGLCHECKFRAMEBUFFERSTATUSPROC checkFramebuffer=nullptr;
} state;
template<class T> void release(T*& value){if(value){value->Release();value=nullptr;}}
template<class T> T load(const char* name){return reinterpret_cast<T>(glfwGetProcAddress(name));}
void restoreDrawable(){
    if(state.originalDc&&state.originalContext)wglMakeCurrent(state.originalDc,state.originalContext);
    if(state.hiddenDc){ReleaseDC(state.hiddenWindow,state.hiddenDc);state.hiddenDc=nullptr;}
    if(state.hiddenWindow){DestroyWindow(state.hiddenWindow);state.hiddenWindow=nullptr;}
    state.originalDc=nullptr;state.originalContext=nullptr;
}
bool offscreenDrawable(HWND owner){
    state.originalDc=wglGetCurrentDC();state.originalContext=wglGetCurrentContext();
    WNDCLASSW windowClass{};windowClass.style=CS_OWNDC;windowClass.lpfnWndProc=DefWindowProcW;
    windowClass.hInstance=GetModuleHandleW(nullptr);windowClass.lpszClassName=L"EmberfallOffscreenRenderer";
    if(!RegisterClassW(&windowClass)&&GetLastError()!=ERROR_CLASS_ALREADY_EXISTS)return false;
    state.hiddenWindow=CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE,windowClass.lpszClassName,L"",WS_POPUP,
                                     0,0,1,1,owner,nullptr,windowClass.hInstance,nullptr);
    if(!state.hiddenWindow)return false;
    state.hiddenDc=GetDC(state.hiddenWindow);PIXELFORMATDESCRIPTOR format{};int index=GetPixelFormat(state.originalDc);
    if(!state.hiddenDc||!DescribePixelFormat(state.originalDc,index,sizeof(format),&format)||!SetPixelFormat(state.hiddenDc,index,&format))return false;
    // Use the same GL context (and all existing textures), but a hidden
    // drawable. The visible HWND must have exactly one presentation owner.
    return wglMakeCurrent(state.hiddenDc,state.originalContext)!=FALSE;
}
void dropSurface(){
    if(state.locked&&state.unlockObjects){state.unlockObjects(state.bridge,1,&state.object);state.locked=false;}
    if(state.object){state.unregisterObject(state.bridge,state.object);state.object=nullptr;}
    if(state.fbo){state.deleteFramebuffers(1,&state.fbo);state.fbo=0;}
    if(state.renderbuffer){state.deleteRenderbuffers(1,&state.renderbuffer);state.renderbuffer=0;}
    release(state.back);release(state.shared);
}
bool fail(const char* operation,HRESULT result=E_FAIL){
    std::snprintf(state.reason,sizeof(state.reason),"%s (0x%08lx)",operation,static_cast<unsigned long>(result));
    state.ready=false;state.disabled=true;glfwSwapInterval(state.interval>0?state.interval:1);dropSurface();
    if(state.bridge){state.closeDevice(state.bridge);state.bridge=nullptr;}
    if(state.frameReady){CloseHandle(state.frameReady);state.frameReady=nullptr;}
    release(state.chain);release(state.context);release(state.device);restoreDrawable();return false;
}
bool surface(int width,int height){
    dropSurface();
    HRESULT result=state.chain->ResizeBuffers(2,width,height,DXGI_FORMAT_R8G8B8A8_UNORM,DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
    if(FAILED(result))return fail("ResizeBuffers",result);
    result=state.chain->GetBuffer(0,__uuidof(ID3D11Texture2D),reinterpret_cast<void**>(&state.back));
    if(FAILED(result))return fail("GetBuffer",result);
    D3D11_TEXTURE2D_DESC description{};
    description.Width=width;description.Height=height;description.MipLevels=1;description.ArraySize=1;
    description.Format=DXGI_FORMAT_R8G8B8A8_UNORM;description.SampleDesc.Count=1;
    description.Usage=D3D11_USAGE_DEFAULT;description.BindFlags=D3D11_BIND_RENDER_TARGET;
    result=state.device->CreateTexture2D(&description,nullptr,&state.shared);
    if(FAILED(result))return fail("CreateTexture2D",result);
    state.genRenderbuffers(1,&state.renderbuffer);
    state.object=state.registerObject(state.bridge,state.shared,state.renderbuffer,GL_RENDERBUFFER,WGL_ACCESS_READ_WRITE_NV);
    if(!state.object)return fail("wglDXRegisterObjectNV",HRESULT_FROM_WIN32(GetLastError()));
    if(!state.lockObjects(state.bridge,1,&state.object))return fail("wglDXLockObjectsNV",HRESULT_FROM_WIN32(GetLastError()));
    state.locked=true;state.genFramebuffers(1,&state.fbo);state.bindFramebuffer(GL_FRAMEBUFFER,state.fbo);
    state.attachRenderbuffer(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_RENDERBUFFER,state.renderbuffer);
    GLenum status=state.checkFramebuffer(GL_FRAMEBUFFER);state.bindFramebuffer(GL_FRAMEBUFFER,0);
    state.unlockObjects(state.bridge,1,&state.object);state.locked=false;
    if(status!=GL_FRAMEBUFFER_COMPLETE)return fail("Shared framebuffer is incomplete");
    state.width=width;state.height=height;return true;
}
bool initialize(HWND window,int width,int height){
    if(!glfwExtensionSupported("WGL_NV_DX_interop2"))return fail("GPU sharing extension is unavailable");
    state.openDevice=load<PFNWGLDXOPENDEVICENVPROC>("wglDXOpenDeviceNV");
    state.closeDevice=load<PFNWGLDXCLOSEDEVICENVPROC>("wglDXCloseDeviceNV");
    state.registerObject=load<PFNWGLDXREGISTEROBJECTNVPROC>("wglDXRegisterObjectNV");
    state.unregisterObject=load<PFNWGLDXUNREGISTEROBJECTNVPROC>("wglDXUnregisterObjectNV");
    state.lockObjects=load<PFNWGLDXLOCKOBJECTSNVPROC>("wglDXLockObjectsNV");
    state.unlockObjects=load<PFNWGLDXUNLOCKOBJECTSNVPROC>("wglDXUnlockObjectsNV");
    state.genFramebuffers=load<PFNGLGENFRAMEBUFFERSPROC>("glGenFramebuffers");
    state.deleteFramebuffers=load<PFNGLDELETEFRAMEBUFFERSPROC>("glDeleteFramebuffers");
    state.bindFramebuffer=load<PFNGLBINDFRAMEBUFFERPROC>("glBindFramebuffer");
    state.genRenderbuffers=load<PFNGLGENRENDERBUFFERSPROC>("glGenRenderbuffers");
    state.deleteRenderbuffers=load<PFNGLDELETERENDERBUFFERSPROC>("glDeleteRenderbuffers");
    state.attachRenderbuffer=load<PFNGLFRAMEBUFFERRENDERBUFFERPROC>("glFramebufferRenderbuffer");
    state.checkFramebuffer=load<PFNGLCHECKFRAMEBUFFERSTATUSPROC>("glCheckFramebufferStatus");
    if(!state.openDevice||!state.closeDevice||!state.registerObject||!state.unregisterObject||!state.lockObjects||!state.unlockObjects
       ||!state.genFramebuffers||!state.deleteFramebuffers||!state.bindFramebuffer||!state.genRenderbuffers||!state.deleteRenderbuffers
       ||!state.attachRenderbuffer||!state.checkFramebuffer)return fail("GPU sharing entry points are unavailable");
    if(!offscreenDrawable(window))return fail("Offscreen rendering drawable",HRESULT_FROM_WIN32(GetLastError()));
    HRESULT result=D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_HARDWARE,nullptr,D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                                   nullptr,0,D3D11_SDK_VERSION,&state.device,nullptr,&state.context);
    if(FAILED(result))return fail("D3D11CreateDevice",result);
    IDXGIDevice1* dxDevice=nullptr;IDXGIAdapter* adapter=nullptr;IDXGIFactory2* factory=nullptr;
    result=state.device->QueryInterface(__uuidof(IDXGIDevice1),reinterpret_cast<void**>(&dxDevice));
    if(SUCCEEDED(result)){dxDevice->SetMaximumFrameLatency(1);result=dxDevice->GetAdapter(&adapter);}
    if(SUCCEEDED(result))result=adapter->GetParent(__uuidof(IDXGIFactory2),reinterpret_cast<void**>(&factory));
    release(adapter);release(dxDevice);
    if(FAILED(result)){release(factory);return fail("DXGI factory",result);}
    DXGI_SWAP_CHAIN_DESC1 description{};
    description.Width=width;description.Height=height;description.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
    description.SampleDesc.Count=1;description.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;
    description.BufferCount=2;description.Scaling=DXGI_SCALING_STRETCH;
    description.Flags=DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
    description.SwapEffect=DXGI_SWAP_EFFECT_FLIP_DISCARD;description.AlphaMode=DXGI_ALPHA_MODE_IGNORE;
    result=factory->CreateSwapChainForHwnd(state.device,window,&description,nullptr,nullptr,&state.chain);
    if(FAILED(result)){
        description.SwapEffect=DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        result=factory->CreateSwapChainForHwnd(state.device,window,&description,nullptr,nullptr,&state.chain);
    }
    factory->MakeWindowAssociation(window,DXGI_MWA_NO_ALT_ENTER);release(factory);
    if(FAILED(result))return fail("CreateSwapChainForHwnd",result);
    IDXGISwapChain2* latency=nullptr;
    result=state.chain->QueryInterface(__uuidof(IDXGISwapChain2),reinterpret_cast<void**>(&latency));
    if(FAILED(result))return fail("Swap chain latency interface",result);
    latency->SetMaximumFrameLatency(1);state.frameReady=latency->GetFrameLatencyWaitableObject();release(latency);
    if(!state.frameReady)return fail("Frame readiness handle");
    state.bridge=state.openDevice(state.device);
    if(!state.bridge)return fail("wglDXOpenDeviceNV",HRESULT_FROM_WIN32(GetLastError()));
    if(!surface(width,height))return false;
    state.ready=true;glfwSwapInterval(0);return true;
}
}
bool begin(void* window,int width,int height){
    if(state.disabled||width<=0||height<=0)return false;
    if(!state.ready&&!initialize(static_cast<HWND>(window),width,height))return false;
    if((state.width!=width||state.height!=height)&&!surface(width,height))return false;
    if(!state.lockObjects(state.bridge,1,&state.object))return fail("Frame lock",HRESULT_FROM_WIN32(GetLastError()));
    state.locked=true;return true;
}
void wait(){if(state.ready&&state.frameReady)WaitForSingleObjectEx(state.frameReady,100,FALSE);}
unsigned int framebuffer(){return state.fbo;}
bool end(){
    if(!state.ready||!state.locked)return false;
    if(!state.unlockObjects(state.bridge,1,&state.object))return fail("Frame unlock",HRESULT_FROM_WIN32(GetLastError()));
    state.locked=false;state.context->CopyResource(state.back,state.shared);return true;
}
bool present(int interval){
    if(!state.ready)return false;
    state.interval=interval;
    HRESULT result=state.chain->Present(static_cast<UINT>(interval),0);
    if(FAILED(result))return fail("Present",result);
    if(result==DXGI_STATUS_OCCLUDED)Sleep(20);
    state.frames++;return true;
}
bool active(){return state.ready;}
unsigned char* readback(int& width,int& height){
    if(!state.ready||state.locked)return nullptr;
    D3D11_TEXTURE2D_DESC description{};state.shared->GetDesc(&description);
    description.Usage=D3D11_USAGE_STAGING;description.BindFlags=0;description.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
    ID3D11Texture2D* staging=nullptr;
    if(FAILED(state.device->CreateTexture2D(&description,nullptr,&staging)))return nullptr;
    state.context->CopyResource(staging,state.shared);D3D11_MAPPED_SUBRESOURCE mapped{};
    if(FAILED(state.context->Map(staging,0,D3D11_MAP_READ,0,&mapped))){release(staging);return nullptr;}
    width=state.width;height=state.height;auto* pixels=static_cast<unsigned char*>(std::malloc(size_t(width)*height*4));
    if(pixels)for(int row=0;row<height;row++)std::memcpy(pixels+size_t(row)*width*4,static_cast<unsigned char*>(mapped.pData)+size_t(row)*mapped.RowPitch,size_t(width)*4);
    state.context->Unmap(staging,0);release(staging);return pixels;
}
void shutdown(){
    state.ready=false;dropSurface();if(state.bridge){state.closeDevice(state.bridge);state.bridge=nullptr;}
    release(state.chain);release(state.context);release(state.device);
    if(state.frameReady){CloseHandle(state.frameReady);state.frameReady=nullptr;}
    restoreDrawable();
}
const char* name(){return state.ready?"DXGI flip / V-sync":"OpenGL V-sync";}
const char* error(){return state.reason;}
unsigned int presentCount(){return state.frames;}
unsigned int suppressedSwaps(){return state.swaps;}
int lastSyncInterval(){return state.interval;}
void countSuppressedSwap(){state.swaps++;}
}
extern "C" void __wrap_glfwSwapBuffers(GLFWwindow* window){
    // EndDrawing still updates raylib's clock and polls input. Only its old
    // unsynchronized window swap is suppressed while DXGI owns presentation.
    if(sync_present::active())sync_present::countSuppressedSwap();else __real_glfwSwapBuffers(window);
}
#else
namespace sync_present {
bool begin(void*,int,int){return false;}unsigned int framebuffer(){return 0;}bool end(){return false;}
void wait(){}
bool present(int){return false;}bool active(){return false;}unsigned char* readback(int&,int&){return nullptr;}
void shutdown(){}const char* name(){return "OpenGL V-sync";}const char* error(){return "";}
unsigned int presentCount(){return 0;}unsigned int suppressedSwaps(){return 0;}int lastSyncInterval(){return 0;}
}
#endif
