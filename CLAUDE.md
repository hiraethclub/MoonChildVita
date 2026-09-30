# Moon Child FE: PS Vita port

This repo is a fork of [MorsGames/MoonChildFE](https://github.com/MorsGames/MoonChildFE) (full upstream history merged in, remote `upstream`) with a PlayStation Vita homebrew port added on top. It is a platform port: gameplay, physics, timing and level logic must not change.

## Building the Vita VPK

```bash
git submodule update --init --recursive   # or just Libraries/zlib and Libraries/pl_mpeg
bash Scripts/BuildGameVita.sh             # add "Clean" for a fresh build dir
```

- Without VitaSDK installed, the script re-runs itself inside the pinned `vitasdk/vitasdk` Docker image (digest in the script and in `.github/workflows/vita.yml`; update both together).
- In the cloud session the Docker daemon may need starting first: `nohup dockerd >/tmp/dockerd.log 2>&1 &`, then wait for `docker info`.
- Output: `Bin/Vita/MoonChildFE-vita-<version>-<shorthash>.vpk` plus the matching unstripped `.elf` (keep it for symbolising crash dumps).
- CMake preset: `vita` / `build-vita-release` (single config Ninja, required because `vita_create_self` expects the ELF in the binary dir). Toolchain wrapper `CMake/Toolchains/Vita.cmake` includes `$VITASDK/share/vita.toolchain.cmake`.
- Port version lives in `CMake/Platforms/Vita.cmake` (`MOONCHILD_VITA_VERSION`), or override with the `MOONCHILD_VITA_VERSION` env var. Bump it for every build handed over. APP_VER in param.sfo is derived from it (0.1.0 -> 00.10).
- Title ID: `MCFE01997`.
- Desktop builds are unaffected; verified with `bash Scripts/BuildGameUnix.sh LinuxX64`.
- CI: `.github/workflows/vita.yml` builds on every push and uploads the VPK + ELF as the `MoonChildFE-Vita` artifact.

## Layout and decisions

- Vita code lives in `Platform/Vita/`; shared files only get `#ifdef MOONCHILD_VITA` hooks (Main.cpp, Host.cpp, Resources.cpp, PlatformBackends.cpp) and small `VITA` guards in `CMakeLists.txt`.
- SDL3: uses the SDL3 **packaged in the vitasdk image (3.4.16)**, not the 3.4.10 submodule (`MOONCHILD_VENDORED_SDL3=OFF` in the preset). 3.4.16 contains a Vita GXM fix for texture updates (SDL commit e5648d443) that sits on our per-frame upload path. The submodule is left at 3.4.10 for desktop.
- Rendering: the Vita has no desktop OpenGL, so a new `VitaRenderer` (renderer backend value `Vita`) uses SDL_Renderer on SDL's native GXM driver. The window backend is the unchanged `SDL3Window` (SDL forces the window to 960x544). Two streaming textures are alternated so the CPU never overwrites one the GPU is still reading.
- `MOONCHILD_DESKTOP_MODE` is off on Vita (like Android), which hides the keyboard remap menu and fullscreen toggle in the game's own menus.
- gamecontrollerdb is not used on Vita; SDL's Vita joystick driver has a built-in mapping.
- Paths: data from `app0:data/`, saves/settings/log in `ux0:data/MoonChildFE/`.
- Heap: `_newlib_heap_size_user` = 192 MB, main thread stack 2 MB (`Platform/Vita/VitaPlatform.cpp`). CPU/GPU clocks set to 444/222/222/166 MHz at startup.
- Version overlay: `v<version> <hash>` drawn with SDL's debug font in the bottom right border whenever `ingameflg == 0` (title sequence and menus).
- LiveArea: only `icon0.png` so far, taken from the 128x128 image inside `Windows/game.ico`. bg/startup are Phase 3.

## Logging

`ux0:data/MoonChildFE/log.txt` (previous run kept as `log_prev.txt`). Unbuffered, so it survives crashes. Contains: version/hash/SDL version, clocks, render drivers and chosen renderer, window/output size, audio driver + device format, gamepads, data/writable paths, every resolved file path once with its size or NOT FOUND, frame rate at 5 s, 30 s and every 2 min, and all existing `printf`/`fprintf(stderr)` output from the shared code (stdout/stderr are redirected into the same file in append mode).

## Licences

- Code: MIT (Reinier van Vliet / proofofconcept.nl), see `LICENSE`.
- Assets (graphics, audio, music, movies): CC BY-NC 4.0. Graphics by Metin Seven, music and sound by Ramon Braumuller. Redistribution in a free, non-commercial VPK with attribution is allowed; LICENSE and README (credits) are bundled inside the VPK.
- The fork's original initial commit had an AGPL-3.0 LICENSE; it was replaced by upstream's dual licence during the history merge.

## Known issues / risks

- 0.1.0 (b26769a) confirmed by the user to boot and play correctly on real hardware. No log reviewed yet.
- `savefile()` in `mc.cpp` writes via `FullPath` into `app0:` (read-only). It is only used by the recorder/editor paths, not normal play.
- Touch-to-mouse emulation is still on (inherited from `SDL3Window`); to be revisited in Phase 2.
- Controls use the shared SDL3 gamepad mapping (Cross jump, Square action, Circle back, Start, Select back); Phase 2 will tune it.
- Display is 1:1 centred only; the 725x544 fill option is Phase 2.
- Audio: the game opens a 44.1 kHz float stream; SDL's Vita driver converts to S16 and, because the main port only accepts 48 kHz, uses the BGM port at 44.1 kHz. Speed/crackle unverified on hardware; the log records the device format.
- pl_mpeg FMV decoding speed on the Vita CPU is unverified; the game's existing MOVIES: ON/OFF option can skip them.
- `std::chrono::steady_clock` and `std::this_thread::sleep_for` drive the main loop timing; if they misbehave on Vita newlib, the frame rate lines in the log will show it.
