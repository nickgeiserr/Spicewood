# Ghidra: import Dumper-7 names and types

`ImportDumper7IdMap.java` loads the `.idmap` file Dumper-7 writes to `IDAMappings/` into Ghidra.

It names:
- ~12,800 native exec functions (`AActor::execAddComponent`, ...) with the UFunction's C++ signature as a comment
- ~6,200 vtables (`UObject_VFT`, ...)
- the engine globals: `GObjects`, `GNames`, `UObject::ProcessEvent`, `FName::AppendString`

and optionally creates every reflected struct and enum as data types under `/Dumper7`.

Names you've set yourself are never overwritten, so it's safe to rerun after a redump.

## Setup (once)

Copy `ImportDumper7IdMap.java` into `%USERPROFILE%\ghidra_scripts`. Ghidra picks up scripts from there automatically.

## Running it

1. Open the program in the CodeBrowser.
2. **Window > Script Manager**, search for `ImportDumper7IdMap` (category *Unreal*), and run it.
   It's also under **Tools > Unreal > Import Dumper-7 IDMap**.
3. Pick the `.idmap` file, e.g. `C:\Dumper-7\<version>\IDAMappings\<version>.idmap`.
4. Choose whether to import types too. Names take seconds, types take a few minutes.

## If it says a lot of addresses are "not in memory"

Some process dumpers write a wrong `VirtualSize` for `.text` in the dump's PE header, so Ghidra only
loads part of the code. For the MCD2 dump, `.text` said `0x3A98000` but really runs to `0x812D000`,
so over half the game's code was missing. The fix is a copy of the dump with `.text`'s VirtualSize set
to (next section's address - `.text`'s address), imported fresh.

## If almost nothing gets named

The file's addresses are relative to the image base. For a memory dump, the program's image base
has to be the module base the dump was taken at (the dump's file name usually contains it, like
`_7FF72D0A0000_`). Fix it in **Window > Memory Map > Set Image Base** and run the script again.
