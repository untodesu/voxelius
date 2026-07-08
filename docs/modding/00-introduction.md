# Modding: introduction

## Namespaced identifiers

Voxelius's engine uses _namespaced identifiers_ akin to what Minecraft uses. A namespaced identifier consists of, well... a namespace, and, you're not going to believe it, an identifier.  

```
[<namespace>:]<identifier>
```

Namespace can be optional. When omitted in a mod's scripts, it defaults to the mod's namespace, otherwise it uses `builtin` (the quote-unquote "mod" ID associated with the base Voxelius game) for the namespace.  

## Filesystem

Generally speaking, runtime accesses only four locations:  

|Name|Example path|Description|  
|----|----|----|  
|Game directory|`${PWD}/data`|Read-only game assets|  
|User directory|`${APPDATA}/Voxelius`|User-specified game assets|  
|Modifications|`${PWD}/mods`|Modifications|  

All three are mounted as virtual root with mods being appended to the list and user directory prepended; this defines the priority - user directory _always_ overrides _anything_ defined by game directory, which overrides _anything_ defined by mods directory.  

Most of the time, files in that system are accessed using namespaced identifiers, so most of the content you'd see in all the directories is arranged into something of this sort:  

|Identifier|Resolved path|Notes|  
|----|----|----|  
|`builtin:scripts/init.lua`|`data/builtin/scripts/init.lua`| |  
|`builtin:block/stone01.png`|`data/builtin/textures/block/stone01.png`|The subdirectory can be omitted if C++ code decides to pass it instead, so the identifiers in stuff like block definitions are much shorter to read|  
|`builtin:cube`|`data/builtin/models/block/cube.json`|Same here|  
|`coolmod:init.lua`|`mods/coolmod/scripts/init.lua`|Whatever custom content there is|  
