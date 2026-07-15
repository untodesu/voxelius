# Lua Base

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
|`collectgarbage`|Sandbox breaking|  

## Replaced globals

|Global|Overriden behaviour|  
|----|----|  
|`dofile`|Treats the input path as a namespaced identifier with the `scripts` subdirectory implicitly defined|  
|`require`|Inputs a namespaced ID with the namespace _required to be explicit_. Files are fetched from the `lib` directory|  
|`print`|Alias to uuLog's `LOG_INFO` macro|  
