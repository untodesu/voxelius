# Features

Feature is a terrain generation primitive used to bring randomness into the world and to enhance gameplay. Features can currently be assigned to biomes to be randomly scattered around, and a way to place features from scripts is planned  

Stored features are located in the directory: `namespace:features`  

## Top-level fields

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`anchor`|`string`|yes|N/D|Feature anchor|  
|`palette`|`object[]`|no|`[]`|Block palette to use|  
|`parts`|`object[]`|yes|N/D|Feature parts|  

### Anchor values

Features have a starting point. These starting points need to be anchored onto something. The engine provides three modes of anchoring:  

|Name|Description|Image|    
|----|----|----|    
|`"surface"`|Feature's origin point is assumed to be a surface block (eg. grass)|![](anchor-surface.png)|    
|`"ceiling"`|Feature's origin point is assumed to be a ceiling block (eg. a cave ceiling)|![](anchor-ceiling.png)|  
|`"whatever"`|Feature is placed whereever the game decides is a good spot|![](anchor-whatever.png)|    

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
    "block": "bultin:stone_slab",
    "states": {
      "orientation": "top"
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
    "overwrite": ["gas", "foil"]
  }
]
```

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`block`|`integer`|yes|N/D|Palette entry index|  
|`offset`|`integer[3]`|yes|N/D|Offset in blocks from the origi point|  
|`overwrite`|`string[]`|no|`{}`|List of block tags the feature can and will overwrite if they occupy the space we need|  

### Tag values

|JSON|Lua constant|  
|----|----|  
|`"gas"`|`blocks.TAG_GAS`|  
|`"rock"`|`blocks.TAG_ROCK`|  
|`"soil"`|`blocks.TAG_SOIL`|  
|`"turf"`|`blocks.TAG_TURF`|  
|`"foil"`|`blocks.TAG_FOIL`|  
|`"wood"`|`blocks.TAG_WOOD`|  
