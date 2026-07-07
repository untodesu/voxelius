# Modding: Lua API

## Core libraries

Voxelius's runtime provides common Lua APIs:  

|Table|Notes|  
|----|----|  
|`LUA_GNAME`| |  
|`LUA_TABLIBNAME`|May have some of its values stripped later in development|  
|`LUA_STRLIBNAME`| |  
|`LUA_MATHLIBNAME`| |  
|`LUA_UTF8LIBNAME`| |  

## Stripped globals

|Global|Reason|  
|----|----|  
|`loadfile`|Sandbox breaking|  
|`load`|Sandbox breaking|  
|`require`|TODO|  

## Replaced globals

|Global|Overriden behaviour|  
|----|----|  
|`dofile`|Treats the input path as a namespaced identifier with the `scripts` subdirectory implicitly defined|  
|`print`|Alias to uuLog's `LOG_INFO` macro|  

## Error handling

The mod's root script, `modname/scripts/init.lua` can handle errors itself, though if an error propagates out of it (into the initializing `lua_pcall`), the mod will be considered as a load fail.  
