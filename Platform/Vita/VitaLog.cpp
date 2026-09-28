#include "VitaLog.h"

#include "VitaPlatform.h"

#include <psp2/kernel/processmgr.h>

#include <cstdarg>
#include <cstdio>
#include <set>
#include <string>
#include <sys/stat.h>

static FILE* LogFile = nullptr;
static std::set<std::string> SeenPaths;

namespace VitaLog
{
    void Init()
    {
        if (LogFile != nullptr)
        {
            return;
        }

        // No trailing slash, to be safe with sceIoMkdir
        mkdir("ux0:data/MoonChildFE", 0777);

        char logPath[256];
        char prevPath[256];
        std::snprintf(logPath, sizeof(logPath), "%slog.txt", VITA_WRITABLE_DIR);
        std::snprintf(prevPath, sizeof(prevPath), "%slog_prev.txt", VITA_WRITABLE_DIR);
        std::remove(prevPath);
        std::rename(logPath, prevPath);

        // Truncate, then open every handle in append mode so they don't overwrite each other
        FILE* truncated = std::fopen(logPath, "w");
        if (truncated != nullptr)
        {
            std::fclose(truncated);
        }

        LogFile = std::fopen(logPath, "a");
        if (LogFile == nullptr)
        {
            return;
        }
        setvbuf(LogFile, nullptr, _IONBF, 0);

        // Send the existing printf error messages from the shared code to the log as well
        if (std::freopen(logPath, "a", stdout) != nullptr)
        {
            setvbuf(stdout, nullptr, _IONBF, 0);
        }
        if (std::freopen(logPath, "a", stderr) != nullptr)
        {
            setvbuf(stderr, nullptr, _IONBF, 0);
        }
    }

    void Write(const char* format, ...)
    {
        if (LogFile == nullptr)
        {
            return;
        }

        const unsigned long long ms = sceKernelGetProcessTimeWide() / 1000ull;
        std::fprintf(LogFile, "[%6llu.%03llu] ", ms / 1000ull, ms % 1000ull);

        va_list args;
        va_start(args, format);
        std::vfprintf(LogFile, format, args);
        va_end(args);

        std::fputc('\n', LogFile);
        std::fflush(LogFile);
    }

    void NotePath(const char* path)
    {
        if (path == nullptr || !SeenPaths.insert(path).second)
        {
            return;
        }

        struct stat info;
        const bool exists = stat(path, &info) == 0;
        if (exists)
        {
            Write("File: %s (%ld bytes)", path, static_cast<long>(info.st_size));
        }
        else
        {
            Write("File: %s (NOT FOUND)", path);
        }
    }
}
