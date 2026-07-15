# Block Models

Block models define the visible geometry of a block  

Block models are located in the directory: `namespace:models/block`  

> **NOTE:** textures _names_ are not part of the model. The model only declares named slots that it needs (eg. `top`, `north`, `nwse`, etc); the actual texture files are supplied per-block using `textures` field during registration  

> **NOTE:** if the slot is missing, `default` slot is used instead

## Top-level fields

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`textures`|`string[]`|yes|N/D|List of texture slot names|  
|`elements`|`object[]`|yes|N/D|List of cuboids that make the model|  

## Cuboid object

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`min`|`number[3]`|yes|N/D|Cuboid start in 1/16ths of a block|  
|`max`|`number[3]`|yes|N/D|Cuboid end in 1/16ths of a block|  
|`origin`|`number[3]`|no|`[0, 0, 0]`|If `rotation` is set, pivot point in 1/16ths of a block|  
|`rotation`|`number[3]`|no|`[0, 0, 0]`|Euler rotation in degrees about X, Y and Z axes. Each axis is clamped to `[-180, 180]` and snapped to the nearest multiple of 15 degrees|  
|`rescale`|`bool`|no|`true`|Whether the model should be forced into block bounds|  
|`shade`|`bool`|no|`true`|Whether a basic fake-light shading is applied to the cuboid|  
|`faces`|`object`|yes|N/D|Per-direction face definitions|  

## Face object

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`texture`|`string`|yes|N/D|Texture slot name|  
|`uv`|`number[4]`|no|dynamic|Lets a face sample a sub-rect of the texture|  
|`uv_rotation`|`number`|no|0|UV rotation, either 0, 90, 180 or 270|  
|`cullface`|`string`|no|N/D|The face is skipped if the neighbouring block in that direction has a face that's marked as opaque to touch|  
|`tint`|`integer`|no|N/D|Tint group index used by climate and biomes|  
|`world_locked`|`bool`|no|`false`|Samples the texture slot of whichever face ends up on this side after `model_facing`, instead of this face's own slot|  

## Example

```json
{
  "textures": ["top", "bottom", "east", "west", "north", "south"],
  "elements": [
    {
      "min": [0, 0, 0],
      "max": [16, 16, 16],
      "faces": {
        "top":      { "texture": "top",     "cullface": "bottom"    },
        "bottom":   { "texture": "bottom",  "cullface": "top"       },
        "north":    { "texture": "north",   "cullface": "south"     },
        "south":    { "texture": "south",   "cullface": "north"     },
        "east":     { "texture": "east",    "cullface": "west"      },
        "west":     { "texture": "west",    "cullface": "east"      }
      }
    }
  ]
}
```
