#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include "ma.h"

#define PARAMETER_FAILED(x) { errno = EINVAL; return x; }

void MEMORY_FREE(void *ptr, ...) {
    va_list args;
    va_start(args, ptr);
    
    // check if the first argument is null (no arguments passed)
    if (ptr != NULL) {
        free(ptr);
    }
    
    // loop through the remaining arguments
    void *next_ptr;
    while ((next_ptr = va_arg(args, void*)) != NULL) {
        free(next_ptr);
    }
    
    va_end(args);
}

#define MEMORY_FAILED(x, ...) { \
    errno = ENOMEM; \
    MEMORY_FREE(__VA_ARGS__); \
    return x; \
}

// get ceil for division of two numbers
static int ceil_div(size_t f, size_t s) {
    return (f + s - 1) / s;
}

/** INITIALIZATION **/

// function to initialize a new state struct
// uses : ceil_div() to get a upper bound for size of all blocks
static state_t new_state(size_t n) {
    int blocksize = ceil_div(n , 64);
    return (state_t){.array = (uint64_t*)calloc(blocksize, sizeof(uint64_t)), .blockSize = blocksize, .bitsSize = n};
}

static machines_array_t new_machines_array(size_t n, moore_t *prevptr) {
    machines_array_t m = (machines_array_t){.capacity = n, .size = n, .array = (moore_t **)realloc(prevptr,sizeof(moore_t*) * n)};
    if(n == 0) m.array = NULL;
    return m;
}

#define INIT_MACHINE_VALUE 0

static machines_array_t* new_child_machines(size_t n) {
    machines_array_t *array = (machines_array_t*)malloc(sizeof(machines_array_t) * n);
    for(size_t i = 0; i < n; i++) {
        array[i] = new_machines_array(INIT_MACHINE_VALUE, NULL);
    }
    return array;
}

static int update_output(moore_t *machine) {
    uint64_t *outputarray = (uint64_t*)calloc(machine->outputState.blockSize, sizeof(uint64_t));
    if(!outputarray) MEMORY_FAILED(-1, NULL);
    machine->outputFunction(outputarray, machine->curState.array, machine->outputState.bitsSize, machine->curState.bitsSize);
    machine->outputState.array = outputarray;
    return 0; // Return success or handle other cases as needed
}


moore_t *ma_create_full(size_t n, size_t m, size_t s, transition_function_t t, output_function_t y, uint64_t const *q){
    // check if the parameters are correct
    if(!s || !m || !t || !y || !q) 
        PARAMETER_FAILED(NULL);

    // define a pointer to our new machine
    moore_t *newmachine = (moore_t*)malloc(sizeof(moore_t));
    
    // define a new machine
    *newmachine = (moore_t){
        .inputState = new_state(n),
        .curState = new_state(s),
        .outputState = new_state(m),
        .transitionFunction = t,
        .outputFunction = y,
        .parentMachines = new_machines_array(n, NULL),
        .childMachines = new_child_machines(m)
    };

    //UPD states

    // if n == 0 we set the pointer for input to null to avoid calloc malfunction
    if(n == 0) newmachine->inputState.array = NULL;
    // set cur state of machine
    newmachine->curState.array = q;
    // set output state
    if (update_output(newmachine) < 0) {
        MEMORY_FAILED(NULL, newmachine->inputState.array, newmachine->curState.array, newmachine->outputState.array,
                      newmachine->parentMachines.array, newmachine->childMachines);
    }

    // check for correct memory allocation
    if((!newmachine->inputState.array && n != 0) || !newmachine->curState.array || !newmachine->outputState.array ||
    !newmachine->parentMachines.array || !newmachine->childMachines) 
        MEMORY_FAILED(NULL,newmachine->inputState.array, newmachine->curState.array, newmachine->outputState.array,
        newmachine->parentMachines.array, newmachine->childMachines);

    return newmachine;
}

// declare a mirror function for ma_create_simple
static void mirror_function(uint64_t *output, uint64_t const *state, size_t m , size_t some) {
    // mark as used
    (void)some;
    // calculate the number of 64-bit blocks
    int block = ceil_div(m, 64);
    // copy the correct number of bytes
    memcpy(output, state, block * sizeof(uint64_t));
}

moore_t *ma_create_simple(size_t n, size_t m, transition_function_t t){
    // check if m == 0 - we don't want to calloc empty array
    if(m == 0) 
        PARAMETER_FAILED(NULL);

    uint64_t const *emptyarray = (uint64_t*)calloc(ceil_div(m, 64), sizeof(uint64_t));
    return ma_create_full(n, m, m, t, mirror_function, emptyarray);
}

/** DELETING **/

void ma_delete(moore_t *a){

    free(a->parentMachines.array);
    moore_t machine = *a;
    for(int i = 0; i < a->outputState.bitsSize; i++) {
        free(machine.childMachines[i].array);
    }
    free(machine.childMachines);

    free((void *)a->inputState.array);
    free((void *)a->curState.array);
    free((void *)a->outputState.array);

    free(a);
}

/** SETTING **/

int ma_set_input(moore_t *a, uint64_t const *input)
{
    return 0;
}

int ma_set_state(moore_t *a, uint64_t const *state)
{
    return 0;
}