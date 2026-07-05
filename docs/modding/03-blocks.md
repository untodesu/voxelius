# Modding: Blocks API

Blocks are registered at init time (after that, `blocks.add` is stripped from all the Lua states in the game); this describes functions and possible values

## Constants

### `blocks.RENDER`

Defines a block rendering step

|Name|Description|  
|----|----|  
|`blocks.RENDER_SOLID`|Block is rendered as opaque, cullable geometry|  
|`blocks.RENDER_NONE`|Block is not rendered (eg. air)|  

### `blocks.FACE`

Defines a block face used for culling and other grid-aligned operations  

> **TIP:** blockstates are hashed when getting parsed by runtime host, so literally anything can be passed there, including these values!! So instead of typing a string every time, you can use these to define a block orientation  

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

### `blocks.TOOL`

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

### `blocks.TOUCH`

Defines a touch response for a block  

|Name|Description|  
|----|----|  
|`blocks.TOUCH_NONE`|Block is not collidable|  
|`blocks.TOUCH_SOLID`|Movement is stopped in the touch direction|  
|`blocks.TOUCH_BOUNCE`|Movement is reflected with some attenuation. Reserved for future use (e.g. slime-like blocks), no builtin block uses it yet|  
|`blocks.TOUCH_THROUGH`|Movement is attenuated but direction is unchanged. Reserved for future use (e.g. cobweb, liquids), no builtin block uses it yet|  

### `blocks.TAG`

Freeform grouping tags checked at runtime with `blocks.has_tag`. Any block can carry any number of tags via the `tags` registration field.

|Name|Description|  
|----|----|  
|`blocks.TAG_GAS`|Non-solid, passable block (eg. air)|  
|`blocks.TAG_ROCK`|Stone-family block, used by tools/worldgen that care about "is this rock"|  

### `blocks.NULL_BLOCK`

Defines an empty, undefined or otherwise invalid block ID. Gameplay/gamedev-wise it can be treated as a block of void or vacuum  

## Functions

### `blocks.get(name)`

Retreive a numeric block ID from a namespaced block ID  

**Arguments:**

- `name` is a namespaced ID of a block, eg. `mymod:coolblockname`;  

**Return value:**

- Numeric block ID if the game likes your input;  
- If the block is missing or the namespace ID is malformed, `blocks.NULL_BLOCK` is returned;  

### `blocks.has_tag(id, tag) -> bool`

Check whether a block carries a given tag.

**Arguments:**

- `id` is a numeric block ID, as returned by `blocks.get` or passed into a handler (eg. `world:bget`'s return value);  
- `tag` is one of the `blocks.TAG_*` constants;  

**Return value:**

- `true` if the block's `tags` list (see registration table below) contains `tag`, `false` otherwise (including for `blocks.NULL_BLOCK`);  

### `blocks.add(name, def)`  
### `blocks.add(name, prototype, def)`  

Register a new block in the registry  

**Arguments:**

- `name` is a namespaced ID of a block, eg `mymod:coolblockname`;  
- `def` is a table of block definitions, see below for that;  
- `prototype` (3-argument form) is a base table of block definitions shared across a family of blocks (eg. all stone variants). `def` is merged on top of it, with `def`'s fields taking priority on conflicts — this lets a mod define common fields (`render`, `textures`, `health`, `tools`, `emission`, `dissipation`, `tags`, ...) once and only override what differs per block;  

**Notes:**

If the same block is already registered (mind you `mymod:myblock` and `joe_mod:myblock` are different blocks) in the registry, the encroaching block is renamed by rule (see below) and the game keeps loading, but the conflict is always surfaced as an **error** printed to the game console — this is never silent, since it almost always means two mods stomping on each other's namespace.

**Rename rule:** on conflict, the encroaching `name` is suffixed with `~N`, where `N` is the smallest integer starting at `1` that produces a free id (e.g. `mymod:myblock~1`). This keeps loading deterministic and reproducible across runs with the same mod list/order.

**WARNING:** after the game has loaded all the mods, this function is stripped from all the Lua states in the host application!!!  

## Registration table

### Fields

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`render`|`integer`|yes|N/D|Specify block rendering mode|  
|`model_name`|`string`|yes, unless `render` is `blocks.RENDER_NONE`|N/D|Name of the block model (`data/models/block/<model_name>.json`)|  
|`model_offset`|`table<integer,3>`|no|`[0, 0, 0]`|Offset (in 1/16ths of a block, same units as model element coordinates) applied to the render model. Analogous to `bcoll_offset` but for the visual geometry rather than collision|  
|`bcoll_name`|`string`|no|N/D|Name of the collision model (`data/models/bcoll/<bcoll_name>.json`). Omit for non-collidable blocks (eg. air)|  
|`bcoll_offset`|`table<integer,3>`|no|`[0, 0, 0]`|Offset (in 1/16ths of a block, same units as model/collision element coordinates) applied to the collision model. Used eg. by a top slab to sit flush with the top of the space|  
|`animated`|`bool`|no|`false`|When set to `true`, arrays of textures are used as frames instead of a varied-by-position|  
|`textures`|`table<string, string[]>`|yes, unless `render` is `blocks.RENDER_NONE`|N/D|Maps a model texture slot name (eg. `top`, `bottom`) to a list of texture variants. Any slot required by the model that isn't defined here falls back to the `default` entry|  
|`health`|`integer`|no|`0`|Hit points; how many hits (roughly, tool/effect-dependent) it takes to break the block, zero means it breaks instantly|  
|`tools`|`table<integer>`|no|`[]`|Preferred tools for breaking the block at full speed with full drops. Bare hands (`blocks.TOOL_NONE`) can always break any block — slower, and possibly with degraded/no drops — this list only specifies which tools get the *proper* result|  
|`sound`|`string`|no|N/D|Specify sound set for the block|  
|`emission`|`integer`|no|`0`|Specify light emission of the block|  
|`dissipation`|`integer`|no|`0`|Specify the amount of light the block dissipates when passed through|  
|`touch`|`integer`|no|`blocks.TOUCH_SOLID`|Specify the touch response of a block|  
|`tags`|`table<integer>`|no|`[]`|List of `blocks.TAG_*` values, queryable at runtime via `blocks.has_tag`|  
|`drops`|`table<table<...>>`|no|`[]`|Specify item drops for specific tools and conditions, see below|  
|`states`|`table<string,table<...>>`|no|`{}`|Specify a table of blockstates. Each entry has a `default` and an optional `hint` list used only for load-time validation of `variants[].when` (see below)|  
|`variants`|`table<table<...>>`|no|`[]`|Specify variants of the block|  
|`on_rtick`|`function(world, bx, by, bz)`|no|N/D|Random tick handler|  
|`on_stick`|`function(world, bx, by, bz)`|no|N/D|Scheduled tick handler, fired via `world:sched`|  
|`on_place`|`function(world, bx, by, bz, placement) -> table\|nil`|no|N/D|Place handler, see below|  
|`on_break`|`function(world, bx, by, bz)`|no|N/D|Break handler|  
|`on_interact`|`function(world, bx, by, bz, actor)`|no|N/D|Interaction handler|  

### Drops example

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

### State example

```lua
states = {
  <state_name> = {
    default = "<default_state>",
    hint = { "<value_a>", "<value_b>", ... }  -- optional
  }
}
```

Blockstate values are hashed strings — any value can be written via `world:sset`, `hint` does not restrict this at runtime. `hint` is purely a registration-time cross-check: every `when` clause across this block's `variants` is validated against the union of `hint` lists of the states it references, and any value not present in `hint` produces a console warning at load time (typo protection, e.g. `orientation = "bottum"`). Blocks that omit `hint` for a state simply skip validation for that state.

### Variants example

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

### `on_place` handler

```lua
on_place = function(world, bx, by, bz, placement)
  -- placement.tblock is the block ID of the block placement was targeted against
  -- placement.tface is one of blocks.FACE_* — the face of that block that was clicked
  -- placement.tx, placement.ty, placement.tz are that target block's coordinates

  if condition_to_reject then
    return nil -- blocks the placement entirely
  end

  return { <state_name> = "<state_value>", ... } -- permits placement with these initial states
end
```

Returning `nil` blocks the placement; returning a table (empty or not) permits it, with any entries in the table used as initial blockstate values for states not covered by their `default`.
