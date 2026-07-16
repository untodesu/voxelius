# Features

Features define templates the world generator scatters into biomes  

Features are located in the directory: `namespace:features`  

## Top-level fields

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`anchor`|`string`|no|`"surface"`|Where the feature origin is anchored before `parts` offsets are applied|  
|`palette`|`string[]`|yes|N/D|Block palette for the feature, a list of full namespaced block IDs. Each _requires_ the namespace|  
|`parts`|`object[]`|yes|N/D|List of blocks to place|  

The `anchor` field can be one of the following values:  

- `surface` marks the feature as anchoring to a column surface  

- `floor` marks the feature as anchoring to a floor of a cave  

- `ceiling` marks the feature as anchoring to a ceiling of a cave or bottom of a floating island  

## Part object

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`overwrite`|`string[]`|no|`[]`|Defines a list of block tags that should be overwritten when encountered during placement|  
|`offset`|`integer[3]`|yes|N/D|Block offset from the anchor|  
|`block`|`integer`|yes|N/D|Index into the block palette|  

### Overwrite to block tag matching

|Value|`blocks.TAG` constant|  
|----|----|  
|`gas`|`blocks.TAG_GAS`|  
|`rock`|`blocks.TAG_ROCK`|  
|`soil`|`blocks.TAG_SOIL`|  
|`turf`|`blocks.TAG_TURF`|  
|`foil`|`blocks.TAG_FOIL`|  
|`wood`|`blocks.TAG_WOOD`|  

## Notes

- Parts are evaluated in the order they are declared in the file  

- A part always replaces null/void blocks  

- Parts may cross chunk boundaries  

## Example: bush

A single block feature anchored to the surface  

```json
{
  "anchor": "surface",
  "palette": ["builtin:bush"],
  "parts": [
    {
        "overwrite": ["gas"],
        "offset": [0, 0, 0],
        "block": 0
    }
  ]
}
```
