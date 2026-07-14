# File Format: Block Collisions

Block collisions define the physical shape of a block that's used for touch/movement response; these models are independent from the block's render geometry  

Block collisions are located in the directory: `namespace:collisions/block`  

## Top-level fields

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`elements`|`object[]`|yes|N/D|List of axis-aligned boxes that make the model|  

## Element object

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`min`|`number[3]`|yes|N/D|AABB start in 1/16ths of a block|  
|`max`|`number[3]`|yes|N/D|AABB end in 1/16ths of a block|  

## Examples

### Example: cube collision

```json
{
  "elements": [
    {
      "min": [0, 0, 0],
      "max": [16, 16, 16]
    }
  ]
}
```

### Example: slab collision

```json
{
  "elements": [
    {
      "min": [0, 0, 0],
      "max": [16, 8, 16]
    }
  ]
}
```

### Example: stairs collision

```json
{
  "elements": [
    {
      "min": [0, 0, 0],
      "max": [16, 8, 16]
    },
    {
      "min": [0, 8, 0],
      "max": [8, 16, 16],
    }
  ]
}
```
