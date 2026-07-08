# Modding: block collisions

Block collisions define the physical shape of a block that's used for touch/movement response; these models are independent from the block's render geometry.  

## Top-level fields

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`elements`|`object[]`|yes|N/D|List of axis-aligned boxes that make the model|  

## Element object

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`from`|`number[3]`|yes|N/D|AABB start in 1/16ths of a block|  
|`to`|`number[3]`|yes|N/D|AABB end in 1/16ths of a block|  

## Examples

### Example: cube collision

```json
{
  "elements": [
    {
      "from": [0, 0, 0],
      "to": [16, 16, 16]
    }
  ]
}
```

### Example: slab collision

```json
{
  "elements": [
    {
      "from": [0, 0, 0],
      "to": [16, 8, 16]
    }
  ]
}
```

### Example: stairs collision

```json
{
  "elements": [
    {
      "from": [0, 0, 0],
      "to": [16, 8, 16]
    },
    {
      "from": [0, 8, 0],
      "to": [8, 16, 16],
    }
  ]
}
```
