#include "sync_present.hpp"
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <d3dcompiler.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
extern "C" void (*glfwGetProcAddress(const char*))(void);
extern "C" void glfwSwapInterval(int);
struct GLFWwindow;
extern "C" void __real_glfwSwapBuffers(GLFWwindow*);
namespace sync_present {
namespace {
struct Slot {GLuint buffer=0;GLsync fence=nullptr;unsigned sequence=0;};
struct Presenter {
    ID3D11Device* device=nullptr;ID3D11DeviceContext* context=nullptr;
    IDXGISwapChain1* chain=nullptr;ID3D11Texture2D* back=nullptr;ID3D11Texture2D* upload=nullptr;
    ID3D11RenderTargetView* target=nullptr;ID3D11ShaderResourceView* source=nullptr;
    ID3D11VertexShader* vertexShader=nullptr;ID3D11PixelShader* pixelShader=nullptr;
    ID3D11SamplerState* sampler=nullptr;ID3D11RasterizerState* rasterizer=nullptr;
    std::vector<unsigned char> pixels;
    HWND hiddenWindow=nullptr,presentWindow=nullptr;HDC originalDc=nullptr,hiddenDc=nullptr;HGLRC originalContext=nullptr;
    GLuint texture=0,fbo=0;std::array<Slot,3> slots{};
    int width=0,height=0,cursor=0,interval=0;bool ready=false,disabled=false,dirty=false;
    unsigned frames=0,swaps=0,sequence=0,uploaded=0;char reason[160]{};
    PFNGLGENFRAMEBUFFERSPROC genFramebuffers=nullptr;PFNGLDELETEFRAMEBUFFERSPROC deleteFramebuffers=nullptr;
    PFNGLBINDFRAMEBUFFERPROC bindFramebuffer=nullptr;PFNGLFRAMEBUFFERTEXTURE2DPROC attachTexture=nullptr;
    PFNGLCHECKFRAMEBUFFERSTATUSPROC checkFramebuffer=nullptr;
    PFNGLGENBUFFERSPROC genBuffers=nullptr;PFNGLDELETEBUFFERSPROC deleteBuffers=nullptr;
    PFNGLBINDBUFFERPROC bindBuffer=nullptr;PFNGLBUFFERDATAPROC bufferData=nullptr;
    PFNGLMAPBUFFERRANGEPROC mapBuffer=nullptr;PFNGLUNMAPBUFFERPROC unmapBuffer=nullptr;
    PFNGLFENCESYNCPROC fenceSync=nullptr;PFNGLCLIENTWAITSYNCPROC clientWait=nullptr;PFNGLDELETESYNCPROC deleteSync=nullptr;
} state;
double stageTimes[4]{};
double clockNow(){LARGE_INTEGER value,frequency;QueryPerformanceCounter(&value);QueryPerformanceFrequency(&frequency);return double(value.QuadPart)/frequency.QuadPart;}
template<class T>void release(T*& p){if(p){p->Release();p=nullptr;}}
template<class T>T load(const char* name){return reinterpret_cast<T>(glfwGetProcAddress(name));}
void restoreDrawable(){
    if(state.originalDc&&state.originalContext)wglMakeCurrent(state.originalDc,state.originalContext);
    if(state.hiddenDc){ReleaseDC(state.hiddenWindow,state.hiddenDc);state.hiddenDc=nullptr;}
    if(state.hiddenWindow){DestroyWindow(state.hiddenWindow);state.hiddenWindow=nullptr;}
    if(state.presentWindow){DestroyWindow(state.presentWindow);state.presentWindow=nullptr;}
    state.originalDc=nullptr;state.originalContext=nullptr;
}
bool offscreenDrawable(HWND owner){
    state.originalDc=wglGetCurrentDC();state.originalContext=wglGetCurrentContext();
    WNDCLASSW wc{};wc.style=CS_OWNDC;wc.lpfnWndProc=DefWindowProcW;
    wc.hInstance=GetModuleHandleW(nullptr);wc.lpszClassName=L"EmberfallOffscreenRenderer";
    if(!RegisterClassW(&wc)&&GetLastError()!=ERROR_CLASS_ALREADY_EXISTS)return false;
    state.hiddenWindow=CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE,wc.lpszClassName,L"",WS_POPUP,0,0,1,1,owner,nullptr,wc.hInstance,nullptr);
    if(!state.hiddenWindow)return false;
    state.hiddenDc=GetDC(state.hiddenWindow);PIXELFORMATDESCRIPTOR format{};int index=GetPixelFormat(state.originalDc);
    if(!state.hiddenDc||!DescribePixelFormat(state.originalDc,index,sizeof(format),&format)||!SetPixelFormat(state.hiddenDc,index,&format))return false;
    // Keep all existing raylib resources in the same GL context. Only DXGI
    // presents to the visible window; GL draws to offscreen framebuffers.
    return wglMakeCurrent(state.hiddenDc,state.originalContext)!=FALSE;
}
void dropSurface(){
    if(state.context){state.context->OMSetRenderTargets(0,nullptr,nullptr);ID3D11ShaderResourceView* empty=nullptr;state.context->PSSetShaderResources(0,1,&empty);}
    for(auto& slot:state.slots){if(slot.fence)state.deleteSync(slot.fence);if(slot.buffer)state.deleteBuffers(1,&slot.buffer);slot={};}
    if(state.fbo){state.deleteFramebuffers(1,&state.fbo);state.fbo=0;}
    if(state.texture){glDeleteTextures(1,&state.texture);state.texture=0;}
    release(state.target);release(state.source);release(state.back);release(state.upload);state.cursor=0;state.uploaded=0;state.sequence=0;
}
bool fail(const char* operation,HRESULT hr=E_FAIL){
    std::snprintf(state.reason,sizeof(state.reason),"%s (0x%08lx)",operation,static_cast<unsigned long>(hr));
    state.ready=false;state.disabled=true;dropSurface();release(state.chain);release(state.vertexShader);release(state.pixelShader);release(state.sampler);release(state.rasterizer);release(state.context);release(state.device);
    restoreDrawable();glfwSwapInterval(state.interval>0?state.interval:1);return false;
}
bool surface(int width,int height){
    if(state.presentWindow)SetWindowPos(state.presentWindow,nullptr,0,0,width,height,SWP_NOACTIVATE|SWP_NOZORDER);
    dropSurface();HRESULT hr=state.chain->ResizeBuffers(2,width,height,DXGI_FORMAT_R8G8B8A8_UNORM,0);
    if(FAILED(hr))return fail("ResizeBuffers",hr);
    hr=state.chain->GetBuffer(0,__uuidof(ID3D11Texture2D),reinterpret_cast<void**>(&state.back));
    if(FAILED(hr))return fail("GetBuffer",hr);
    hr=state.device->CreateRenderTargetView(state.back,nullptr,&state.target);if(FAILED(hr))return fail("Create target view",hr);
    D3D11_TEXTURE2D_DESC td{};td.Width=width;td.Height=height;td.MipLevels=1;td.ArraySize=1;
    td.Format=DXGI_FORMAT_R8G8B8A8_UNORM;td.SampleDesc.Count=1;td.Usage=D3D11_USAGE_DEFAULT;td.BindFlags=D3D11_BIND_SHADER_RESOURCE;
    hr=state.device->CreateTexture2D(&td,nullptr,&state.upload);if(FAILED(hr))return fail("Create upload surface",hr);
    hr=state.device->CreateShaderResourceView(state.upload,nullptr,&state.source);if(FAILED(hr))return fail("Create source view",hr);
    glGenTextures(1,&state.texture);glBindTexture(GL_TEXTURE_2D,state.texture);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    state.genFramebuffers(1,&state.fbo);state.bindFramebuffer(GL_FRAMEBUFFER,state.fbo);
    state.attachTexture(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,state.texture,0);
    GLenum status=state.checkFramebuffer(GL_FRAMEBUFFER);state.bindFramebuffer(GL_FRAMEBUFFER,0);
    if(status!=GL_FRAMEBUFFER_COMPLETE)return fail("Final framebuffer is incomplete");
    for(auto& slot:state.slots){state.genBuffers(1,&slot.buffer);state.bindBuffer(GL_PIXEL_PACK_BUFFER,slot.buffer);
        state.bufferData(GL_PIXEL_PACK_BUFFER,size_t(width)*height*4,nullptr,GL_STREAM_READ);}
    state.bindBuffer(GL_PIXEL_PACK_BUFFER,0);state.width=width;state.height=height;state.pixels.resize(size_t(width)*height*4);return true;
}
bool shaders(){
    const char* code="struct V{float4 p:SV_Position;float2 uv:TEXCOORD0;};V vs(uint id:SV_VertexID){V v;v.uv=float2((id<<1)&2,id&2);v.p=float4(v.uv*float2(2,-2)+float2(-1,1),0,1);return v;}Texture2D image:register(t0);SamplerState sampleState:register(s0);float4 ps(V v):SV_Target{return image.Sample(sampleState,v.uv);}";
    ID3DBlob* vertex=nullptr;ID3DBlob* pixel=nullptr;ID3DBlob* errors=nullptr;
    HRESULT hr=D3DCompile(code,std::strlen(code),nullptr,nullptr,nullptr,"vs","vs_4_0",D3DCOMPILE_OPTIMIZATION_LEVEL3,0,&vertex,&errors);release(errors);
    if(SUCCEEDED(hr))hr=D3DCompile(code,std::strlen(code),nullptr,nullptr,nullptr,"ps","ps_4_0",D3DCOMPILE_OPTIMIZATION_LEVEL3,0,&pixel,&errors);release(errors);
    if(SUCCEEDED(hr))hr=state.device->CreateVertexShader(vertex->GetBufferPointer(),vertex->GetBufferSize(),nullptr,&state.vertexShader);
    if(SUCCEEDED(hr))hr=state.device->CreatePixelShader(pixel->GetBufferPointer(),pixel->GetBufferSize(),nullptr,&state.pixelShader);
    release(vertex);release(pixel);if(FAILED(hr))return fail("Presentation shaders",hr);
    D3D11_SAMPLER_DESC sampler{};sampler.Filter=D3D11_FILTER_MIN_MAG_MIP_POINT;sampler.AddressU=sampler.AddressV=sampler.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;sampler.MaxLOD=D3D11_FLOAT32_MAX;
    hr=state.device->CreateSamplerState(&sampler,&state.sampler);if(FAILED(hr))return fail("Presentation sampler",hr);
    D3D11_RASTERIZER_DESC rasterizer{};rasterizer.FillMode=D3D11_FILL_SOLID;rasterizer.CullMode=D3D11_CULL_NONE;rasterizer.DepthClipEnable=TRUE;
    hr=state.device->CreateRasterizerState(&rasterizer,&state.rasterizer);return SUCCEEDED(hr)||fail("Presentation rasterizer",hr);
}
bool initialize(HWND window,int width,int height){
    ShowWindow(window,SW_SHOWNORMAL);
    state.genFramebuffers=load<PFNGLGENFRAMEBUFFERSPROC>("glGenFramebuffers");state.deleteFramebuffers=load<PFNGLDELETEFRAMEBUFFERSPROC>("glDeleteFramebuffers");
    state.bindFramebuffer=load<PFNGLBINDFRAMEBUFFERPROC>("glBindFramebuffer");state.attachTexture=load<PFNGLFRAMEBUFFERTEXTURE2DPROC>("glFramebufferTexture2D");
    state.checkFramebuffer=load<PFNGLCHECKFRAMEBUFFERSTATUSPROC>("glCheckFramebufferStatus");
    state.genBuffers=load<PFNGLGENBUFFERSPROC>("glGenBuffers");state.deleteBuffers=load<PFNGLDELETEBUFFERSPROC>("glDeleteBuffers");
    state.bindBuffer=load<PFNGLBINDBUFFERPROC>("glBindBuffer");state.bufferData=load<PFNGLBUFFERDATAPROC>("glBufferData");
    state.mapBuffer=load<PFNGLMAPBUFFERRANGEPROC>("glMapBufferRange");state.unmapBuffer=load<PFNGLUNMAPBUFFERPROC>("glUnmapBuffer");
    state.fenceSync=load<PFNGLFENCESYNCPROC>("glFenceSync");state.clientWait=load<PFNGLCLIENTWAITSYNCPROC>("glClientWaitSync");state.deleteSync=load<PFNGLDELETESYNCPROC>("glDeleteSync");
    if(!state.genFramebuffers||!state.deleteFramebuffers||!state.bindFramebuffer||!state.attachTexture||!state.checkFramebuffer
       ||!state.genBuffers||!state.deleteBuffers||!state.bindBuffer||!state.bufferData||!state.mapBuffer||!state.unmapBuffer
       ||!state.fenceSync||!state.clientWait||!state.deleteSync)return fail("Asynchronous frame transfer is unavailable");
    // Release the old drawable's swap policy before moving the context and
    // creating DXGI. WGL swap control belongs to the current drawable.
    glfwSwapInterval(0);
    if(!offscreenDrawable(window))return fail("Offscreen rendering drawable",HRESULT_FROM_WIN32(GetLastError()));
    HRESULT hr=D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_HARDWARE,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&state.device,nullptr,&state.context);
    if(FAILED(hr))return fail("D3D11CreateDevice",hr);
    if(!shaders())return false;
    IDXGIDevice1* dxDevice=nullptr;IDXGIAdapter* adapter=nullptr;IDXGIFactory2* factory=nullptr;
    hr=state.device->QueryInterface(__uuidof(IDXGIDevice1),reinterpret_cast<void**>(&dxDevice));
    if(SUCCEEDED(hr)){dxDevice->SetMaximumFrameLatency(3);hr=dxDevice->GetAdapter(&adapter);}
    if(SUCCEEDED(hr))hr=adapter->GetParent(__uuidof(IDXGIFactory2),reinterpret_cast<void**>(&factory));
    release(adapter);release(dxDevice);if(FAILED(hr)){release(factory);return fail("DXGI factory",hr);}
    DXGI_SWAP_CHAIN_DESC1 sd{};sd.Width=width;sd.Height=height;sd.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.SampleDesc.Count=1;sd.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;sd.BufferCount=2;sd.Scaling=DXGI_SCALING_STRETCH;
    sd.SwapEffect=DXGI_SWAP_EFFECT_FLIP_DISCARD;sd.AlphaMode=DXGI_ALPHA_MODE_IGNORE;
    hr=factory->CreateSwapChainForHwnd(state.device,window,&sd,nullptr,nullptr,&state.chain);
    if(FAILED(hr)){sd.SwapEffect=DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;hr=factory->CreateSwapChainForHwnd(state.device,window,&sd,nullptr,nullptr,&state.chain);}
    factory->MakeWindowAssociation(window,DXGI_MWA_NO_ALT_ENTER);release(factory);
    if(FAILED(hr))return fail("CreateSwapChainForHwnd",hr);
    if(!surface(width,height))return false;
    state.ready=true;glfwSwapInterval(0);return true;
}
bool uploadReady(bool latest){
    Slot* chosen=nullptr;
    for(auto& slot:state.slots){
        if(!slot.fence||slot.sequence<=state.uploaded||(latest&&slot.sequence!=state.sequence))continue;
        GLenum ready=state.clientWait(slot.fence,latest?GL_SYNC_FLUSH_COMMANDS_BIT:0,latest?1000000000ull:0);
        if((ready==GL_ALREADY_SIGNALED||ready==GL_CONDITION_SATISFIED)&&(!chosen||slot.sequence>chosen->sequence))chosen=&slot;
    }
    if(chosen){
        state.bindBuffer(GL_PIXEL_PACK_BUFFER,chosen->buffer);
        void* pixels=state.mapBuffer(GL_PIXEL_PACK_BUFFER,0,size_t(state.width)*state.height*4,GL_MAP_READ_BIT);
        if(!pixels){state.bindBuffer(GL_PIXEL_PACK_BUFFER,0);return fail("Frame transfer map");}
        std::memcpy(state.pixels.data(),pixels,state.pixels.size());
        state.unmapBuffer(GL_PIXEL_PACK_BUFFER);state.bindBuffer(GL_PIXEL_PACK_BUFFER,0);
        state.dirty=true;state.uploaded=chosen->sequence;
    }
    return true;
}
void drawFrame(){
    if(state.dirty){state.context->UpdateSubresource(state.upload,0,nullptr,state.pixels.data(),state.width*4,0);state.dirty=false;}
    D3D11_VIEWPORT viewport{0,0,float(state.width),float(state.height),0,1};state.context->RSSetViewports(1,&viewport);state.context->RSSetState(state.rasterizer);
    state.context->OMSetRenderTargets(1,&state.target,nullptr);state.context->IASetInputLayout(nullptr);state.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    state.context->VSSetShader(state.vertexShader,nullptr,0);state.context->PSSetShader(state.pixelShader,nullptr,0);
    state.context->PSSetShaderResources(0,1,&state.source);state.context->PSSetSamplers(0,1,&state.sampler);state.context->Draw(3,0);
}
}
bool begin(void* window,int width,int height){
    if(state.disabled||width<=0||height<=0)return false;
    if(!state.ready&&!initialize(static_cast<HWND>(window),width,height))return false;
    if((state.width!=width||state.height!=height)&&!surface(width,height))return false;return true;
}
void wait(){}
unsigned int framebuffer(){return state.fbo;}
bool end(){
    if(!state.ready)return false;
    Slot& slot=state.slots[state.cursor];if(slot.fence)state.deleteSync(slot.fence);
    state.bindFramebuffer(GL_READ_FRAMEBUFFER,state.fbo);state.bindBuffer(GL_PIXEL_PACK_BUFFER,slot.buffer);
    glReadPixels(0,0,state.width,state.height,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);
    slot.fence=state.fenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,0);slot.sequence=++state.sequence;
    state.bindBuffer(GL_PIXEL_PACK_BUFFER,0);state.bindFramebuffer(GL_READ_FRAMEBUFFER,0);glFlush();
    state.cursor=(state.cursor+1)%int(state.slots.size());return uploadReady(state.uploaded==0);
}
bool present(int interval){
    if(!state.ready)return false;state.interval=interval;
    double a=clockNow();wglMakeCurrent(nullptr,nullptr);double b=clockNow();drawFrame();double c=clockNow();HRESULT hr=state.chain->Present(static_cast<UINT>(interval),0);double d=clockNow();
    wglMakeCurrent(state.hiddenDc,state.originalContext);double e=clockNow();stageTimes[0]=b-a;stageTimes[1]=c-b;stageTimes[2]=d-c;stageTimes[3]=e-d;
    if(FAILED(hr))return fail("Present",hr);if(hr==DXGI_STATUS_OCCLUDED){std::snprintf(state.reason,sizeof(state.reason),"Occluded: child visible=%d, parent visible=%d",IsWindowVisible(state.presentWindow),IsWindowVisible(GetParent(state.presentWindow)));Sleep(20);}state.frames++;return true;
}
bool active(){return state.ready;}
unsigned char* readback(int& width,int& height){
    if(!state.ready||!uploadReady(true))return nullptr;
    drawFrame();
    D3D11_TEXTURE2D_DESC td{};state.back->GetDesc(&td);td.Usage=D3D11_USAGE_STAGING;td.BindFlags=0;td.MiscFlags=0;td.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
    ID3D11Texture2D* staging=nullptr;if(FAILED(state.device->CreateTexture2D(&td,nullptr,&staging)))return nullptr;
    state.context->CopyResource(staging,state.back);D3D11_MAPPED_SUBRESOURCE mapped{};
    if(FAILED(state.context->Map(staging,0,D3D11_MAP_READ,0,&mapped))){release(staging);return nullptr;}
    width=state.width;height=state.height;auto* pixels=static_cast<unsigned char*>(std::malloc(size_t(width)*height*4));
    if(pixels)for(int row=0;row<height;row++)std::memcpy(pixels+size_t(row)*width*4,static_cast<unsigned char*>(mapped.pData)+size_t(row)*mapped.RowPitch,size_t(width)*4);
    state.context->Unmap(staging,0);release(staging);return pixels;
}
void shutdown(){state.ready=false;dropSurface();release(state.chain);release(state.vertexShader);release(state.pixelShader);release(state.sampler);release(state.rasterizer);release(state.context);release(state.device);restoreDrawable();}
const char* name(){return state.ready?"DXGI flip / V-sync":"OpenGL V-sync";}
const char* error(){return state.reason;}
unsigned presentCount(){return state.frames;}unsigned suppressedSwaps(){return state.swaps;}int lastSyncInterval(){return state.interval;}
void countSuppressedSwap(){state.swaps++;}
double stageTime(int stage){return stageTimes[stage];}
}
extern "C" void __wrap_glfwSwapBuffers(GLFWwindow* window){
    if(sync_present::active())sync_present::countSuppressedSwap();else __real_glfwSwapBuffers(window);
}
#else
namespace sync_present {
bool begin(void*,int,int){return false;}void wait(){}unsigned int framebuffer(){return 0;}bool end(){return false;}
bool present(int){return false;}bool active(){return false;}unsigned char* readback(int&,int&){return nullptr;}
void shutdown(){}const char* name(){return "OpenGL V-sync";}const char* error(){return "";}
unsigned int presentCount(){return 0;}unsigned int suppressedSwaps(){return 0;}int lastSyncInterval(){return 0;}
}
#endif
