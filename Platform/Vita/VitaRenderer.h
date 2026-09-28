#pragma once

#include "IRenderer.h"

#include <SDL3/SDL.h>

#include <cstdint>

// PS Vita renderer using SDL's native GXM render driver
// The Vita has no desktop OpenGL, so the GL/GLES renderers can't be used here
class VitaRenderer final : public IRenderer
{
public:
    bool Init(IWindow* hostWindow) override;
    void Destroy() override;

    void BeginFrame() override;
    void DrawFrame(const unsigned char* rgbaPixels, int width, int height) override;
    void EndFrame() override;

    void DisplaySetVSync(bool enabled) override;

private:
    void DrawVersionOverlay(int outputWidth, int outputHeight);
    void LogFrameStats();

    static constexpr int TEXTURE_COUNT = 2;

    SDL_Renderer* Renderer = nullptr;
    SDL_Texture* Textures[TEXTURE_COUNT] = {};
    int CurrentTexture = 0;

    uint64_t StatsStartNs = 0;
    uint64_t StatsFrames = 0;
    int StatsReports = 0;
};
