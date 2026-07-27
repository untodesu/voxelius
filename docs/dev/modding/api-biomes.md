# Biomes API

Mods can register biome types for the world generator.

## Constants: realms

Voxelius uses a single world "dimension". Vertical range is split into realms instead. Each biome belongs to a realm. A realm is a horizontal slice of the world with its own terrain generation model.

|Name|Y-Range|Description|
|----|----|----|
|`biomes.REALM_SKY`|`256..767`|Rare, large, ore-rich floating islands above the highest surface points|
|`biomes.REALM_SURFACE`|`-64..255`|Common terrain and biomes. Players and most playable structures spawn here|
|`biomes.REALM_UNDERGROUND`|`-256..-63`|Classic cave systems, underground rivers, and solid ore amounts|
|`biomes.REALM_THE_DEPTHS`|`-512..-255`|Vast dark caverns. Ore-rich if players take risks|

## Constant: null biome

`biomes.NULL_BIOME` is an empty, undefined, or otherwise invalid biome ID. Treat it as a void biome for gameplay and gamedev.

## Functions

### Function: `biomes.add(name, def) -> integer`

Registers a new biome.

#### Arguments

- `name` is the namespaced biome ID
- `def` is the biome definition table

#### Return value

Returns the numeric biome ID.

## Biome definition

|Field|Type|Required|Default|Description|
|----|----|----|----|----|
|`realm`|`integer`|yes|N/D|One of `biomes.REALM_XXXX` constants|
|`temperature`|`integer`|no|50|Target temperature in climate space, 0 to 99|
|`humidity`|`integer`|no|50|Target humidity in climate space, 0 to 99|
|`continentalness`|`integer`|no|50|Target continentalness, 0 to 99|
|`weirdness`|`integer`|no|50|Target weirdness / relief, 0 to 99|
|`priority`|`integer`|no|0|If two biomes claim the same climate target during setup, higher priority keeps the point. The loser is nudged until the point is unique|
|`offset`|`number`|no|0|Added to climate distance when picking a biome. Larger values make the biome harder to win / effectively smaller|
|`palette`|`table`|no|`{}`|Biome block palette|
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
|`feature`|`string`|yes|N/D|Feature to place|
|`chance`|`number`|no|0.5|Chance of a placement attempt|
|`need_above`|`integer[]`|no|`{}`|List of `blocks.TAG_XXXX` constants. The block above the origin must have at least one listed tag|
|`need_below`|`integer[]`|no|`{}`|List of `blocks.TAG_XXXX` constants. The block below the origin must have at least one listed tag|

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

Biome placement uses a sparse multi-noise model. At generation time the engine samples four continuous climate noises (temperature, humidity, continentalness, and weirdness). It picks the biome whose target point in that space is nearest.

During mod loading, each biome defines a nucleation point at its target coordinates (0..99 per axis). If two biomes collide, higher `priority` keeps the cell. The loser is nudged randomly until it gets a free slot.

### Notes: `biomes.REALM_SURFACE`

For the surface realm, the same continuous noises also drive terrain shape. Weirdness scales density amplitude. Continentalness shifts base height, so oceans and inland areas stay separate.
