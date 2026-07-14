# Library: Slabs

The `builtin:slabs.lua` library provides utility functions for registering slab types of regular blocks.  

## Importing

```lua
local slabs = require("builtin:slabs.lua")
```

## Functions

### Function: `slabs.on_place(identifier) -> function`

Produces an `on_place` callback for a specified slab identifier  

#### Arguments

- `identifier` is the namespaced block ID. Namespace can be omitted, in which case `core.NAMESPACE` is substituted  

#### Return value

- A function that implements correct slab placement logic to be used in block registrations  

### Function: `slabs.register(identifier, prototype, options) -> integer`

A wrapper for `blocks.register` to define a slab block family  

#### Arguments

- `identifier` is the namespaced block ID. Namespace can be omitted, in which case `core.NAMESPACE` is substituted  

- `prototype` is the base (prototype) block definition which the function builds upon  

- `options` is an optional table with additional parameters

#### Return value

- Numeric block ID

#### Notes

- The registered block family will have an `orientation` state and variants for `top`, `bottom`, and `double` values of it  

- The function doesn't modify touch response values from the prototype, though it does define block collisions  
