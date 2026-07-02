# Modding: mods manifest

Every mod's root directory contains a `modinfo.conf` file that contains some useful information about the mod. The file uses the common configuration format used by the engine (see `core/config/map.cc`);

## Fields

|Name|Required|Default|Description|  
|----|----|----|----|  
|`name`|yes|N/D|The mod's name/ID used for namespacing|  
|`author`|no|N/D|Metadata: the mod's author/authors|  
|`homepage`|no|N/D|Metadata: the mod's homepage URL|  
|`tracker`|no|N/D|Metadata: the mod's issue tracker URL|  
|`license`|no|`ARR`|Metadata: the mod's license as a short SPDX identifier|  
|`display`|no|Value of `name`|Metadata: the mod's display name|  
|`description`|no|N/D|Metadata: a short description|  
|`depends`|no|N/D|Comma-separated list of mod IDs that must be in loaded state before this mod's scripts are invoked|  

> **NOTE:** all mods implicitly depend on `builtin`  

## Example

```ini
name = kaboom
depends = extratools,fire

author = untodesu
homepage = https://untode.su/
tracker = https://untode.su/
license = BSD-3-Clause
display = Ka-Boom!
description = Adds explosive blocks to Voxelius
```

## Load order

Mods are ordered by a topological sort of dependencies. Within a single "group" of mods in that list, things are loaded in an alphabetical order. Mods without dependencies (ie only depending on `builtin`) are loaded first.  

> **NOTE:** dependency cycles cause the mod to fail  

> **NOTE:** non-existent dependencies cause the mod to fail  
