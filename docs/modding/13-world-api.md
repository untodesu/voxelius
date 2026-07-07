# Modding: world API

World is a C++ object interfaced into Lua as a method-call object (`world:method(...)`, `world` is implicitly passed as `self`) that operates with blocks/voxels within a dimension. Each game can support multiple worlds, which parameters are also registered in Lua.  


## Functions

### Function: `world:bget(bx, by, bz) -> integer`

Retreive a numeric block ID from the world  

#### Arguments  

- `bx` is the world-scale block X position;  
- `by` is the world-scale block Y position;  
- `bz` is the world-scale block Z position;  

#### Return value

- Normally, a numeric block ID;  
- If there is no block or the chunk is not present, `blocks.NULL_BLOCK` is returned;  

### Function: `world:bset(bx, by, bz, id)`

Assign a block ID in the world

#### Arguments  

- `bx` is the world-scale block X position;  
- `by` is the world-scale block Y position;  
- `bz` is the world-scale block Z position;  
- `id` is the numeric block ID; to unset the block (aka set to void), pass `blocks.NULL_BLOCK` as the value;  

### Function: `world:lget(bx, by, bz) -> integer`

Retreive lighting information about a block position  

#### Arguments  

- `bx` is the world-scale block X position;  
- `by` is the world-scale block Y position;  
- `bz` is the world-scale block Z position;  

#### Return value

- Luminance value of the block  

### Function: `world:sget(bx, by, bz, st) -> string`

Retrive a blockstate at a given position  

#### Arguments  

- `bx` is the world-scale block X position;  
- `by` is the world-scale block Y position;  
- `bz` is the world-scale block Z position;  

#### Return value

- Blockstate's value normally;  
- If the block is void, has no such blockstate or the chunk is not present, `nil` is returned;  

### Function: `world:sset(bx, by, bz, st, val)`

Assign a blockstate to a given position  

#### Arguments  

- `bx` is the world-scale block X position;  
- `by` is the world-scale block Y position;  
- `bz` is the world-scale block Z position;  
- `st` is the blockstate name;  
- `val` is the blockstate value;  

### Function: `world:tget(bx, by, bz) -> integer`

Retreive a biome-defined temperature base for a given block

#### Arguments  

- `bx` is the world-scale block X position;  
- `by` is the world-scale block Y position;  
- `bz` is the world-scale block Z position;  

#### Return value

- Temperature in Kelvin degrees;
- If the block is void or the chunk is not present, `0` is returned

#### Notes

Most biomes might not define this, if so, `298` is defined for most of them, which corresponds to 25 degrees Celcius  

### Function: `world:tsget(bx, by, bz) -> integer`

Retreive a biome-influenced seasonal temperature for a given block

#### Arguments  

- `bx` is the world-scale block X position;  
- `by` is the world-scale block Y position;  
- `bz` is the world-scale block Z position;  

#### Return value

- Temperature in Kelvin degrees;
- If the block is void or the chunk is not present, `0` is returned

#### Notes

This can be influenced by season, weather and other factors. Relatively low values (sub-zero Centigrade) can cause snow to fall when the game wants it to be raining at that specific location  

### Function: `world:sched(bx, by, bz, dt)`

Schedule an `on_stick` call to a block, `dt` ticks from now.  

- `bx` is the world-scale block X position;  
- `by` is the world-scale block Y position;  
- `bz` is the world-scale block Z position;  
- `dt` is the tick delay for the callback;  
