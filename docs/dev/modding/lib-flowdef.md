# Fluids library

The `builtin:flowdef` library provides helpers for flowing blocks.

> **NOTE:** the library at least attempts Minecraft-like flowing logic, including infinite water sources and similar behaviour.

## Importing

```lua
local flowdef = require("builtin:flowdef")
```

### Flow config

|Field|Type|Default|Description|
|----|----|----|----|
|`max_level`|`integer`|8|Max level for a fluid blockstate|
|`full_height`|`integer`|14|Maximum fluid level for blockstates|
|`level_decrease`|`integer`|1|Level decrement with flow distance|
|`delay`|`integer`|5|Scripted tick delay|
|`infinite`|`boolean`|`false`|If set, flowing fluids may merge into sources|
|`slope_range`|`integer`|4|Max distance to look for a downwards slope to flow into|

### Function: `flowdef.states(config_src) -> table`

Produces a states object for block registration.

#### Arguments

- `config_src` is the source flow config table

#### Return value

A table to plug into `states` of a block definition.

### Function: `flowdef.variants(config_src) -> table`

Produces a variants object for block registration.

#### Arguments

- `config_src` is the source flow config table

#### Return value

A table to plug into `variants` of a block definition.

### Function: `flowdef.on_place(config_src) -> function`

Produces an `on_place` callback for the given flow config.

#### Arguments

- `config_src` is the source flow config table

#### Return value

A function that implements update logic for flowing blocks.

### Function: `flowdef.on_tick(config_src) -> function`

Produces an `on_tick` callback for the given flow config.

#### Arguments

- `config_src` is the source flow config table

#### Return value

A function that implements update logic for flowing blocks.
