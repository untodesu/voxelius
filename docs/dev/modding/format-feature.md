# Features

A feature is an atomic world-generation template: a palette of blocks and relative part offsets. Features are placed by the engine as scatter decorators (via `biome.scatter`) or as structure pieces (future). Placing features from scripts is planned.

Stored features live under: `namespace:features`

Features are authored facing **north** (`blocks.FACE_NORTH`). Placement rules supply world origin and facing at runtime.

## Top-level fields

|Field|Type|Required|Default|Description|
|----|----|----|----|----|
|`palette`|`object[]`|no|`[]`|Block palette to use|
|`parts`|`object[]`|yes|N/D|Feature parts|

## Palette entries

```json
"palette": [
  {
    "block": "builtin:stone_slab",
    "states": {
      "orientation": "bottom"
    }
  },
  {
    "block": "builtin:grass"
  }
]
```

|Field|Type|Required|Default|Description|
|----|----|----|----|----|
|`block`|`string`|yes|N/D|Namespaced block ID|
|`states`|`object`|no|`{}`|Blockstate key-values|

## Parts entries

```json
"parts": [
  {
    "block": 0,
    "offset": [0, 1, 0],
    "overwrite": ["gas", "xfoil"]
  }
]
```

|Field|Type|Required|Default|Description|
|----|----|----|----|----|
|`block`|`integer`|yes|N/D|Palette entry index|
|`offset`|`integer[3]`|yes|N/D|Offset in blocks from the origin point|
|`overwrite`|`string[]`|no|`{}`|Block tags the feature can overwrite when they occupy the needed space|

### Tag values

|JSON|Lua constant|
|----|----|
|`"gas"`|`blocks.TAG_GAS`|
|`"rock"`|`blocks.TAG_ROCK`|
|`"soil"`|`blocks.TAG_SOIL`|
|`"turf"`|`blocks.TAG_TURF`|
|`"xfoil"`|`blocks.TAG_XFOIL`|
|`"sfoil"`|`blocks.TAG_SFOIL`|
|`"wood"`|`blocks.TAG_WOOD`|
