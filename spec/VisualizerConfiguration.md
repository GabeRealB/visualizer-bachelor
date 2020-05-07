## Specification
```antlr
uint8_type
    : [0, 255]
    ;

uint32_type
    : [0, 2^32 - 1]
    ;

int32_type
    : [-2^31, 2^31 - 1]
    ;

boolean_type
    : 'true'
    | 'false'
    ;

color_type
    : '[' uint8_type ',' uint8_type ','uint8_type ']'
    ;

size_type
    : '[' uint32_type ',' uint32_type ','uint32_type ']'
    ;

position_type
    : '[' int32_type ',' int32_type ','int32_type ']'
    ;

tiling_info_type
    : '[' uint32_type ',' uint32_type ','uint32_type ']'
    ;

traversal_order_type
    : '12'
    | '21'
    | '102'
    | '201'
    | '120'
    | '210'
    ;

inner_cube_type
    :   '{'   
            '"color":' color_type ',' 
            '"tiling":' tiling_info_type ','
            ('"traversal_order":' traversal_order_type ',')?
            '"inner_cubes": [' (inner_cube_type ',')* ']' 
        '}'
    ;

outer_cube_type
    :   '{'
            '"position":' position_type ',' 
            '"size":' size_type ',' 
            '"color":' color_type ',' 
            '"tiling":' tiling_info_type ','
            ('"traversal_order":' traversal_order_type ',')?
            '"inner_cubes": [' (inner_cube_type ',')* ']' 
        '}'
    ;

visualizer_configuration_type
    :   '{'
            '"resolution": [' uint32_type ',' uint32_type '],'
            '"fullscreen": ' boolean_type ','
            '"cubes": [' (outer_cube_type ',')* ']'
        '}'
    ;
```

## Example
````json
{
    "resolution" : [1920, 1080],
    "fullscreen" : false,
    "cubes": [
        {
            "position": [1,1,1],
            "size": [5,5,5],
            "tiling": [2,2,2],
            "color": [255,255,255],
            "traversal_order": 102,
            "inner_cubes": [
                {
                    "tiling": [2,2,2],
                    "color": [255,255,255],
                    "traversal_order": 102,
                    "inner_cubes": []
                }
            ]
        }
    ]
}
````