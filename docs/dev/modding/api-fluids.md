# Fluids API

Each block can define a fluid it's filled with and a level  

## Constants: fluid gravity

Defines a direction the fluid's level decreases in  

|Name|Description|  
|----|----|  
|`fluids.GRAVITY_DOWN`|The fluid sticks to the bottom face of a block|  
|`fluids.GRAVITY_UP`|The fluid sticks to the top face of a block|  

## Constant: null fluid

The `fluids.NULL_FLUID` constant defines an empty or non-existent fluid

## Functions

### Function: `fluids.add(name, def) -> integer`

Register a new fluid in the registry

#### Arguments

- `name` is a namespaced ID of a fluid, eg `mymod:mywater`  
- `def` is a fluid definition table, see below for that  

#### Return value

Returns the numeric fluid ID

#### Notes

**Rename rule:** on conflict, the encroaching `name` is suffixed with `~N`, where `N` is the smallest integer starting at `1` that produces a free id (e.g. `mymod:mywater~1`). This keeps loading deterministic and reproducible across runs with the same mod list/order.

## Fluid definition

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`gravity`|`integer`|yes|N/D|One of `fluids.GRAVITY_XXXX` constants|  
|`opaque`|`boolean`|no|`false`|If set to `true`, the fluid's quads will be forced into a `blocks.RENDER_SOLID` rendering step instead of its own|  
|`max_level`|`integer`|no|N/D|If set and a `fluid_level` property in a block definition is higher than this, the engine will emit a warning into console|  
|`tint`|`integer`|no|0|Tint group index used by climate and biomes|  
|`textures`|`table`|yes|N/D|Fluid textures|  

## Fluid textures

- Fluids are _assumed_ to be animated. This means a fluid will not textures randomized by world position  

- Fluids only define _two_ hard-coded texture slots - `still` and `flowing`  

- The texture syntax is otherwise equivalent to one for [block definitions](api-blocks.md)  
