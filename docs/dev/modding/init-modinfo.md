# Mod Metadata

Every mod root directory contains a `modinfo.conf` file. The file uses the common configuration format of the engine.

## Fields

|Name|Required|Default|Description|
|----|----|----|----|
|`name`|yes|N/D|Mod name/ID used for namespacing|
|`version`|yes|`0.0.0`|Semantic version used for compatibility checks|
|`hard_depends`|no|N/D|Whitespace-separated list of hard dependencies|
|`soft_depends`|no|N/D|Whitespace-separated list of soft dependencies|
|`conflicts`|no|N/D|Whitespace-separated list of conflicting mods|
|`meta_author`|no|N/D|Metadata: author or authors|
|`meta_homepage`|no|N/D|Metadata: homepage URL|
|`meta_tracker`|no|N/D|Metadata: issue tracker URL|
|`meta_license`|no|`ARR`|Metadata: license as a short SPDX identifier|
|`display_name`|no|Value of `name`|Metadata: display name|
|`display_desc`|no|N/D|Metadata: short description|

> **NOTE:** every mod implicitly depends on `builtin` at the version the game runtime ships (see `core/version.cc`).

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

The engine orders mods by a topological sort of their dependencies. Within one group in that list, mods load in alphabetical order. Mods with no dependencies beyond `builtin` load first.

> **NOTE:** a missing dependency causes the mod to fail.
