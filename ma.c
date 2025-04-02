#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include "ma.h"

#define BLOCK_SIZE 64

#define EMPTY_ARRAY NULL 
#define EMPTY_STACK NULL
#define EMPTY_CONNECT (connect_elem_t){.connectedMachine = NULL, .connectedBit = -1}
#define EMPTY_LIST (machines_stack_t){.top = NULL}

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

/** MACHINES_STACK HANDLING **/

static void unlink_node(node_t *nodetodel, machines_stack_t *stack) {
    // if stack is empty we don't do anything
    if(stack->top == NULL) {
        return;
    }  

    // if we delete top of the we just set the top to prev (might be null)
    if(stack->top == nodetodel) {
        stack->top = nodetodel->prev;
        return;
    }
    
    // we iterate over whole list looking for that node 
    node_t *curnode = stack->top, *prevnode = NULL;
    while(curnode != NULL && curnode != nodetodel) {
        node_t *ptr = curnode;
        curnode = curnode->prev;
        prevnode = ptr;
    }

    // if we don't find the node we simply return
    if(curnode == NULL) return;

    // we unlink the pointer
    prevnode->prev = curnode->prev;
}

// we add node to the back of the list 
static void push_node(node_t *node, machines_stack_t *stack) {
    // case where stack is empty
    if(stack->top == NULL){
        stack->top = node;
        node->prev = NULL;  // make sure last element in stack points to null 
        return;
    }

    // if stack isn't empty  
    node_t *prevnode = stack->top;
    stack->top = node;
    node->prev = prevnode;
}


/** INITIALIZATION **/

// function to initialize parents array 
// initially we set each bit to NULL - because we don't have any parents
static connect_elem_t *init_parent_machines(size_t n) {
    if(n == 0) return EMPTY_ARRAY;

    connect_elem_t *parentarray = (connect_elem_t *)malloc(sizeof(connect_elem_t) * n);
    if(parentarray == NULL) return EMPTY_ARRAY;

    for(size_t i = 0; i < n; i++) {
        parentarray[i] = EMPTY_CONNECT;
    }
    
    return parentarray;
}

// function intializes array of arrays of machines, sets every machine to be empty 
static machines_stack_t* init_child_machines(size_t m) {
    machines_stack_t *childarray = (machines_stack_t *)malloc(sizeof(machines_stack_t) * m);
    if(childarray == NULL) return EMPTY_ARRAY;

    for(size_t i = 0; i < m; i++) {
        childarray[i] = EMPTY_LIST;
    }

    return childarray;
}

// function to initialize a new state struct
// uses : ceil_div() to get a upper bound for size of all blocks
static state_t init_state(size_t n) {
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
        .inputState = init_state(n),                // set state of input for n 
        .curState = init_state(s),                  // set state of current state for s
        .outputState = init_state(m),               // set state of output for m
        .transitionFunction = t,                    // set both functions
        .outputFunction = y,
        .parentMachines = init_parent_machines(n),  // structure to know 'parent' of every bit in inputState
        .childMachines = init_child_machines(m)     // structure to track 'child' for every bit in outputState

    };

    //UPD states

    // if n == 0 we set the pointer for input to null to avoid calloc malfunction
    if(n == 0) newmachine->inputState.array = NULL;
    // set cur state of machine
    newmachine->curState.array = q;

    // check for correct memory allocation and update_output
    // if not correct - free all of the adresses 
    if(get_new_output(newmachine) < 0 || (!newmachine->inputState.array && n != 0) || 
    !newmachine->curState.array || !newmachine->outputState.array ||
    !newmachine->parentMachines || !newmachine->childMachines) 
        MEMORY_FAILED(
            NULL,                               // return value if fail 
            newmachine->inputState.array,       // structs to free
            newmachine->curState.array, 
            newmachine->outputState.array,
            newmachine->parentMachines, 
            newmachine->childMachines);

    return newmachine;
}

// declare a mirror function for ma_create_simple
static void identity_function(uint64_t *output, uint64_t const *state, size_t m , size_t some) {
    // mark as used
    (void)some;
    // calculate the number of 64-bit blocks
    int block = ceil_div(m, BLOCK_SIZE);
    // copy the correct number of bytes
    memcpy(output, state, block * sizeof(uint64_t));
}

moore_t *ma_create_simple(size_t n, size_t m, transition_function_t t) {
    // check if m == 0 - we don't want to calloc empty array
    if(m == 0) 
        PARAMETER_FAILED(NULL);

    uint64_t const *emptyarray = (uint64_t*)calloc(ceil_div(m, BLOCK_SIZE), sizeof(uint64_t));
    return ma_create_full(n, m, m, t, identity_function, emptyarray);
}

/* CONNECTING/DISCONNECTING */

static int connect_machines_bits(connect_elem_t *)

static int disconnect_machines_bits()

int ma_connect(moore_t *a_in, size_t in, moore_t *a_out, size_t out, size_t num) {
    return 0;
}

int ma_disconnect(moore_t *a_in, size_t in, size_t num)
{
    return 0;
}

/** DELETING **/

void ma_delete(moore_t *a) {

    
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

uint64_t const * ma_get_output(moore_t const *a){
    return a->outputState.array;
}

int ma_step(moore_t *at[], size_t num)
{
    return 0;
}
