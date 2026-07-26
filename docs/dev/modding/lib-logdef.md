# Logs library

The `builtin:logdef` library provides utility functions for registering log-like blocks  

## Importing

```lua
local logdef = require("builtin:logdef")
```

### Function: `logdef.on_place(identifier) -> function`

Produces an `on_place` callback for a specified log identifier  

#### Arguments

- `identifier` is the namespaced block ID. Namespace can be omitted, in which case `core.NAMESPACE` is substituted  

#### Return value

- A function that implements correct log placement logic to be used in block registrations  

### Function: `logdef.add_block(identifier, prototype) -> integer`

A wrapper for `blocks.add` to define a log block family  

#### Arguments

- `identifier` is the namespaced block ID. Namespace can be omitted, in which case `core.NAMESPACE` is substituted  

- `prototype` is the base (prototype) block definition which the function builds upon  

#### Return value

- Numeric block ID

#### Notes

- The registered block family will have an `orientation` state and variants for `north`, `east`, and `up` values of it  
