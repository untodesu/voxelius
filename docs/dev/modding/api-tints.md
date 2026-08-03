# Tint API

Each mod can register a named tint. Tints paint specific block faces during rendering with a biome-specific color. One block type can serve grass, bushes, water, and similar surfaces.

## Constant: null tint

`tints.NULL_TINT` is an empty, undefined, or otherwise invalid tint ID. Treat it as a "no tint" marker for block models and fluids in gameplay and gamedev.

## Functions

### Function: `tints.add(name, def) -> integer`

Registers a new named tint.

#### Arguments

- `name` is the namespaced tint ID
- `def` is the tint definition table

#### Return value

Returns the numeric tint ID.

#### Notes

**Rename rule:** on conflict, the engine adds a `~N` suffix to the encroaching `name`. `N` is the smallest integer, starting at `1`, that gives a free id (e.g. `mymod:mytint~1`). Loading stays deterministic for the same mod list and order.

## Tint definition

|Field|Type|Required|Default|Description|
|----|----|----|----|----|
|`default_color`|`number[3]`|yes|N/D|Default tint color when unspecified by a biome|
