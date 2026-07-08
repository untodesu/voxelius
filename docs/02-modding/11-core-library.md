# Modding: core library

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

As mentioned in the [overview](00-overview.md), namespaced identifiers can have the namespace omitted and be replaced with a default value. That value for mods is also contained in the `core.NAMESPACE` constant.  
