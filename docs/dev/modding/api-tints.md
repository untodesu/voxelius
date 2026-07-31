# Tints API

## Constant: null tint

`tints.NULL_TINT` is an empty, undefined or otherwise invalid tint ID. Treat it as a "no tint" marker for blocks

## Functions

### Function: `tints.add(name, def) -> integer`

Registers a new tint

#### Arguments

- `name` is the namespaced tint ID
- `def` is the tint definition table

#### Return value

Returns the numeric tint ID.

## Tint definition

|Field|Type|Required|Default|Description|
|----|----|----|----|----|
|`default_color`|`number[3]`|yes|N/D|Default tint color when unspecified by a biome|  
