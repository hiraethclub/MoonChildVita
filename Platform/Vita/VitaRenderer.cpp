#include "VitaRenderer.h"

#include "PlatformConfig.h"
#include "SDL3Window.h"
#include "VitaLog.h"

#include <cstring>

// 1 while playing a level, 0 in the title sequence and menus
extern unsigned short ingameflg;

static const char* VERSION_OVERLAY_TEXT = "v" MOONCHILD_VITA_VERSION_STR " " MOONCHILD_GIT_HASH_STR;

bool VitaRenderer::Init(IWindow* hostWindow)
{
    SDL3Window* sdlWindow = dynamic_cast<SDL3Window*>(hostWindow);
    if (sdlWindow == nullptr || sdlWindow->GetNativeWindow() == nullptr)
    {
        VitaLog::Write("Renderer: window is not an SDL3 window!");
        return false;
    }
    SDL_Window* window = sdlWindow->GetNativeWindow();

    int windowWidth = 0;
    int windowHeight = 0;
    SDL_GetWindowSizeInPixels(window, &windowWidth, &windowHeight);
    VitaLog::Write("Window: %dx%d pixels", windowWidth, windowHeight);

    const int driverCount = SDL_GetNumRenderDrivers();
    for (int i = 0; i < driverCount; ++i)
    {
        VitaLog::Write("Render driver %d available: %s", i, SDL_GetRenderDriver(i));
    }

    Renderer = SDL_CreateRenderer(window, nullptr);
    if (Renderer == nullptr)
    {
        VitaLog::Write("SDL_CreateRenderer failed: %s", SDL_GetError());
        return false;
    }

    int outputWidth = 0;
    int outputHeight = 0;
    SDL_GetCurrentRenderOutputSize(Renderer, &outputWidth, &outputHeight);
    VitaLog::Write("Renderer: %s, output %dx%d", SDL_GetRendererName(Renderer), outputWidth, outputHeight);

    if (!SDL_SetRenderVSync(Renderer, 1))
    {
        VitaLog::Write("SDL_SetRenderVSync failed: %s", SDL_GetError());
    }

    const int w = GAME_FRAMEBUFFER_WIDTH;
    const int h = GAME_FRAMEBUFFER_HEIGHT;
    for (int i = 0; i < TEXTURE_COUNT; ++i)
    {
        // RGBA32 matches the byte order of the game's pixel buffer
        Textures[i] = SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, w, h);
        if (Textures[i] == nullptr)
        {
            VitaLog::Write("SDL_CreateTexture failed: %s", SDL_GetError());
            Destroy();
            return false;
        }
        SDL_SetTextureScaleMode(Textures[i], SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(Textures[i], SDL_BLENDMODE_NONE);
    }
    VitaLog::Write("Renderer: %d streaming textures of %dx%d created", TEXTURE_COUNT, w, h);

    StatsStartNs = SDL_GetTicksNS();
    return true;
}

void VitaRenderer::Destroy()
{
    for (int i = 0; i < TEXTURE_COUNT; ++i)
    {
        if (Textures[i] != nullptr)
        {
            SDL_DestroyTexture(Textures[i]);
            Textures[i] = nullptr;
        }
    }
    if (Renderer != nullptr)
    {
        SDL_DestroyRenderer(Renderer);
        Renderer = nullptr;
    }
}

void VitaRenderer::BeginFrame()
{
    SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255);
    SDL_RenderClear(Renderer);
}

void VitaRenderer::DrawFrame(const unsigned char* rgbaPixels, int width, int height)
{
    // Alternate between two textures so we never write into one the GPU might still be reading
    CurrentTexture = (CurrentTexture + 1) % TEXTURE_COUNT;
    SDL_Texture* texture = Textures[CurrentTexture];

    if (!SDL_UpdateTexture(texture, nullptr, rgbaPixels, width * 4))
    {
        static bool reported = false;
        if (!reported)
        {
            VitaLog::Write("SDL_UpdateTexture failed: %s", SDL_GetError());
            reported = true;
        }
    }

    int outputWidth = 0;
    int outputHeight = 0;
    SDL_GetCurrentRenderOutputSize(Renderer, &outputWidth, &outputHeight);

    // 1:1 and centred, with borders
    SDL_FRect dst;
    dst.w = static_cast<float>(width);
    dst.h = static_cast<float>(height);
    if (outputWidth < width || outputHeight < height)
    {
        // Shouldn't happen on a Vita, but scale down to fit rather than crop
        const float scaleX = static_cast<float>(outputWidth) / width;
        const float scaleY = static_cast<float>(outputHeight) / height;
        const float scale = (scaleX < scaleY) ? scaleX : scaleY;
        dst.w = width * scale;
        dst.h = height * scale;
    }
    dst.x = static_cast<float>(static_cast<int>((outputWidth - dst.w) * 0.5f));
    dst.y = static_cast<float>(static_cast<int>((outputHeight - dst.h) * 0.5f));

    SDL_RenderTexture(Renderer, texture, nullptr, &dst);

    if (ingameflg == 0)
    {
        DrawVersionOverlay(outputWidth, outputHeight);
    }
}

void VitaRenderer::DrawVersionOverlay(int outputWidth, int outputHeight)
{
    // Bottom right corner, which is in the black border with the 1:1 layout
    const int textWidth = static_cast<int>(std::strlen(VERSION_OVERLAY_TEXT)) * SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;
    const float x = static_cast<float>(outputWidth - textWidth - 4);
    const float y = static_cast<float>(outputHeight - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE - 4);

    SDL_SetRenderDrawColor(Renderer, 110, 110, 110, 255);
    SDL_RenderDebugText(Renderer, x, y, VERSION_OVERLAY_TEXT);
}

void VitaRenderer::EndFrame()
{
    if (!SDL_RenderPresent(Renderer))
    {
        static bool reported = false;
        if (!reported)
        {
            VitaLog::Write("SDL_RenderPresent failed: %s", SDL_GetError());
            reported = true;
        }
    }
    LogFrameStats();
}

void VitaRenderer::LogFrameStats()
{
    // Presented frames per second, at 5s, 30s and then every 2 minutes
    ++StatsFrames;
    const uint64_t now = SDL_GetTicksNS();
    const uint64_t elapsed = now - StatsStartNs;
    const uint64_t interval = (StatsReports == 0) ? 5000000000ull : (StatsReports == 1) ? 25000000000ull : 120000000000ull;
    if (elapsed < interval)
    {
        return;
    }

    const double fps = static_cast<double>(StatsFrames) * 1e9 / static_cast<double>(elapsed);
    VitaLog::Write("Presented %llu frames in %.2f s (%.2f fps)",
                   static_cast<unsigned long long>(StatsFrames), elapsed / 1e9, fps);
    StatsStartNs = now;
    StatsFrames = 0;
    ++StatsReports;
}

void VitaRenderer::DisplaySetVSync(bool enabled)
{
    if (Renderer == nullptr)
    {
        return;
    }
    VitaLog::Write("VSync %s", enabled ? "on" : "off");
    if (!SDL_SetRenderVSync(Renderer, enabled ? 1 : SDL_RENDERER_VSYNC_DISABLED))
    {
        VitaLog::Write("SDL_SetRenderVSync failed: %s", SDL_GetError());
    }
}
