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
|`temperature`|`integer`|no|50|Target temperature in climate space, ranging from 0 to 99|  
|`humidity`|`integer`|no|50|Target humidity in climate space, ranging from 0 to 99|  
|`continentalness`|`integer`|no|50|Target continentalness, ranging from 0 to 99|  
|`weirdness`|`integer`|no|50|Target weirdness / relief, ranging from 0 to 99|  
|`priority`|`integer`|no|0|When two biomes claim the same climate target during setup, higher priority keeps the point; the loser is nudged until unique|  
|`offset`|`number`|no|0|Added to climate distance when picking a biome. Larger values make the biome harder to win / effectively smaller|  
|`palette`|`table`|no|`{}`|Biome's block palette|  
|`scatter`|`table[]`|no|`{}`|List of features to scatter|  

## Palette table

|Field|Description|  
|----|----|  
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
|`need_above`|`integer[]`|no|`{}`|List of `blocks.TAG_XXXX` constants. When checking if a scatter entry is qualified for placement, generator checks if the block above the origin has at least one of the listed tags|  
|`need_below`|`integer[]`|no|`{}`|List of `blocks.TAG_XXXX` constants. When checking if a scatter entry is qualified for placement, generator checks if the block below the origin has at least one of the listed tags|  

## Example

```lua
biomes.add("plains", {
  realm = biomes.REALM_SURFACE,
  temperature = 50,
  humidity = 50,
  continentalness = 55,
  weirdness = 35,
  priority = 0,
  offset = 0,

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

## Climate / multi-noise

Biome placement uses a sparse multi-noise model; at generation time, the engine samples four continious climate noises (temperature, humidity, continentalness and weirdness) and picks the biome which target point in that space is nearest  

During mod loading, each biome defines a nucleation point at its target coordinates (0..99 per axis). If two biomes ever collide, higher `priority` keeps the cell and the loser is nudged randomly until it gets a free slot

### Notes: `biomes.REALM_SURFACE`

For the surface realm, the same continious noises also drive the terrain shape: weirdness scales density amplitude and continentalness shifts the base height, making ocenas and inland areas separate  
