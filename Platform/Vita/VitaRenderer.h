#pragma once

#include "IRenderer.h"

#include <SDL3/SDL.h>

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

    // false: 1:1 and centred with borders, true: aspect correct 725x544
    void SetFillScreen(bool enabled);

private:
    void DrawVersionOverlay(int outputWidth, int outputHeight);

    static constexpr int TEXTURE_COUNT = 2;

    // Fill mode scales up by this much with nearest filtering first, then down with linear filtering
    // That keeps the pixels sharp without the uneven widths of plain nearest scaling by 1.133
    static constexpr int FILL_PRESCALE = 2;
    static constexpr int FILL_WIDTH = 725;

    SDL_Renderer* Renderer = nullptr;
    SDL_Texture* Textures[TEXTURE_COUNT] = {};
    SDL_Texture* PrescaleTarget = nullptr;
    int CurrentTexture = 0;
    bool FillScreen = false;
};
