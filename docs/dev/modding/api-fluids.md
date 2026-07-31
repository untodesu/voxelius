# Fluids API

Each block can define a fluid it holds and a fill level.

## Constants: fluid gravity

Defines the direction in which the fluid level decreases.

|Name|Description|
|----|----|
|`fluids.GRAVITY_DOWN`|The fluid sticks to the bottom face of a block|
|`fluids.GRAVITY_UP`|The fluid sticks to the top face of a block|

## Constant: null fluid

`fluids.NULL_FLUID` is an empty or non-existent fluid ID.

## Functions

### Function: `fluids.add(name, def) -> integer`

Register a new fluid in the registry.

#### Arguments

- `name` is a namespaced fluid ID, eg `mymod:mywater`
- `def` is a fluid definition table (see below)

#### Return value

Returns the numeric fluid ID.

#### Notes

**Rename rule:** on conflict, the encroaching `name` gets a `~N` suffix. `N` is the smallest integer starting at `1` that yields a free id (e.g. `mymod:mywater~1`). Loading stays deterministic for the same mod list and order.

## Fluid definition

|Field|Type|Required|Default|Description|
|----|----|----|----|----|
|`gravity`|`integer`|yes|N/D|One of `fluids.GRAVITY_XXXX` constants|
|`textures`|`table`|yes|N/D|Fluid textures|
|`full_level`|`integer`|yes|N/D|Level that counts as a full cell, in 1/16ths of a block|
|`opaque`|`boolean`|no|`false`|If `true`, fluid quads use the `blocks.RENDER_SOLID` step instead of the fluid step|
|`tint`|`string`|no|`nil`|Tint group index used by climate and biomes|
|`fog_density`|`number`|no|1.0|When the client camera is inside this fluid, fog distance is divided by this value|
|`fog_color`|`number[3]`|no|`{1,1,1}`|When the client camera is inside this fluid, fog color is replaced with this|

## Fluid textures

- Fluids are _assumed_ to be animated. Texture frames are not randomized by world position.
- Fluids define only two hard-coded texture slots: `still` and `flowing`.
- Texture syntax otherwise matches [block definitions](api-blocks.md).
