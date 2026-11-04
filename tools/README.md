# Isolated CE developer build

`build_zig.py` builds the **whole** CE DLL, including existing CE functionality
and the experimental district-score API. It never deploys or launches anything.
Use a separate output directory, not the source checkout or a game/mod directory.
Python and the compiler are developer tools; the resulting mod does not need them.

Supply explicit paths to Zig 0.15.2, MinHook 1.3.4, AsmJit source revision
`e7239626b8d5db1624faa08293fd470fe019cd88`, Capstone 5.0.3, and the installed
original GS GameCore. `dependencies.json` records acquisition identities and
license references. No dependency is downloaded by this script.

```
python tools/build_zig.py --zig /path/to/zig.exe --minhook /path/to/minhook-1.3.4 --asmjit /path/to/asmjit-e723962 --capstone /path/to/capstone-5.0.3 --gamecore /path/to/GameCore_XP2_FinalRelease.dll --out /separate/build-output
```

The script stages sources, restores only AsmJit files absent from CE's tracked
checkout (notably the omitted `x86/` backend), and builds existing CE plus MinHook,
Capstone's x86 backend, and the new native module. Existing CE-vendored AsmJit
core files take precedence. The restored upstream snapshot shares the same
1.13 ABI and 95 of CE's 97 tracked files match after newline normalization; CE's
two differing files are retained unchanged. Capstone is rebuilt from source with
its matching headers, not linked against an untracked binary library.

The resulting AMD64 PE must preserve all 157 original export names/ordinals;
155 remain forwarders, while Create/Destroy stay local wrappers. The original
GameCore is SHA-256 qualified but never copied or distributed. Receipts include
compiler identity, exact sources, commands, binary hash, exports and imports.
`--skip-native` is a compile-only development stage, not a working feature build.
Use a fresh output directory after deleting/renaming source files; staged copies
are incrementally refreshed and are not a general source-tree synchronizer.

The original MSVC project also includes the native C11 source, Bcrypt, and MASM
shim. That route still expects upstream CE's separately configured dependencies.
The portable build uses Win64 calling conventions at native C boundaries and
static Zig-supplied C++ runtime support. MSVC build compatibility and broader CE
compatibility require separate validation; a successful link is not gameplay
qualification. No C++ standard-library objects are passed across GameCore APIs.

Native hooks initialize once per process. Lua registration still happens for
each gameplay state, and numeric policy/diagnostic state resets on context
creation and before destruction. Hooks never invoke Lua on AI worker threads.
The supported target is the exact Windows GS original hash in the native
identity header. Unsupported builds leave this capability unavailable.
