#include "Host.h"

static int TheBeginningOfANewAndExcitinglyDifferentStory()
{
#ifdef __EMSCRIPTEN__
    static Host host;
#else
    Host host;
#endif

    return host.Run();
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
