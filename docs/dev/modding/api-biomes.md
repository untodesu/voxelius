# Biomes API

Mods can register new biome types for the world generator  

## Constants: limits

General engine-provided limits for reference  

|Name|Value|Description|  
|----|----|----|  
|`biomes.TEMP_MIN`|200|Minimum temperature, K|  
|`biomes.TEMP_MAX`|330|Maximum temperature, K|  
|`biomes.HUMID_MIN`|0|Minimum relative humidity, percent|  
|`biomes.HUMID_MAX`|100|Maximum relative humidity, percent|  
|`biomes.AXIS_MIN`|0|Third axis minimum, percent|  
|`biomes.AXIS_MAX`|100|Third axis maximum, percent|  

## Constants: realms

Each biome is categorized into a realm. Realms mainly differ from each other by depth and noise generation groups  

|Name|Description|  
|----|----|  
|`biomes.REALM_SURFACE`|Basic world surface|  
|`biomes.REALM_CAVE`|Basic underground caves|  
|`biomes.REALM_DEEP`|Deep underground layers and caves|  
|`biomes.REALM_SKY`|High up, floating islands and whatnot|  

## Lookup tables

Each biome defines its characteristics as nucleations points for a flood-fill lookup table. Each LUT is built per-realm and has three axes:  

- First axis is the nominal temperature in Kelvin degrees  

- Second axis is the relative humidity in percent  

- Third axis depends on the realm  

|Realm|Biome definition field|Descripton|  
|----|----|----|  
|`biomes.REALM_SURFACE`|`weirdness`|Terrain weirdness, 0 = regular, 100 = exotic|  
|`biomes.REALM_CAVE`|`depth`|Relative depth|  
|`biomes.REALM_DEEP`|`depth`|Relative depth|  
|`biomes.REALM_SKY`|`altitude`|Relative altitude|  

## Functions

### Function: `biomes.add(name, def) -> integer`

Registers a new biome  

#### Arguments

- `name` is the namespaced biome ID  

- `def` is the biome definition table  

#### Return value

Returns the numeric biome ID used for debugging  

## Biome definition

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`realm`|`integer`|yes|N/D|One of `biomes.REALM` values|  
|`temperature`|`integer`|yes|N/D|Nominal temperature nucleation point|  
|`humidity`|`integer`|yes|N/D|Relative humidity nucleation point|  
|`weirdness`|`integer`|depends|N/D|Required for `biomes.REALM_SURFACE`|  
|`depth`|`integer`|depends|N/D|Required for `biomes.REALM_CAVE` and `biomes.REALM_DEEP`|  
|`altitude`|`integer`|depends|N/D|Required for `biomes.REALM_SKY`|  
|`priority`|`integer`|no|0|In case of an equal flood-fill distance, the biome with higher priority wins|  
|`blocks`|`table`|no|`{}`|Block palette|  
|`strata`|`table`|no|`nil`|Explicit layers, overrides `blocks` logic|  
|`features`|`table`|no|`{}`|List of features to scatter|  

## Block palette

|Field|Description|  
|----|----|  
|`base`|Base block, eg. stone|  
|`filler`|Under-surface block, eg. dirt|  
|`surface`|Surface block, eg. grass|  
|`fluid`|Fluid block, eg. water|  
|`ceiling`|Ceiling block, eg. stone|  

## Strata example

```lua
strata = {
    { depth = -8, block = "builtin:stone" },
    { depth = -3, block = "builtin:dirt"  },
    { depth =  0, block = "builtin:grass" },
}
```

Depth is the offset from a reference level in blocks, where zero means the surface block level  

If strata is defined, `blocks` table is ignored  

## Feature table

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`id`|`string`|yes|N/D|Feature's namespaced ID|  
|`chance`|`number`|no|0.5|Chance of a placement attempt|  
|`requires_sky`|`boolean`|no|`false`|Does it need a `blocks.TAG_GAS` block above its origin?|  
|`requires_floor`|`boolean`|no|`false`|Does it need a solid floor under its origin?|  
|`min_depth`|`integer`|no|`nil`|Minimum depth required under the surface|  
|`max_depth`|`integer`|no|`nil`|Maximum depth required under the surface|  
|`min_altitude`|`integer`|no|`nil`|Minumum altitude for a sky realm|  
|`max_altitude`|`integer`|no|`nil`|Maximum altitude for a sky realm|  
