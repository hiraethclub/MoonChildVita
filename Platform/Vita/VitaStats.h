#pragma once

// Frame timing breakdown for the log, so performance problems can be diagnosed without a debugger
namespace VitaStats
{
    enum Section
    {
        SECTION_EVENTS,   // OS event pumping
        SECTION_TICKS,    // game ticks, including game logic and drawing into the pixel buffer
        SECTION_MOVIE,    // FMV decoding, part of SECTION_TICKS
        SECTION_UPLOAD,   // copying the pixel buffer into a texture
        SECTION_DRAW,     // queueing the draw calls
        SECTION_PRESENT,  // SDL_RenderPresent, including waiting for a free display buffer
        SECTION_COUNT
    };

    void Begin(Section section);
    void End(Section section);

    // Called after every present, writes a report to the log every so often
    void FramePresented();
}
