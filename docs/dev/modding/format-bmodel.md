# Block Models

Block models define the visible geometry of a block.

Block models live under: `namespace:models/block`

> **NOTE:** texture _names_ are not part of the model. The model only declares named slots that it needs (and these names are arbitrary: `top`, `north`, `nwse`, even `cumshit9000` for all I care). The actual texture filenames are supplied per block definition. See [Blocks API](api-blocks.md).

<!-- separator -->

> **NOTE:** if the slot is missing, the `default` slot is used instead.

<!-- separator -->

> **NOTE:** namespaced IDs inside block models _must_ have a namespace. This is a thing because the engine's resource management only uses namespaced IDs for path resolutions and raw C++ load/free functions don't get a namespace as an argument

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
|`rotation`|`number[3]`|no|`[0, 0, 0]`|Euler rotation in degrees about X, Y and Z. Each axis is clamped to `[-180, 180]` and snapped to the nearest multiple of 15 degrees|
|`rescale`|`bool`|no|`true`|Whether the model is forced into block bounds|
|`shade`|`bool`|no|`true`|Whether basic fake-light shading is applied to the cuboid|
|`faces`|`object`|yes|N/D|Per-direction face definitions|

## Face object

|Field|Type|Required|Default|Description|
|----|----|----|----|----|
|`texture`|`string`|yes|N/D|Texture slot name|
|`uv`|`number[4]`|no|dynamic|Lets a face sample a sub-rect of the texture|
|`uv_rotation`|`number`|no|0|UV rotation: 0, 90, 180, or 270|
|`cullface`|`string`|no|N/D|Skip this face if the neighbouring block in that direction has a face marked opaque to touch|
|`tint`|`string`|no|`null`|Namespaced tint id|

### UV resolution

- If a face has no explicit `uv` rect, UV coordinates come from the world-space side the face points to after facing rotation (see [Blocks API](api-blocks.md)).
- If a face _does_ have an explicit `uv` rect, orientation stays locked. Facing rotation does not change it.

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
