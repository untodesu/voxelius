# Biomes API

Mods can register new biome types for the world generator

## Constants: realms

Because Voxelius defines a single "dimension" for the world, the vertical range of blocks can be utilized. Each biome is categorized into a realm. Realms are horizontal slices of the world assumed to have a specific terrain generation model.  

|Name|Y-Range|Description|  
|----|----|----|  
|`biomes.REALM_SKY`|`256..767`|Rare, large and ore-rich floating islands above highest surface points|  
|`biomes.REALM_SURFACE`|`-64..255`|Common terrain and biomes. Players and most playable structures spawn there|  
|`biomes.REALM_UNDERGROUND`|`-256..-63`|Classic cave systems, underground rivers and generally good ore amounts|  
|`biomes.REALM_THE_DEPTHS`|`-512..-255`|Vast and very dark caverns, quite ore-rich if players take risks|  

## Cosntant: null biome

The `biomes.NULL_BIOME` constant defines an empty, undefined or otherwise invalid biome ID. Gameplay/gamedev-wise it can be treated as a void biome  

## Functions

### Function: `biomes.add(name, def) -> integer`

Registers a new biome  

#### Arguments

- `name` is the namespaced biome ID  

- `def` is the biome definition table  

#### Return value

Returns the numeric biome ID

## Biome definition

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`realm`|`integer`|yes|N/D|One of `biomes.REALM_XXXX` constants|  
|`lut_temp`|`integer`|no|50|Nominal biome temperature, ranging from 0 to 99|  
|`lut_humd`|`integer`|no|50|Nominal biome humidity, ranging from 0 to 99|  
|`lut_axis`|`integer`|no|50|Realm-dependent LUT axis, ranging from 0 to 99|  
|`priority`|`integer`|no|0|Flood-fill conflicts are resolved using this one|  
|`palette`|`table`|no|`{}`|Biome's block palette|  
|`scatter`|`table`|no|`{}`|List of features to scatter|  

## Palette table

|Field|Description|  
|----|----|  
|`empty`|Empty block type, eg `builtin:air`|  
|`basic`|Basic block type, eg `builtin:stone`|  
|`filler`|Filler block type, eg `builtin:dirt`|  
|`surface`|Surface block type, eg `builtin:grass`|  
|`ceiling`|Ceiling block type, eg `builtin:roots`|  
|`fluid`|Fluid block type, eg `builtin:water`|  

## Scatter table

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`name`|`string`|yes|N/D|Feature's namespaced ID|  
|`chance`|`number`|no|0.5|Chance of a placement attempt|  
|`need_gas`|`boolean`|no|`false`|Whether placement must have a block with the `blocks.TAG_GAS` tag above the origin|  
|`need_floor`|`boolean`|no|`false`|Whether placement must have a solid block under the origin|  
