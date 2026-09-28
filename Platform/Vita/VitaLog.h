#pragma once

// Log file for testing on real hardware, written to ux0:data/MoonChildFE/log.txt
// Every line is flushed straight away so the log survives a crash
// The previous run's log is kept as log_prev.txt
namespace VitaLog
{
    void Init();
    void Write(const char* format, ...) __attribute__((format(printf, 1, 2)));

    // Logs a resolved file path the first time it's seen, and whether it exists
    void NotePath(const char* path);
}
