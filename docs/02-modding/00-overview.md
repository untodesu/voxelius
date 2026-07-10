# Modding: overview

Starting with the project's 17th rewrite (this is a joke, though it's like the third major rewrite in the game's history dating back to 2021), Voxelius _the game_ builds on top of Voxelius _the engine_, which provides a bunch of Lua hooks to define blocks and other stuff that enhances core gameplay

## Namespaced identifiers

Voxelius's engine uses _namespaced identifiers_ akin to what Minecraft uses. A namespaced identifier consists of, well... a namespace, and, you're not going to believe it, an identifier.  

```
[<namespace>:]<identifier>
```

Namespace can be optional. When omitted in a mod's scripts, it defaults to the mod's namespace, otherwise it uses `builtin` (the quote-unquote "mod" ID associated with the base Voxelius game) for the namespace.  

Valid characters in both namespace and identifier include:

- Latin alphabet: `A-Z` and `a-z` ASCII range;  
- Numeric values: `0-9` ASCII range;  
- Underscore: `_` ASCII character;  
- Tilde: `~` ASCII character. Not recommended for general use, see below;  

During initialization, if the engine sees fully duplicate entries in a registry (eg. `mymod:myblock` is registered twice), it will append `~N` to the end and will try until it hits 10000 renames at which point I am not promising any defined behaviour...  

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

## Error handling

The mod's root script, `modname/scripts/init.lua` can handle errors itself, though if an error propagates out of it (into the initializing `lua_pcall`), the mod will be considered as a load fail.  
