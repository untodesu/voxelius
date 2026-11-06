# Build Guide

The build uses CMake presets. A preset names the platform, the configuration and, when the name ends in `-prof`, profiling. Each preset configures into its own directory under `build/`.

The presets use schema version 6, so the build needs CMake 3.25 or newer. The [Requirements](init-requirements.md) page lists the compilers and the other tools.

## Presets

|Preset|Platform|Configuration|
|----|----|----|
|`generic-debug`|Any POSIX host|Debug|
|`generic-release`|Any POSIX host|Release|
|`windows-x32-debug`|Windows x86|Debug|
|`windows-x32-release`|Windows x86|Release|
|`windows-x64-debug`|Windows x64|Debug|
|`windows-x64-release`|Windows x64|Release|

Each of these presets has a `-prof` variant, for example `generic-release-prof` or `windows-x64-debug-prof`.

A preset exists only on its platform. Run `cmake --list-presets` to see the presets that the current host can use.

## Building

Configure first, then build. Both commands take the same preset name:

```sh
cmake --preset windows-x64-debug
cmake --build --preset windows-x64-debug
```

Add `--parallel` to the build command to use every core: `cmake --build --preset windows-x64-debug --parallel`.

The executables land in `build/<preset>/`. The Windows presets use a multi-configuration generator, so the configuration comes first: `build/windows-x64-debug/Debug/vclient.exe`.

> **NOTE:** CMake picks the generator. On Windows it is Visual Studio, on a POSIX host it is usually Ninja or GNU make. The Visual Studio generator writes no `compile_commands.json`. The [Editor Setup](init-editor.md) page explains what clangd needs.

## The scripts

The scripts in `scripts/` wrap one preset each. They configure and then build, from the repository root:

|Target|Command|
|----|----|
|Windows x64, debug|`scripts/build-windows-x64-debug.bat`|
|Windows x64, release|`scripts/build-windows-x64-release.bat`|
|Windows x86, debug|`scripts/build-windows-x32-debug.bat`|
|Windows x86, release|`scripts/build-windows-x32-release.bat`|
|Any POSIX host, debug|`scripts/build-generic-debug.sh`|
|Any POSIX host, release|`scripts/build-generic-release.sh`|

Each target also has a `-prof` script, for example `scripts/build-windows-x64-debug-prof.bat` or `scripts/build-generic-release-prof.sh`.

## Profiling

A `-prof` preset defines `TRACY_ENABLE`, `TRACY_ON_DEMAND` and `TRACY_NO_FRAME_IMAGE`. The game then records the profiling zones and waits for the Tracy profiler to connect. A normal preset defines none of these macros, so the zones cost nothing in a build that ships.

Do not ship a `-prof` build.

## Without a preset

A preset covers the usual targets. For another generator or another configuration, configure by hand and keep the build directory under `build/`:

```sh
cmake -S . -B build/my-build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/my-build --parallel
```

The Ninja and GNU make generators write `build/my-build/compile_commands.json`. The [Editor Setup](init-editor.md) page explains how clangd finds it.
