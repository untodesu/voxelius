# Features API

Feature is a terrain generation primitive used to bring randomness into the world and to enhance gameplay. Features can currently be assigned to biomes to be randomly scattered around, and a way to place features from scripts is planned  

## Constants: anchor

Features have a starting point. These starting points need to be anchored onto something. The engine provides three modes of anchoring:  

|Name|Description|Image|    
|----|----|----|    
|`features.ANCHOR_SURFACE`|Feature's origin point is assumed to be a surface block (eg. grass)|![](anchor-surface.png)|    
|`features.ANCHOR_CEILING`|Feature's origin point is assumed to be a ceiling block (eg. a cave ceiling)|![](anchor-ceiling.png)|  
|`features.ANCHOR_WHATEVER`|Feature is placed whereever the game decides is a good spot|![](anchor-whatever.png)|    

## Constant: null feature

The `features.NULL_FEATURE` constant defines an empty, undefined or otherwise invalid feature ID. This is only used for debugging  

## Functions

### Function: `features.add(name, def) -> integer`

Register a new feature in the registry  

#### Arguments

- `name` is a namespaced ID of a feature, eg `builtin:oak_tree`  
- `def` is a feature definition table, see below for that  

#### Return value

Numeric feature ID or `features.NULL_FEATURE` if the game didn't like your code  

#### Notes

**Rename rule:** on conflict, the encroaching `name` is suffixed with `~N`, where `N` is the smallest integer starting at `1` that produces a free id (e.g. `mymod:myfeature~1`). This keeps loading deterministic and reproducible across runs with the same mod list/order.

## Feature definition

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`anchor`|`integer`|yes|N/D|One of `features.ANCHOR_XXXX` constants|  
|`palette`|`table[]`|no|`{}`|Block palette to use|  
|`parts`|`table[]`|yes|N/D|Feature parts|  

### Palette entries

```lua
palette = {
  {
    block = "builtin:stone_slab",
    states = {
      orientation = "bottom"
    }
  },
  {
    block = "builtin:stone_slab",
    states = {
      orientation = "top"
    }
  },
  {
    block = "builtin:grass"
  }
}
```

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`block`|`string`|yes|N/D|Numeric block ID|  
|`states`|`table`|no|`{}`|Blockstate key-vales|  

### Parts entries

```lua
parts = {
  {
    block = 0,
    offset = { 0, 1, 0 },
    overwrite = { blocks.TAG_GAS, blocks.TAG_FOIL }
  }
}
```

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`block`|`integer`|yes|N/D|Palette entry index|  
|`offset`|`integer[3]`|yes|N/D|Offset in blocks from the origin point|  
|`overwrite`|`integer[]`|no|`{}`|List of block tags the feature can and will overwrite if they occupy the space we need|  
