# Filesystem

## Mod root

A mod's root directory is `data/<namespace>` in the virtual filesystem, where `<namespace>` is the mod's name. Each mod may use these directories:

|Directory|Description|
|----|----|
|`collisions`|Collision models for blocks and entities|
|`features`|World generation feature templates|
|`fonts`|Fonts in binary form. Currently the engine uses them only in `builtin`. Font paths are hard-coded in the engine|
|`lang`|Translation files|
|`lib`|Lua libraries for `require`|
|`misc`|Miscellaneous files|
|`models`|3D models for blocks and entities|
|`music`|Music streamed from disk|
|`scripts`|Lua scripts|
|`shaders`|GLSL shader sources. Currently the engine uses them only in `builtin`. Shader paths are hard-coded in the engine|
|`sounds`|Sound effects|
|`textures`|Textures|

## Virtual filesystem

The engine uses PhysFS for the VFS. It mounts these system paths:

|Mount|Path|Access|Description|
|----|----|----|----|
|`/data`|`${PWD}/data`|Read-only|Vendored mods, including `builtin`|
|`/data`|`${APPDATA}/Voxelius/mods` or similar|Read-only|User-provided mods|
|`/`|`${APPDATA}/Voxelius` or similar|Read/Write|User directory. Settings and world saves live here|
