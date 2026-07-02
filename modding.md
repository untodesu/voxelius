# Modding: Core API and considerations

Base utilities available in every Lua state from the moment it's created — logging and script loading. Everything here lives under the `core` global table.

## Constants

### `core.VERSION`

Specifies game versions. See `core/version.hh` for the C++ equivalent  

|Value|Description|  
|----|----|  
|`core.VERSION_MAJOR`|Major version number|  
|`core.VERSION_MINOR`|Minor version number|  
|`core.VERSION_PATCH`|Patch version number|  
|`core.VERSION_SCM_BRANCH`|SCM branch name|  
|`core.VERSION_SCM_REVISION`|SCM revision aka git commit hash|  
|`core.VERSION_SEMANTIC`|Semantic version string|  
|`core.VERSION`|Full build-system-defined version string|  

### `core.NAMESPACE`

The current mod's namespace (the `name` field from `modinfo.conf`). Used as the implicit namespace for identifiers that don't specify one — see below.

## Functions

### `core.log_info(...)`

Print an informational message to the game console/log.

### `core.log_warning(...)`

Print a warning to the game console/log. Used for recoverable issues (e.g. a missing texture falling back to a placeholder, a deprecated field still being read).

### `core.log_error(...)`

Print an error to the game console/log. Used for issues that don't stop loading but leave something broken (e.g. `blocks.add` id conflict, malformed blockmodel).

### `core.log_critical(...)`

Print a critical error to the game console/log. Implies the game cannot continue in a well-defined state; the host application may abort after printing.

### `core.log_debug(...)`

Print a debug message. Only shown when the engine is running with debug logging enabled; a no-op (but still valid to call) otherwise, so mods don't need to guard calls with a debug flag check.

### `core.do_file(path)`

Load and immediately execute another Lua script, in the same Lua state (shares globals with the caller).

**Arguments:**

- `path` is a namespaced virtual filesystem path, eg. `"builtin:blocks/stone.lua"`;  

**Notes:**

- The function doesn't use caching. Calling `do_file` twice with the same path just executes the file two times;  

- A missing or malformed file raises a Lua error; any mod that raises an error is assumed to fail to load. This _does not_ abort the whole game boot unless it's a `builtin` mod;  

## Namespaced identifiers

```
[<namespace>:][identifier]
```

- `namespace` can be omitted, in which case `core.NAMESPACE` is used;  
- `identifier` must be a valid C identifier with an exception of an allowed `~` character that follows the same rules as underscores;  

## Error handling

The host tracks everything a mod registers (block IDs, etc.) while its scripts are running. If an uncaught error propagates out of the mod's top-level script, everything that the mod has registered is rolled back and the mod is marked as failed to load. Any other mods that [depend](01-modinfo.md) on the failed mod are also skipped.  

> **NOTE:** if we talk in terms of `builtin` mod, the game straight up crashes because you can't have the game with broken built-in content  

# Modding: mods manifest

Every mod's root directory contains a `modinfo.conf` file that contains some useful information about the mod. The file uses the common configuration format used by the engine (see `core/config/map.cc`);

## Fields

|Name|Required|Default|Description|  
|----|----|----|----|  
|`name`|yes|N/D|The mod's name/ID used for namespacing|  
|`author`|no|N/D|Metadata: the mod's author/authors|  
|`homepage`|no|N/D|Metadata: the mod's homepage URL|  
|`tracker`|no|N/D|Metadata: the mod's issue tracker URL|  
|`license`|no|`ARR`|Metadata: the mod's license as a short SPDX identifier|  
|`display`|no|Value of `name`|Metadata: the mod's display name|  
|`description`|no|N/D|Metadata: a short description|  
|`depends`|no|N/D|Comma-separated list of mod IDs that must be in loaded state before this mod's scripts are invoked|  

> **NOTE:** all mods implicitly depend on `builtin`  

## Example

```ini
name = kaboom
depends = extratools,fire

author = untodesu
homepage = https://untode.su/
tracker = https://untode.su/
license = BSD-3-Clause
display = Ka-Boom!
description = Adds explosive blocks to Voxelius
```

## Load order

Mods are ordered by a topological sort of dependencies. Within a single "group" of mods in that list, things are loaded in an alphabetical order. Mods without dependencies (ie only depending on `builtin`) are loaded first.  

> **NOTE:** dependency cycles cause the mod to fail  

> **NOTE:** non-existent dependencies cause the mod to fail  


# Modding: World API

World is a C++ object interfaced into Lua as a method-call object (`world:method(...)`, `world` is implicitly passed as `self`) that operates with blocks/voxels within a dimension. Each game can support multiple worlds, which parameters are also registered in Lua.  

## Functions

### `world:bget(bx, by, bz) -> integer`

Retreive a numeric block ID from the world  

**Arguments:**  

- `bx` is the world-scale block X position;  
- `by` is the world-scale block Y position;  
- `bz` is the world-scale block Z position;  

**Return value:**

- Normally, a numeric block ID;  
- If there is no block or the chunk is not present, `blocks.NULL_BLOCK` is returned;  

### `world:bset(bx, by, bz, id)`

Assign a block ID in the world

**Arguments:**  

- `bx` is the world-scale block X position;  
- `by` is the world-scale block Y position;  
- `bz` is the world-scale block Z position;  
- `id` is the numeric block ID; to unset the block (aka set to void), pass `blocks.NULL_BLOCK` as the value;  

### `world:lget(bx, by, bz)`

Retreive lighting information about a block position  

**Arguments:**  

- `bx` is the world-scale block X position;  
- `by` is the world-scale block Y position;  
- `bz` is the world-scale block Z position;  

**Return value:**

- Luminance value of the block  

### `world:sget(bx, by, bz, st) -> string`

Retrive a blockstate at a given position  

**Arguments:**  

- `bx` is the world-scale block X position;  
- `by` is the world-scale block Y position;  
- `bz` is the world-scale block Z position;  

**Return value:**

- Blockstate's value normally;  
- If the block is void, has no such blockstate or the chunk is not present, `nil` is returned;  

### `world:sset(bx, by, bz, st, val)`

Assign a blockstate to a given position  

**Arguments:**  

- `bx` is the world-scale block X position;  
- `by` is the world-scale block Y position;  
- `bz` is the world-scale block Z position;  
- `st` is the blockstate name;  
- `val` is the blockstate value;  

### `world:tget(bx, by, bz) -> integer`

Retreive a biome-defined temperature base for a given block

**Arguments:**  

- `bx` is the world-scale block X position;  
- `by` is the world-scale block Y position;  
- `bz` is the world-scale block Z position;  

**Return value:**

- Temperature in Kelvin degrees;
- If the block is void or the chunk is not present, `0` is returned

**Notes:**

Most biomes might not define this, if so, `298` is defined for most of them, which corresponds to 25 degrees Celcius  

### `world:tsget(bx, by, bz) -> integer`

Retreive a biome-influenced seasonal temperature for a given block

**Arguments:**  

- `bx` is the world-scale block X position;  
- `by` is the world-scale block Y position;  
- `bz` is the world-scale block Z position;  

**Return value:**

- Temperature in Kelvin degrees;
- If the block is void or the chunk is not present, `0` is returned

**Notes:**

This can be influenced by season, weather and other factors. Relatively low values (sub-zero Centigrade) can cause snow to fall when the game wants it to be raining at that specific location  

### `world:sched(bx, by, bz, dt)`

Schedule an `on_stick` call to a block, `dt` ticks from now.  

- `bx` is the world-scale block X position;  
- `by` is the world-scale block Y position;  
- `bz` is the world-scale block Z position;  
- `dt` is the tick delay for the callback;  

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

