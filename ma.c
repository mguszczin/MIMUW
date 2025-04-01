#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include "ma.h"

#define BLOCK_SIZE 64

// TO DO COSMETICS: 
// DEF VALUE OF EMPTY ARRAY TO NULL

// get ceil for division of two numbers
static size_t ceil_div(size_t f, size_t s) {
    return (f + s - 1) / s;
}

/** ERROR HANDLING **/

#define PARAMETER_FAILED(x) { errno = EINVAL; return x; }

// free all the pointers given as arguments
void memory_free(void *ptr, ...) {
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
    memory_free(__VA_ARGS__); \
    return x; \
}

/** MACHINES_ARRAY_T HANDLING **/

// function generates new machines array 
// if n = 0 return m.array equal to NULL
static machines_array_t new_machines_array(size_t nsize, size_t ncapacity, machines_array_t *prevptr) {
    bit_location_t *newarray = (bit_location_t *)realloc(prevptr->array ,sizeof(bit_location_t) * ncapacity);

    machines_array_t m = (machines_array_t){.capacity = ncapacity, .size = nsize, .array = newarray};

    if(ncapacity == 0) m.array = NULL;
    return m;
}

// add new element to machines_array, if array to small - resize to 2x + 1 bigger 
// uses : new_machines_array to resize
// returns -1 if allocation went wrong else 0 
static int add_element_to_machines_array(machines_array_t *mptr, bit_location_t ma) {
    // change capacity to 2*x + 1 if its to small 
    if(mptr->size >= mptr->capacity) {
        *mptr = new_machines_array(mptr->size, mptr->capacity * 2 + 1, mptr);                     // +1 to avoid case where capacity = 0
        if(mptr->array == NULL) MEMORY_FAILED(-1,NULL);
    }

    // add element 
    mptr->array[mptr->size] = ma;
    mptr->size++;
    return 0;
}

static bool same_bit_location(bit_location_t a, bit_location_t b) {
    return a.bitPos == b.bitPos && a.block == b.block;
}

static int remove_element_from_machines_array(machines_array_t *mptr, bit_location_t ma) {
    for(size_t i = 0; i < mptr->size; i++) {
        if(!same_bit_location(ma, mptr->array[i])) continue;

        // move all elements to the left
        for (size_t j = i; j < mptr->size - 1; j++) {
            mptr->array[j] = mptr->array[j+1];
        }
        mptr->size--;
        i--; // adjust i to recheck te moved element 
    }

    size_t target_capacity = mptr->size + 2;
    if (mptr->capacity > target_capacity) {
        *mptr = new_machines_array(mptr->size, target_capacity, mptr);
        if(mptr->size != 0 && (mptr->array) == NULL) return -1;
    }
    return 0;

}

/** INITIALIZATION **/

#define EMPTY_MACHINE_ARRAY (machines_array_t){.size =0, .capacity = 0, .array = NULL}

// function to initialize parents array 
// initially each bit chooses itself as parent 
static machines_array_t init_parent_machines(size_t n, moore_t *newmachine) {
    if(n == 0) return EMPTY_MACHINE_ARRAY;

    machines_array_t m = (machines_array_t){.capacity = n, .size = n, .array = (bit_location_t *)malloc(sizeof(bit_location_t) * n)};
    
    if(m.array == NULL) return m;           // if array points to null we are done 

    for(size_t i = 0; i < n; i++) {
        int blind = ceil_div(i, BLOCK_SIZE);        // get block index 
        int indinside = i % BLOCK_SIZE;             // get index inside block
        m.array[i] = (bit_location_t){.block = newmachine->inputState.array[blind], indinside};
    }
}

// function intializes array of arrays of machines, sets every machine to be empty 
static machines_array_t* init_child_machines(size_t n) {
    machines_array_t *array = (machines_array_t*)malloc(sizeof(machines_array_t) * n);
    for(size_t i = 0; i < n; i++) {
        array[i] = EMPTY_MACHINE_ARRAY;
    }
    return array;
}

// function to initialize a new state struct
// uses : ceil_div() to get a upper bound for size of all blocks
static state_t new_state(size_t n) {
    int blocksize = ceil_div(n , BLOCK_SIZE);
    return (state_t){.array = (uint64_t*)calloc(blocksize, sizeof(uint64_t)), .blockSize = blocksize, .bitsSize = n};
}

// Calc the new output and then assign it to the array machine points to 
static int get_new_output(moore_t *ma) {
    uint64_t *outputarray = (uint64_t*)calloc(ma->outputState.blockSize, sizeof(uint64_t));
    if(!outputarray) MEMORY_FAILED(-1, NULL);

    ma->outputFunction(outputarray, ma->curState.array, ma->outputState.bitsSize, ma->curState.bitsSize);
    memcpy(ma->outputState.array, outputarray, ma->outputState.blockSize * sizeof(uint64_t));

    free(outputarray);
    return 0; // Return success or handle other cases as needed
}


moore_t *ma_create_full(size_t n, size_t m, size_t s, transition_function_t t, output_function_t y, uint64_t const *q){
    // check if the parameters are correct
    if(!s || !m || !t || !y || !q) 
        PARAMETER_FAILED(NULL);

    // create a pointer to our new machine
    moore_t *newmachine = (moore_t*)malloc(sizeof(moore_t));
    
    // define a new machine
    *newmachine = (moore_t){
        .inputState = new_state(n),         // set state of input for n 
        .curState = new_state(s),           // set state of current state for s
        .outputState = new_state(m),        // set state of output for m
        .transitionFunction = t,            // set both functions
        .outputFunction = y
    };
    newmachine->parentMachines = init_parent_machines(n, newmachine);  // structure to know 'parent' of every bit in inputState
    newmachine->childMachines = init_child_machines(m);          // structure to track 'child' for every bit in outputState

    //UPD states

    // if n == 0 we set the pointer for input to null to avoid calloc malfunction
    if(n == 0) newmachine->inputState.array = NULL;
    // set cur state of machine
    newmachine->curState.array = q;

    // check for correct memory allocation and update_output
    // if not correct - free all of the adresses 
    if(get_new_output(newmachine) < 0 || (!newmachine->inputState.array && n != 0) || 
    !newmachine->curState.array || !newmachine->outputState.array ||
    !newmachine->parentMachines.array || !newmachine->childMachines) 
        MEMORY_FAILED(
            NULL,
            newmachine->inputState.array, 
            newmachine->curState.array, 
            newmachine->outputState.array,
            newmachine->parentMachines.array, 
            newmachine->childMachines);

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

moore_t *ma_create_simple(size_t n, size_t m, transition_function_t t) {
    // check if m == 0 - we don't want to calloc empty array
    if(m == 0) 
        PARAMETER_FAILED(NULL);

    uint64_t const *emptyarray = (uint64_t*)calloc(ceil_div(m, 64), sizeof(uint64_t));
    return ma_create_full(n, m, m, t, mirror_function, emptyarray);
}

/** DELETING **/

void ma_delete(moore_t *a) {

    free(a->parentMachines.array);
    moore_t machine = *a;
    for(int i = 0; i < a->outputState.bitsSize; i++) {
        for(size_t j = 0; j < machine.childMachines[i].size; j++) {
            moore_t *mptr = machine.childMachines[i].array[j];
            remove_element_from_machines_array(&mptr->parentMachines, a);
        }
        free(machine.childMachines[i].array);
    }
    free(machine.childMachines);

    free((void *)a->inputState.array);
    free((void *)a->curState.array);
    free((void *)a->outputState.array);

    free(a);
}

/** CONNECTING **/

int ma_connect(moore_t *a_in, size_t in, moore_t *a_out, size_t out, size_t num) {
    return 0;
}

int ma_disconnect(moore_t *a_in, size_t in, size_t num)
{
    return 0;
}

/** SETTING **/

int ma_set_input(moore_t *a, uint64_t const *input){
    free(a->inputState.array);
    a->inputState.array = input;
    return 0;
}

int ma_set_state(moore_t *a, uint64_t const *state){
    free(a->curState.array);
    a->curState.array = NULL;
    a->curState.array = state;
    if(get_new_output(a) < 0) 
        MEMORY_FAILED(-1, NULL);
    return 0;
}

