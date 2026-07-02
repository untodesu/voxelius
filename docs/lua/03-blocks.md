# Lua API: Blocks

Blocks are registered at init time (after that, `blocks.get` is stripped from all the Lua states in the game); this describes functions and possible values

## Constants

### `blocks.RENDER`

Defines a block rendering step

|Name|Description|  
|----|----|  
|`blocks.RENDER_OPAQUE`|Block is rendered first without alpha-testing|  
|`blocks.RENDER_BLEND`|Block is rendered second using alpha-testing and blending|  
|`blocks.RENDER_NONE`|Block is not rendered|  

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

### `blocks.TOUCH`

Defines a touch response for a block

|Name|Description|  
|----|----|  
|`blocks.TOUCH_NONE`|Block is not collidable|  
|`blocks.TOUCH_SOLID`|Movement is stopped in the touch direction|  
|`blocks.TOUCH_BOUNCE`|Movement is reflected with some attenuation. Reserved for future use (e.g. slime-like blocks), no builtin block uses it yet|  
|`blocks.TOUCH_THROUGH`|Movement is attenuated but direction is unchanged. Reserved for future use (e.g. cobweb, liquids), no builtin block uses it yet|  

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

### `blocks.add(name, def)`  

Register a new block in the registry  

**Arguments:**

- `name` is a namespaced ID of a block, eg `mymod:coolblockname`;  
- `def` is a table of block definitions, see below for that;  

**Notes:**

If the same block is already registered (mind you `mymod:myblock` and `joe_mod:myblock` are different blocks) in the registry, the encroaching block is renamed by rule (see below) and the game keeps loading, but the conflict is always surfaced as an **error** printed to the game console — this is never silent, since it almost always means two mods stomping on each other's namespace.

**Rename rule:** on conflict, the encroaching `name` is suffixed with `~N`, where `N` is the smallest integer starting at `1` that produces a free id (e.g. `mymod:myblock~1`). This keeps loading deterministic and reproducible across runs with the same mod list/order.

**WARNING:** after the game has loaded all the mods, this function is stripped from all the Lua states in the host application!!!  

## Registration table

### Fields

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`render`|`integer`|yes|N/D|Specify block rendering mode|  
|`blockmodel`|`string`|yes|N/D|Specify block model|  
|`animated`|`bool`|no|`false`|When set to `true`, arrays of textures are used as frames instead of a varied-by-position|  
|`textures`|`table<string, string[]>`|yes, unless `render` is specified as `blocks.RENDER_NONE`|N/D|Specify a list of textures. Any faces required by the blockmodel that aren't defined here, will use the `default` entry|  
|`hardness`|`number`|no|`0.0`|Time to break, zero means breaks instantly|  
|`tool`|`array<integer>`|no|`[]`|Preferred tools for breaking the block at full speed with full drops. Bare hands (`blocks.TOOL_NONE`) can always break any block — slower, and possibly with degraded/no drops — this list only specifies which tools get the *proper* result|  
|`sound`|`string`|no|N/D|Specify sound set for the block|  
|`drops`|`array<table<...>>`|no|`[]`|Specify item drops for specific tools and conditions|  
|`emit`|`integer`|no|`0`|Specify light emission of the block|  
|`dissipate`|`integer`|no|`0`|Specify the amount of light the block dissipates when passed through|  
|`touch`|`integer`|no|`blocks.TOUCH_SOLID`|Specify the touch response of a block|  
|`states`|`table<string,table<...>>`|no|`{}`|Specify a table of blockstates. Each entry has a `default` and an optional `hint` list used only for load-time validation of `variants[].when` (see below)|  
|`variants`|`array<table<...>>`|no|`[]`|Specify variants of the block|  
|`on_rtick`|`function(world, bx, by, bz)`|no|N/D|Random tick handler|  
|`on_stick`|`function(world, bx, by, bz)`|no|N/D|Scheduled tick handler|  
|`on_place`|`function(world, bx, by, bz, placement)`|no|N/D|Place handler|  
|`on_break`|`function(world, bx, by, bz)`|no|N/D|Break handler|  
|`on_interact`|`function(world, bx, by, bz, actor)`|no|N/D|Interaction handler|  

### State example

```lua
states = {
    <state_name> = {
        default = "<default_state>",
        hint = ["<value_a>", "<value_b>", ...]  -- optional
    }
}
```

Blockstate values are hashed strings — any value can be written via `world.sset`, `hint` does not restrict this at runtime. `hint` is purely a registration-time cross-check: every `when` clause across this block's `variants` is validated against the union of `hint` lists of the states it references, and any value not present in `hint` produces a console warning at load time (typo protection, e.g. `orientation = "bottum"`). Blocks that omit `hint` for a state simply skip validation for that state.

### Variants example

```lua
variants = [
    {
        when = { <state_name> = "<state_value>" },
        apply = {
            blockmodel = "builtin:slab.json",
            translate = [0, 0, 0]
        }
    }
]
```
