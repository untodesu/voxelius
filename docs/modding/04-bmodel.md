# Modding: Block models

Block models define the geometry to render a block

> **NOTE:** texture _names_ are not part of the model. A model only declares which texture slots it needs (eg. `top`, `side`, etc.); the actual texture files are supplied per-block via `textures` field during registration  

## Top-level fields

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`parent`|`string`|no|N/D|Name of an another block model to inherit|  
|`textures`|`string[]`|depends|`[]`|List of texture slots the model requires|  
|`elements`|`object[]`|depends|`[]`|List of cuboids that make the model up|  

## Model inheritance

Examples of a model that inherits a cube and modifies it slightly:

```json
{
  "parent": "builtin:cube",
  "elements": [
    {
      "from": [0, 8, 0],
      "to": [16, 16, 16],
      "faces": {
        "north": { "cullface": [null] }
      }
    },
  ]
}
```

> **NOTE:** fields on the child model shadow the parent's if defined; this means the example model will have `elements` consist of a single cuboid starting at `[0 8 0]` and ending at `[16 16 16]`  

## Cuboid object

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`from`|`number[3]`|yes|N/D|Cuboid start in 1/16ths of a block|  
|`to`|`number[3]`|yes|N/D|Cuboid end in 1/16ths of a block|  
|`origin`|`number[3]`|no|`[0, 0, 0]`|If `rotation` is set, pivot point in 1/16ths of a block|  
|`rotation`|`number[3]`|no|`[0, 0, 0]`|Euler rotation in degrees about X, Y and Z axes|  
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

## Example

```json
{
  "textures": ["top", "bottom", "east", "west", "north", "south"],
  "elements": [
    {
      "from": [0, 0, 0],
      "to": [16, 16, 16],
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
