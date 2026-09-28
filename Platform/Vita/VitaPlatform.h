#pragma once

// Game data is packaged into the VPK, saves and settings go to the memory card
#define VITA_DATA_DIR "app0:data/"
#define VITA_WRITABLE_DIR "ux0:data/MoonChildFE/"

namespace VitaPlatform
{
    // Called first thing in main
    void Startup();

    // Called once all backends are up, logs what SDL picked
    void LogBackendInfo();

    // Called when the main loop returns
    void Shutdown(int exitCode);
}
