#pragma once

// The final raylib frame goes directly through a shared GPU surface to DXGI.
// DXGI owns the visible swap and always presents with a positive V-sync interval.
namespace sync_present {
bool begin(void* window,int width,int height);
unsigned int framebuffer();
bool end();
bool present(int interval);
bool active();
unsigned char* readback(int& width,int& height);
void shutdown();
const char* name();
const char* error();
unsigned int presentCount();
unsigned int suppressedSwaps();
int lastSyncInterval();
unsigned int occludedCount();
}
