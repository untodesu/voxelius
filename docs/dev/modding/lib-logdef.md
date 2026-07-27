# Logs library

The `builtin:logdef` library provides helpers for registering log-like blocks.

## Importing

```lua
local logdef = require("builtin:logdef")
```

### Function: `logdef.on_place(identifier) -> function`

Produces an `on_place` callback for a given log identifier.

#### Arguments

- `identifier` is the namespaced block ID. If the namespace is omitted, `core.NAMESPACE` is used.

#### Return value

- A function that implements log placement logic for block registration

### Function: `logdef.add_block(identifier, prototype) -> integer`

A wrapper around `blocks.add` that defines a log block family.

#### Arguments

- `identifier` is the namespaced block ID. If the namespace is omitted, `core.NAMESPACE` is used.
- `prototype` is the base block definition the function builds on

#### Return value

- Numeric block ID

#### Notes

- The registered block family gets an `orientation` state and variants for `north`, `east`, and `up`.

![](logdef-states.png)
