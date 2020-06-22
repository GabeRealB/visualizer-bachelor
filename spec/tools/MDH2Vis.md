## Usage
```
mdh2vis [-d directory | --directory directory]
```

### Input
- ``model.json``
- ``gemm_mdh.json``
- ``gemm_tps.json``
- ``stencil_mdh.json``
- ``stencil_tps.json``

### Output
- ``visconfig.json``

## Specification

### `model.json`

```antlr
ascii_t
    : '"' [\x20-\x7E]* '"'
    ;

uint8_t
    : [0, 255]
    ;

color_t
    : '[' uint8_t ',' uint8_t ',' uint8_t ',' uint8_t ']'
    ;

colors_t
    :   '{'
            '"tile:' color_t ','
            '"memory:' color_t ','
            '"thread:' color_t ','
            '"tile_out_of_border:' color_t ','
            '"thread_out_of_border:' color_t
        '}'
    ;

layer_t
    :   '{'
            '"name:' ascii_t ','
            '"name_threads:' ascii_t ','
            '"name_memory:' ascii_t ','
            '"name:' colors_t
        '}'
    ;

model_t
    :   '{'
            '"layer 0:' layer_t ','
            '"layer 1:' layer_t ','
            '"layer 2:' layer_t
        '}'
    ;
```

### ``gemm_mdh.json``

### ``gemm_tps.json``

````antlr
boolean_t
    : 'true'
    | 'false'
    ;

uint8_t
    : [0, 255]
    ;

uint32_t
    : [0, 2^32 - 1]
    ;

vec2_u8_t
    : '[' uint8_t ',' uint8_t ']'
    ;

vec3_u8_t
    : '[' uint8_t ',' uint8_t ',' uint8_t ']'
    ;

vec3_u32_t
    : '[' uint32_t ',' uint32_t ',' uint32_t ']'
    ;

sig_buffer_t
    :   '{'
            '"A":' vec2_u8_t ','
            '"B":' vec2_u8_t
        '}'
    ;

mem_input_t
    :   '{'
            '"A":' uint8_t ','
            '"B":' uint8_t
        '}'
    ;

mem_result_t
    :   '{'
            '"C":' uint8_t
        '}'
    ;

layer_t
    :   '{'
            '"SIG_array_to_OCL":' vec3_u8_t ','
            '"SIG_mdh":' vec3_u8_t ','
            '"SIG_buffer-do":' sig_buffer_t ','
            '"TILE_SIZE":' vec3_u32_t ','
            '"NUM_THREADS":' vec3_u32_t ','
            '"MEM_REGION_INP":' mem_input_t ','
            '"MEM_REGION_RES":' mem_result_t ','
            '"CMB_RES":' boolean_t
        '}'
    ;

gemm_tps_t
    :   '{'
            '"layer 0:' layer_t ','
            '"layer 1:' layer_t ','
            '"layer 2:' layer_t
        '}'
    ;
````

### ``stencil_mdh.json``

### ``stencil_tps.json``
