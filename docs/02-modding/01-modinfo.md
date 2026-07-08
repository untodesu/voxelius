# Modding: mod metadata

Every mod's root directory contains a `modinfo.conf` file that contains some useful information about the mod. The file uses the common configuration format used by the engine (see `core/config/map.cc`);

## Fields

|Name|Required|Default|Description|  
|----|----|----|----|  
|`name`|yes|N/D|The mod's name/ID used for namespacing|  
|`version`|yes|`0.0.0`|The mod's semantic version used for compatibility checks|  
|`hard_depends`|no|N/D|Whitespace-separated list of hard dependencies|  
|`soft_depends`|no|N/D|Whitespace-separated list of soft dependencies|  
|`conflicts`|no|N/D|Whitespace-separated list of conflicting mods|  
|`meta_author`|no|N/D|Metadata: the mod's author/authors|  
|`meta_homepage`|no|N/D|Metadata: the mod's homepage URL|  
|`meta_tracker`|no|N/D|Metadata: the mod's issue tracker URL|  
|`meta_license`|no|`ARR`|Metadata: the mod's license as a short SPDX identifier|  
|`display_name`|no|Value of `name`|Metadata: the mod's display name|  
|`display_desc`|no|N/D|Metadata: a short description|  

> **NOTE:** all mods implicitly depend on `builtin` of the version the game runtime provides (see core/version.cc)  

## Example

```ini
name = kaboom
version = 1.0.0

hard_depends = testmod@2.5.0
soft_depends = extratools fire@1.0.0
conflicts = mclike_tnt@0.1.0

meta_author = untodesu
meta_homepage = https://untode.su/
meta_tracker = https://untode.su/
meta_license = BSD-3-Clause

display_name = Ka-Boom!
display_desc = Adds explosive blocks to Voxelius
```

## Load order

Mods are ordered by a topological sort of dependencies. Within a single "group" of mods in that list, things are loaded in an alphabetical order. Mods without dependencies (ie only depending on `builtin`) are loaded first.  

> **NOTE:** non-existent dependencies cause the mod to fail  
