# Tint API

Each mod can register a named tint. Tints are used to paint specific block faces during rendering with a biome-specific color. This allows for there to be only a single block for stuff like grass, bushes and water

## Constant: null tint

`tints.NULL_TINT` is an empty, undefined or otherwise invalid tint ID. From the gamedev and gameplay point of view, this value is to be treated as a "no tint" marker for block models and fluids

## Functions

### Function: `tints.add(name, def) -> integer`

Registers a new named tint

#### Arguments

- `name` is the namespaced tint ID
- `def` is the tint definition table

#### Return value

Returns the numeric tint ID.

#### Notes

**Rename rule:** on conflict, the encroaching `name` gets a `~N` suffix. `N` is the smallest integer starting at `1` that yields a free id (e.g. `mymod:mytint~1`). Loading stays deterministic for the same mod list and order.

## Tint definition

|Field|Type|Required|Default|Description|
|----|----|----|----|----|
|`default_color`|`number[3]`|yes|N/D|Default tint color when unspecified by a biome|  
