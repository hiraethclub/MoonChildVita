#include "VitaPlatform.h"

#include "VitaLog.h"

#include <SDL3/SDL.h>

#include <psp2/kernel/processmgr.h>
#include <psp2/power.h>

// Newlib heap for the game, the GPU memory is allocated separately by SDL
int _newlib_heap_size_user = 192 * 1024 * 1024;

// The default main thread stack is small, and the original code likes big locals
unsigned int sceUserMainThreadStackSize = 2 * 1024 * 1024;

namespace VitaPlatform
{
    void Startup()
    {
        VitaLog::Init();
        VitaLog::Write("Moon Child FE for PS Vita %s (%s), game version %s",
                       MOONCHILD_VITA_VERSION_STR, MOONCHILD_GIT_HASH_STR, MOONCHILD_APP_VERSION_STR);
        VitaLog::Write("Built %s %s", __DATE__, __TIME__);

        const int sdlVersion = SDL_GetVersion();
        VitaLog::Write("SDL %d.%d.%d (%s)",
                       SDL_VERSIONNUM_MAJOR(sdlVersion), SDL_VERSIONNUM_MINOR(sdlVersion), SDL_VERSIONNUM_MICRO(sdlVersion),
                       SDL_GetRevision());

        // Run at the standard maximum clocks, like most homebrew ports do
        // This only affects performance, the game logic is timed separately
        scePowerSetArmClockFrequency(444);
        scePowerSetBusClockFrequency(222);
        scePowerSetGpuClockFrequency(222);
        scePowerSetGpuXbarClockFrequency(166);
        VitaLog::Write("Clocks: CPU %d MHz, bus %d MHz, GPU %d MHz, xbar %d MHz",
                       scePowerGetArmClockFrequency(), scePowerGetBusClockFrequency(),
                       scePowerGetGpuClockFrequency(), scePowerGetGpuXbarClockFrequency());
    }

    void LogBackendInfo()
    {
        VitaLog::Write("Video driver: %s", SDL_GetCurrentVideoDriver() ? SDL_GetCurrentVideoDriver() : "(none)");

        const char* audioDriver = SDL_GetCurrentAudioDriver();
        VitaLog::Write("Audio driver: %s", audioDriver ? audioDriver : "(none)");

        SDL_AudioSpec spec = {};
        int sampleFrames = 0;
        if (SDL_GetAudioDeviceFormat(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, &sampleFrames))
        {
            const char* deviceName = SDL_GetAudioDeviceName(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK);
            VitaLog::Write("Audio device: %s, %d Hz, %d channels, format 0x%04x, %d sample frames",
                           deviceName ? deviceName : "(default)", spec.freq, spec.channels,
                           static_cast<unsigned>(spec.format), sampleFrames);
        }
        else
        {
            VitaLog::Write("Audio device format query failed: %s", SDL_GetError());
        }

        int count = 0;
        if (SDL_JoystickID* ids = SDL_GetGamepads(&count))
        {
            VitaLog::Write("Gamepads: %d", count);
            for (int i = 0; i < count; ++i)
            {
                const char* name = SDL_GetGamepadNameForID(ids[i]);
                VitaLog::Write("  Gamepad %d: %s", static_cast<int>(ids[i]), name ? name : "(unnamed)");
            }
            SDL_free(ids);
        }
        else
        {
            VitaLog::Write("Gamepad query failed: %s", SDL_GetError());
        }
    }

    void Shutdown(int exitCode)
    {
        VitaLog::Write("Main loop exited with code %d", exitCode);
    }
}
