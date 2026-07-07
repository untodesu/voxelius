# Modding: core API

The engine provides limited access to its subsystems and constants using `core` library.

## Version constants

Specifies the game/engine version. See `core/version.hh` for the C++ equivalent.  

|Value|Example|Description|  
|----|----|----|  
|`core.VERSION_MAJOR`|`16`|Major version number|  
|`core.VERSION_MINOR`|`0`|Minor version number|  
|`core.VERSION_PATCH`|`3`|Patch version number|  
|`core.VERSION_SCM_BRANCH`|`main`|SCM branch name|  
|`core.VERSION_SCM_REVISION`|`deadbeef`|SCM revision aka git commit hash|  
|`core.VERSION_SEMANTIC`|`16.0.3`|Semantic version string|  
|`core.VERSION`|`16.0.3-main [deadbeef]`|Full build-system-defined version string|  

## Current namespace

As mentioned in [00-introduction](00-introduction.md), namespaced identifiers can have the namespace omitted and be replaced with a default value. That value for mods is also contained in the `core.NAMESPACE` constant.  

## Configuration access

Game configuration (`client.conf` and `server.conf` on client and server respectively) can be accessed and modified using the following set of functions:  

### Function: `core.config_has(name) -> boolean`

#### Arguments

- `name` is the configuration value name;  

#### Return value

True when the value is present, false otherwise  

### Function: `core.config_get(name) -> string`

#### Arguments

- `name` is the configuration value name;  

#### Return value

Config variable's value, `nil` if not present  

### Function: `core.config_set(name, value)`

#### Arguments

- `name` is the configuration value name;  
- `value` is the new value;  

