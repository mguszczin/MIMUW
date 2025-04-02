#ifndef MA_H
#define MA_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef void (*transition_function_t)(uint64_t *next_state, uint64_t const *input,
                                      uint64_t const *state, size_t n, size_t s);
typedef void (*output_function_t)(uint64_t *output, uint64_t const *state,
                                  size_t m, size_t s);
// struct representing current state of bit array
// array is split into 64 blocks
typedef struct state {  
    // bit array splited into 64 blocks
    uint64_t *array;
    // size of blocks in array 
    size_t blockSize;
    // amount of bits the array really has
    size_t bitsSize;
} state_t;


typedef struct moore moore_t;
// struct used to define machines array

/* ARRAY/LIST ELEMENT DEFINITION */

typedef struct connect_elem {
    moore_t *connectedMachine;
    size_t connectedBit;
} connect_elem_t;

/* LIST DEFINITION */

typedef struct node {
    connect_elem_t element;
    node_t *prev;
} node_t;

typedef struct machines_stack {
    node_t *top;
} machines_stack_t;

/* MAIN STRUCT DEFINITION */

// structure used to represent moore machine 
// uses: struct state, transition_function_t, output_function_t 
typedef struct moore {
    // all of the possible states of machine
    state_t inputState, curState, outputState;
    // function to get newState 
    transition_function_t transitionFunction;
    // function to get new output state
    output_function_t outputFunction; 
    // machines connected to our input
    connect_elem_t *parentMachines;
    // machines our output is connected to 
    machines_stack_t *childMachines;

} moore_t;


moore_t * ma_create_full(size_t n, size_t m, size_t s, transition_function_t t,
                         output_function_t y, uint64_t const *q);
moore_t * ma_create_simple(size_t n, size_t m, transition_function_t t);
void ma_delete(moore_t *a);
int ma_connect(moore_t *a_in, size_t in, moore_t *a_out, size_t out, size_t num);
int ma_disconnect(moore_t *a_in, size_t in, size_t num);
int ma_set_input(moore_t *a, uint64_t const *input);
int ma_set_state(moore_t *a, uint64_t const *state);
uint64_t const * ma_get_output(moore_t const *a);
int ma_step(moore_t *at[], size_t num);

#endif