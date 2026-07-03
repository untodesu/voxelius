# Modding: Block collision models

Collision models describe the physical shape of a block used for touch/movement response independent from its render geometry;

## Top-level fields

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`parent`|`string`|no|N/D|Name of an another collision model to inherit|  
|`elements`|`object[]`|depends|`[]`|List of AABBs that make the model up|  

## Element object

|Field|Type|Required|Default|Description|  
|----|----|----|----|----|  
|`from`|`number[3]`|yes|N/D|AABB start corner in 1/16ths of a block|  
|`to`|`number[3]`|yes|N/D|AABB end corner in 1/16ths of a block|  

## Example

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
