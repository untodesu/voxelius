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

## Constant: null biome

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
|`priority`|`integer`|no|0|LUT nucleation conflicts are resolved with this value. If the biome has higher priority, the lower-priority nucleation points gets shifted in a random direction. If priorities are equal, the random one wins|  
|`palette`|`table`|no|`{}`|Biome's block palette|  
|`scatter`|`table`|no|`{}`|List of features to scatter|  

## Palette table

|Field|Description|  
|----|----|  
|`empty`|Empty block type, eg `builtin:air`|  
|`basic`|Basic block type, eg `builtin:stone`|  
|`filler`|Filler block type, eg `builtin:dirt`|  
|`surface`|Surface block type, eg `builtin:grass`|  
|`fluid`|Fluid block type, eg `builtin:water`|  

## Palette entry

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`name`|`string`|yes|N/D|Block name|  
|`states`|`table`|no|`{}`|Block states|  

## Scatter entry

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`feature`|`string`|yes|N/D|A feature to place|  
|`chance`|`number`|no|0.5|Chance of a placement attempt|  
|`need_above`|`table`|no|`{}`|List of `blocks.TAG_XXXX` constants. When checking if a scatter entry is qualified for placement, generator checks if the block above the origin has at least one of the listed tags|  
|`need_below`|`table`|no|`{}`|List of `blocks.TAG_XXXX` constants. When checking if a scatter entry is qualified for placement, generator checks if the block below the origin has at least one of the listed tags|  

## Example

```lua
biomes.add("plains", {
  realm = biomes.REALM_SURFACE,
  lut_temp = 50,
  lut_humd = 50,
  lut_axis = 50,
  priority = 0,

  palette = {
    empty = { name = "air" },
    basic = { name = "stone" },
    filler = { name = "dirt" },
    surface = { name = "stone_slab", states = { orientation = "bottom" } },
    fluid = { name = "water" },
  },

  scatter = {
    {
      feature = "oak_tree_01",
      chance = 0.0625,
      need_above = { blocks.TAG_GAS },
      need_below = { blocks.TAG_TURF },
    },
    {
      feature = "bush",
      chance = 0.25,
      need_above = { blocks.TAG_GAS },
      need_below = { blocks.TAG_TURF },
    }
  }
})
```

## Biome LUTs

Every biome defines three LUT axes - temperature, humidity and an extra. These values are sampled from noise during terrain generation and based on their values a biome is chosen. For optimization purposes, the engine builds a 3D lookup table (a matrix?) of biome IDs for each LUT value.  

> **NOTE:** Voxelius uses 32-bit integers as biome IDs so a single lookup table for a realm is about 3.82 MiB. Considering current realm amount and possible expansion in the future, it probably won't run on your 486...  

A [Jump-Flooding Algorithm](https://en.wikipedia.org/wiki/Jump_flooding_algorithm) is used to "fill in" the gaps. It generates map that suspiciously looks like a Voronoi map, so there's somewhat of a "smooth" border between biomes.

> **NOTE:** probably a big TODO, but it would be cool if I also added a special "border" biome kind for all realms to prevent sharp biome borders. Kind of like old Minecraft was using rivers to split biomes apart, just much more data-driven  

![jfa-voronoi.gif](jfa-voronoi.gif)  

Think of each color as a separate biome type - you start with a discrete "nucleation points" for biomes defined by Lua scripts and for a number of steps you extrapolate, which range of parameters a biome occupies   
