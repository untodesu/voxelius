# Lua Base

The Voxelius runtime exposes these common Lua APIs:

|Table|Notes|
|----|----|
|`LUA_GNAME`| |
|`LUA_TABLIBNAME`|Some values may be stripped later in development|
|`LUA_STRLIBNAME`| |
|`LUA_MATHLIBNAME`| |
|`LUA_UTF8LIBNAME`| |

## Stripped globals

|Global|Reason|
|----|----|
|`loadfile`|Sandbox breaking|
|`load`|Sandbox breaking|
|`collectgarbage`|Sandbox breaking|

## Replaced globals

|Global|Overridden behaviour|
|----|----|
|`dofile`|Treats the input path as a namespaced identifier. The `scripts` subdirectory is implied|
|`require`|Takes a namespaced ID. The namespace _must_ be explicit. Files come from the `lib` directory|
|`print`|Alias to the uuLog `LOG_INFO` macro|
