# Slabs Library

The `builtin:slabdef` library provides helpers for registering slab variants of regular blocks.

## Importing

```lua
local slabdef = require("builtin:slabdef")
```

## Functions

### Function: `slabdef.on_place(identifier) -> function`

Produces an `on_place` callback for a given slab identifier.

#### Arguments

- `identifier` is the namespaced block ID. If the namespace is omitted, `core.NAMESPACE` is used.

#### Return value

- A function that implements slab placement logic for block registration

### Function: `slabdef.add_block(identifier, prototype, options) -> integer`

A wrapper around `blocks.add` that defines a slab block family.

#### Arguments

- `identifier` is the namespaced block ID. If the namespace is omitted, `core.NAMESPACE` is used.
- `prototype` is the base block definition the function builds on
- `options` is an optional table with extra parameters

#### Return value

- Numeric block ID

#### Notes

- The registered block family gets an `orientation` state and variants for `top`, `bottom`, and `double`.
- The function does not modify touch response values from the prototype. It does define block collisions.
