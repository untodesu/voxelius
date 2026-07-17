# Blocks API

Mods can register new block types and access existing one using the `blocks` library provided by the engine.  

## Constants: render mode

Defines a block rendering step

|Name|Description|  
|----|----|  
|`blocks.RENDER_NONE`|Block is not rendered (eg. air)|  
|`blocks.RENDER_SOLID`|Block is rendered as opaque, cullable geometry|  
|`blocks.RENDER_ALPHA`|Block is rendered using alpha testing and blending|  

## Constants: block face

Defines a block face used for culling and other grid-aligned operations  

|Name|Description|  
|----|----|  
|`blocks.FACE_NORTH`|North face|  
|`blocks.FACE_SOUTH`|South face|  
|`blocks.FACE_EAST`|East face|  
|`blocks.FACE_WEST`|West face|  
|`blocks.FACE_TOP`|Top/upper face|  
|`blocks.FACE_BOTTOM`|Bottom/lower face|  
|`blocks.FACE_UP`|Same as `blocks.FACE_TOP`|  
|`blocks.FACE_DOWN`|Same as `blocks.FACE_BOTTOM`|  

## Constants: tool conditionals

Defines a tool category used for block interactions  

|Name|Description|  
|----|----|  
|`blocks.TOOL_NONE`|Bare hands|  
|`blocks.TOOL_BLADE`|A bladed tool (eg. sword, knife)|  
|`blocks.TOOL_XBLADE`|A mult-bladed tool (eg. scissors)|  
|`blocks.TOOL_AXE`|Axe-like tool|  
|`blocks.TOOL_HOE`|Hoe-like tool|  
|`blocks.TOOL_SHOVEL`|Shovel-like tool|  
|`blocks.TOOL_HAMMER`|Hammer-like tool|  
|`blocks.TOOL_PICK`|Pickaxe-like tool|  

## Constants: touch responses

Defines a touch response for a block  

|Name|Description|  
|----|----|  
|`blocks.TOUCH_NONE`|Block is not collidable|  
|`blocks.TOUCH_SOLID`|Movement is stopped in the touch direction|  
|`blocks.TOUCH_BOUNCE`|Movement is reflected with some attenuation. Reserved for future use (e.g. slime-like blocks), no builtin block uses it yet|  
|`blocks.TOUCH_THROUGH`|Movement is attenuated but direction is unchanged. Reserved for future use (e.g. cobweb, liquids), no builtin block uses it yet|  

## Constants: category bits

Freeform grouping tags checked at runtime with `blocks.has_tag`. Any block can carry any number of tags via the `tags` registration field.

|Name|Description|  
|----|----|  
|`blocks.TAG_GAS`|Non-solid, passable block (eg. air)|  
|`blocks.TAG_ROCK`|Stone-family block, used by tools/worldgen that care about "is this rock"|  
|`blocks.TAG_SOIL`|A soil-type block. Stuff like dirt, mud and so on end up here|  
|`blocks.TAG_TURF`|A turf-type block. Stuff like grass ends up here|  
|`blocks.TAG_FOIL`|Foilage, leaves, grass, etc|  
|`blocks.TAG_WOOD`|Wooden blocks|  


## Constant: null block

The `blocks.NULL_BLOCK` constant defines an empty, undefined or otherwise invalid block ID. Gameplay/gamedev-wise it can be treated as a void block  

## Functions

### Function: `blocks.get(name) -> integer`

Retreive a numeric block stem ID from a namespaced block ID

#### Arguments

- `name` is a namespaced ID of a block, eg. `mymod:coolblockname`  

#### Return value

- Numeric block ID if the game likes your input  
- If the block is missing or the namespace ID is malformed, `blocks.NULL_BLOCK` is returned  

#### Notes

- If a block defines states and variants, the returned value is a _stem_ ID. If placed in the world, this block is not rendered, not collidable or raycastable  

### Function: `blocks.has_tag(id, tag) -> boolean`

Check if a block has a specified tag.  

#### Arguments

- `name` is a namespaced ID of a block, eg. `mymod:coolblockname`  
- `tag` is a tag constant, eg. `blocks.TAG_SOIL`  

#### Return value

- `true` if the block has the specified tag, `false` otherwise  

### Function: `blocks.add(name, def) -> integer`
### Function: `blocks.add(name, prototype, def) -> integer`

Register a new block in the registry  

#### Arguments

- `name` is a namespaced ID of a block, eg `mymod:coolblockname`  
- `def` is a table of block definitions, see below for that  
- `prototype` (3-argument form) is a base table of block definitions shared across a family of blocks (eg. all stone variants). `def` is merged on top of it, with `def`'s fields taking priority on conflicts  

#### Notes

**Rename rule:** on conflict, the encroaching `name` is suffixed with `~N`, where `N` is the smallest integer starting at `1` that produces a free id (e.g. `mymod:myblock~1`). This keeps loading deterministic and reproducible across runs with the same mod list/order.

## Block definition

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`render`|`integer`|yes|N/D|One of `blocks.RENDER_XXXX` constants|  
|`textures`|`table`|depends|`{}`|Textures to attach to the block model|  
|`animated`|`boolean`|no|`false`|When set to true, multiple textures from the `textures` value are used as animation frames instead of being positionally randomized in the world|  
|`model_name`|`string`|depends|N/D|Block model name for this variant|  
|`model_offset`|`float[3]`|depends|`[0,0,0]`|Offset of the resolved block model|  
|`model_facing`|`integer`|no|`blocks.FACE_NORTH`|One of the `blocks.FACE_XXXX` constants. Says which way the model's own north face should end up pointing; rotates the whole resolved block model|  
|`bcoll_name`|`string`|depends|N/D|Block collision shape for this variant|  
|`bcoll_offset`|`float[3]`|depends|`[0,0,0]`|Block collision offset|  
|`bcoll_facing`|`integer`|no|`blocks.FACE_NORTH`|One of the `blocks.FACE_XXXX` constants. Rotates the whole resolved collision shape the same way `model_facing` rotates the model; set independently since collision doesn't have to follow the visual, though it usually should|  
|`health`|`integer`|no|`0`|Base amount of hit points required for the block to be broken. Varies with different effects active on the tool|  
|`sound`|`string`|no|N/D|Sound set to use for this block|  
|`emission`|`integer`|no|0|Emission light value|  
|`dissipation`|`integer`|no|15|How much light the block eats while light passes through|  
|`touch`|`integer`|no|`blocks.TOUCH_SOLID`|Block's touch response|  
|`touch_coeffs`|`float[3]`|no|`[0,0,0]`|Block's touch response coefficients|  
|`tags`|`integer[]`|no|`[]`|Block tags|  
|`states`|`table`|no|`{}`|Blockstates table|  
|`variants`|`table`|no|`{}`|Variants table|  
|`on_random_tick`|`function`|no|`nil`|Random tick handler|  
|`on_sched_tick`|`function`|no|`nil`|Scheduled tick hanlder|  
|`on_place`|`function`|no|`nil`|Placement handler, can decide whether it's ok or not to place the block there|  
|`on_break`|`function`|no|`nil`|Break handler|  
|`on_interact`|`function`|no|`nil`|Interaction handler|  

### Facing rotation

The `model_facing` and `bcoll_facing` fields each define a single uniquely-determined 90-degree-step rotation of the entire model/collision shape. If we talk in simple terms, these fields define where the model's north face points - by default it points to the world north but it can be changed  

|Value|Rotation|Image|  
|----|----|----|  
|`blocks.FACE_NORTH`|None|![](facing-north.png)|  
|`blocks.FACE_SOUTH`|180 degrees around Y|![](facing-south.png)|  
|`blocks.FACE_EAST`|+90 degrees around Y|![](facing-east.png)|  
|`blocks.FACE_WEST`|-90 degrees around Y|![](facing-west.png)|  
|`blocks.FACE_TOP`|-90 degrees around X|![](facing-top.png)|  
|`blocks.FACE_BOTTOM`|+90 degrees around X|![](facing-bottom.png)|  

### Drops table


```lua
drops = {
  {
    when = { effects = { "silk_touch" } },
    items = {
      { name = "stone", count = 1 }
    }
  },
  {
    -- entries without a `when` always match; put fallback
    -- entries last since matching stops at the first hit
    items = {
      { name = "cobblestone", count = 1 }
    }
  }
}
```

Entries are evaluated top to bottom; the first entry whose `when` clause matches (or that has no `when` at all) provides the drops, later entries are ignored.

### States table

```lua
states = {
  <state_name> = {
    default = "<default_state>",
    hint = { "<value_a>", "<value_b>", ... }  -- optional
  }
}
```

Blockstate values are hashed strings - any value can be written via `world.sset`, `hint` does not restrict this at runtime. `hint` is purely a registration-time cross-check: every `when` clause across this block's `variants` is validated against the union of `hint` lists of the states it references, and any value not present in `hint` produces a console warning at load time (typo protection, e.g. `orientation = "bottum"`). Blocks that omit `hint` for a state simply skip validation for that state.

### Variants

```lua
variants = {
  {
    when = { <state_name> = "<state_value>" },
    overrides = {
      model_name = "slab_top",
      bcoll_offset = { 0, 8, 0 },
      health = 2,
      drops = { ... }
    }
  }
}
```

`overrides` is a table of registration fields (same shape as the top-level definition) that gets merged on top of the block's base definition whenever `when` matches the block's current blockstate values.

### Target table

Some callbacks pass in a `target` table that defines the location at which the initiator was looking when attempting to interact with a block  

|Field|Type|Description|  
|----|----|----|  
|`stem`|`integer`|Numeric block stem ID|  
|`face`|`integer`|Surface direction|  
|`ni`|`number`|Surface normal X/I component|  
|`nj`|`number`|Surface normal Y/J component|  
|`nk`|`number`|Surface normal Z/K component|  
|`bx`|`integer`|Block position X component|  
|`by`|`integer`|Block position Y component|  
|`bz`|`integer`|Block position Z component|  
|`rx`|`number`|Block-local position X component|  
|`ry`|`number`|Block-local position Y component|  
|`rz`|`number`|Block-local position Z component|  

### `on_place` handler

```lua
on_place = function(bx, by, bz, target, actor)
  if condition_to_reject then
    return nil -- blocks the placement entirely
  end

  -- permits placement with these initial states
  return { <state_name> = "<state_value>", ... }
end
```

Returning `nil` blocks the placement; returning a table (empty or not) permits it, with any entries in the table used as initial blockstate values for states not covered by their `default`.

### `on_break` handler

```lua
on_break = function(bx, by, bz, actor)
  -- Actions to do before the block is broken
end
```

### `on_interact` handler

```lua
on_interact = function(bx, by, bz, target, actor)
  -- Actions to do when someone interacts with the block
end
```

### `on_random_tick` handler

```lua
on_random_tick = function(bx, by, bz)
  -- Actions to do when the block is randomly ticked
end
```

### `on_sched_tick` handler

```lua
on_sched_tick = function(bx, by, bz)
  -- Actions to do when the block is ticked via scheduling
end
```
