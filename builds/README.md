# Vita builds

GitHub rejects files over 100 MB, so each VPK is split into parts. Join them on a PC, then copy the `.vpk` to the Vita and install it with VitaShell. Replace `NAME` below with the file name without `.partNNN`, for example `MoonChildFE-vita-0.3.0-19aaeee.vpk`.

Windows (Command Prompt, in this folder):

    copy /b NAME.part000 + NAME.part001 NAME

macOS / Linux:

    cat NAME.part* > NAME

Check the result against the `.sha256` file (`certutil -hashfile NAME SHA256` on Windows, `sha256sum -c NAME.sha256` elsewhere).

Older builds are in the git history of this folder. Every push also builds the VPK in GitHub Actions (artifact `MoonChildFE-Vita`), which needs no joining.
