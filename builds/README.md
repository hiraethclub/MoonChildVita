# Vita builds

GitHub rejects files over 100 MB, so each VPK is split into parts. Join them on a PC, then copy the `.vpk` to the Vita and install it with VitaShell.

Windows (Command Prompt, in this folder):

    copy /b MoonChildFE-vita-0.1.0-b26769a.vpk.part000 + MoonChildFE-vita-0.1.0-b26769a.vpk.part001 MoonChildFE-vita-0.1.0-b26769a.vpk

macOS / Linux:

    cat MoonChildFE-vita-0.1.0-b26769a.vpk.part* > MoonChildFE-vita-0.1.0-b26769a.vpk

Check the result against the `.sha256` file (`certutil -hashfile <file> SHA256` on Windows, `sha256sum -c *.sha256` elsewhere).
