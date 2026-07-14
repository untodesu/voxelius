# Getting Started: Filesystem

## Mod root

Each mod can have the following directories  

|Directory|Description|  
|----|----|  
|`collisions`|Collision models for blocks and entities|  
|`fonts`|Fonts in binary form. Currently only used in `builtin` and have font paths hard-coded inside the engine|  
|`lang`|Translation files|  
|`misc`|Miscellaneous files|  
|`models`|3D models for blocks and entities|  
|`music`|Music that's streamed from disk|  
|`scripts`|Lua scripts|  
|`shaders`|GLSL shader sources. Currently only used in `builtin` and have shader paths hard-coded inside the engine|  
|`sounds`|Sound effects|  
|`textures`|Textures|  

## Virtual filesystem

The engine uses PhysFS for VFS and uses the following system paths:  

|Mountpoint|Path|Access|Description|  
|----|----|----|----|  
|`/`|`${PWD}/data`|Read-only|Vendored mods, including `builtin`|  
|`/`|`${PWD}/mods`|Read-only|Used-provided mods|  
|`/`|`${APPDATA}/Voxelius` or similar|Read/Write|User directory. The game keeps settings and world saves there and the user can override vendored assets within it|  

All three are mounted as virtual root with mods being appended to the list and user directory prepended; this defines the priority - user directory _always_ overrides _anything_ defined by game directory, which overrides _anything_ defined by mods directory.  
