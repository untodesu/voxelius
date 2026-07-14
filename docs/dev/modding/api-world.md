# Engine API: World

Mods can access blocks in the world using the `world` library

## Functions

### Function: `world.get_block(bx, by, bz) -> integer`

Retreive a numeric block ID from the world  

#### Arguments  

- `bx` is the world-scale block X position  
- `by` is the world-scale block Y position  
- `bz` is the world-scale block Z position  

#### Return value

- Normally, a numeric block ID  
- If there is no block or the chunk is not present, `blocks.NULL_BLOCK` is returned  

### Function: `world.set_block(bx, by, bz, id) -> boolean`

Assign a block ID in the world

#### Arguments  

- `bx` is the world-scale block X position  
- `by` is the world-scale block Y position  
- `bz` is the world-scale block Z position  
- `id` is the numeric block ID; to unset the block (aka set to void), pass `blocks.NULL_BLOCK` as the value  

#### Return value

- `true` if the block was set, `false` otherwise (eg. the chunk is not present)  

### Function: `world.get_light(bx, by, bz) -> integer`

Retreive lighting information about a block position  

#### Arguments  

- `bx` is the world-scale block X position  
- `by` is the world-scale block Y position  
- `bz` is the world-scale block Z position  

#### Return value

- Luminance value of the block  

### Function: `world.get_state(bx, by, bz, st) -> string`

Retrive a blockstate at a given position  

#### Arguments  

- `bx` is the world-scale block X position  
- `by` is the world-scale block Y position  
- `bz` is the world-scale block Z position  

#### Return value

- Blockstate's value normally  
- If the block is void, has no such blockstate or the chunk is not present, `nil` is returned  

### Function: `world.set_state(bx, by, bz, st, val) -> boolean`

Assign a blockstate to a given position  

#### Arguments  

- `bx` is the world-scale block X position  
- `by` is the world-scale block Y position  
- `bz` is the world-scale block Z position  
- `st` is the blockstate name  
- `val` is the blockstate value  

#### Return value

- `true` if the blockstate was set, `false` otherwise (eg. the block is void or has no such blockstate)  

### Function: `world.get_temperature_base(bx, by, bz) -> integer`

Retreive a biome-defined temperature base for a given block

#### Arguments  

- `bx` is the world-scale block X position  
- `by` is the world-scale block Y position  
- `bz` is the world-scale block Z position  

#### Return value

- Temperature in Kelvin degrees;
- If the block is void or the chunk is not present, `0` is returned

#### Notes

Most biomes might not define this, if so, `298` is defined for most of them, which corresponds to 25 degrees Celcius  

### Function: `world.get_temperature(bx, by, bz) -> integer`

Retreive a biome-influenced seasonal temperature for a given block

#### Arguments  

- `bx` is the world-scale block X position  
- `by` is the world-scale block Y position  
- `bz` is the world-scale block Z position  

#### Return value

- Temperature in Kelvin degrees;
- If the block is void or the chunk is not present, `0` is returned

#### Notes

This can be influenced by season, weather and other factors. Relatively low values (sub-zero Centigrade) can cause snow to fall when the game wants it to be raining at that specific location  

### Function: `world.schedule(bx, by, bz, dt)`

Schedule an `on_stick` call to a block, `dt` ticks from now.  

- `bx` is the world-scale block X position  
- `by` is the world-scale block Y position  
- `bz` is the world-scale block Z position  
- `dt` is the tick delay for the callback  
