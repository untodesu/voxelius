# Fluids library

The `builtin:flowdef.lua` library provides utility functions for flowing blocks  

> **NOTE:** the library at least attempts to implement Minecraft-like flowing logic, allowing for infinite water sources and such  

## Importing

```lua
local flowdef = require("builtin:flowdef.lua")
```

### Flow config

|Field|Type|Default|Description|  
|----|----|----|----|  
|`max_level`|`integer`|8|Max level for a fluid blockstate|  
|`level_decrease`|`integer`|1|Level decrement with flow distance|  
|`delay`|`integer`|5|Scripted tick delay|  
|`infinite`|`boolean`|`false`|If set, allows flowing fluids to merge into sources|  
|`slope_range`|`integer`|4|Maximum amount of distance to look for a downwards slope to flow into|  

### Function: `flowdef.states(config_src) -> table`

Produces a states object to use in block registering  

#### Arguments

- `config_src` is the source flow config table  

#### Return value

A table to be plugged into `states` of a block definition table  

### Function: `flowdef.variants(config_src) -> table`

Produces a variants object to use in block registering  

#### Arguments

- `config_src` is the source flow config table  

#### Return value

A table to be plugged into `variants` of a block definition table  

### Function: `flowdef.on_place(config_src) -> function`

Produces an `on_place` callback for the specified flow config  

#### Arguments

- `config_src` is the source flow config table  

#### Return value

A function that implements correct update logic for flowing blocks  

### Function: `flowdef.on_tick(config_src) -> function`

Produces an `on_tick` callback for the specified flow config  

#### Arguments

- `config_src` is the source flow config table  

#### Return value

A function that implements correct update logic for flowing blocks  
