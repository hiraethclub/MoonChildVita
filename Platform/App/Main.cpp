#include "Host.h"

#ifdef MOONCHILD_VITA
#include "VitaPlatform.h"
#endif

static int TheBeginningOfANewAndExcitinglyDifferentStory()
{
#ifdef MOONCHILD_VITA
    VitaPlatform::Startup();
#endif

#ifdef __EMSCRIPTEN__
    static Host host;
#else
    Host host;
#endif

    const int result = host.Run();

#ifdef MOONCHILD_VITA
    VitaPlatform::Shutdown(result);
#endif

    return result;
}

#if defined(_WIN32) && !defined(__EMSCRIPTEN__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    return TheBeginningOfANewAndExcitinglyDifferentStory();
}
#else
int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    return TheBeginningOfANewAndExcitinglyDifferentStory();
}
#endif
