## Usage
```
mdh2vis --model model-path --mdh mdh-path --tps tps-path [-out out-dir]
```

### Input
- ``model.json``
- ``mdh.json``
- ``tps.json``

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
            '"colors:' colors_t
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

### ``mdh.json``

```antlr
ascii_t
    : '"' [\x20-\x7E]* '"'
    ;

combine_operators_t
    :   '['
            (ascii_t ',')*
            ascii_t
        ']'
    ;

constant_expr_t
    :   [0, 2^32 - 1]
    ;

operator_expr_t
    :   '+'
    |   '-'
    |   '*'
    |   '/'
    ;

component_expr_t
    :   '"' 'i(1|2|3)' (operator_expr_t constant_expr_t)? '"'
    |   constant_expr_t
    ;

expr_tup_t
    :   '['
            component_expr_t ','
            component_expr_t ','
            component_expr_t
        ']'
    ;

expr_tup_array_t
    :   '['
            (expr_tup_t ',')*
            expr_tup_t
        ']'
    ;

expr_map_t
    :   '{'
            (ascii_t ':' expr_tup_array_t ',')
            ascii_t ':' expr_tup_array_t ','
        '}'
    ;

mdh_in_t
    :   '{'
            '"combine operators":' combine_operators_t 
        '}'
    ;

views_t
    :   '{'
            '"input":' expr_map_t ','
            '"output":' expr_map_t
        '}'
    ;

mdh_t
    :   '{'
            '"MDH":' mdh_t ','
            '"views":' views_t
        '}'
    ;
```

### ``tps.json``

```antlr
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

tps_t
    :   '{'
            '"layer 0:' layer_t ','
            '"layer 1:' layer_t ','
            '"layer 2:' layer_t
        '}'
    ;
```
