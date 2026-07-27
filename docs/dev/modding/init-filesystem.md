# Filesystem

## Mod root

Each mod may use these directories:

|Directory|Description|
|----|----|
|`collisions`|Collision models for blocks and entities|
|`features`|World generation feature templates|
|`fonts`|Fonts in binary form. Currently used only in `builtin`. Font paths are hard-coded in the engine|
|`lang`|Translation files|
|`lib`|Lua libraries for `require`|
|`misc`|Miscellaneous files|
|`models`|3D models for blocks and entities|
|`music`|Music streamed from disk|
|`scripts`|Lua scripts|
|`shaders`|GLSL shader sources. Currently used only in `builtin`. Shader paths are hard-coded in the engine|
|`sounds`|Sound effects|
|`textures`|Textures|

## Virtual filesystem

The engine uses PhysFS for the VFS. It mounts these system paths:

|Mount|Path|Access|Description|
|----|----|----|----|
|`/`|`${PWD}/data`|Read-only|Vendored mods, including `builtin`|
|`/`|`${PWD}/mods`|Read-only|User-provided mods|
|`/`|`${APPDATA}/Voxelius` or similar|Read/Write|User directory. Settings and world saves live here. The user can also override vendored assets|

All three mount at virtual root. Mods are appended. The user directory is prepended. That sets priority: the user directory _always_ overrides the game directory, and the game directory overrides mods.
