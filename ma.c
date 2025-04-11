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

// struct used to define machines array

/* ARRAY/LIST ELEMENT DEFINITION */

typedef struct connect_elem {
    moore_t *connectedMachine;     // pointer to connected machine
    size_t connectedBit;           // index of bit it's connected to
} connect_elem_t;

/* LIST DEFINITION */

typedef struct node {
    connect_elem_t element;        // data stored in this node
    struct node *prev;             // pointer to previous node (linked list)
} node_t;

typedef struct machines_stack {
    node_t *top;                   // pointer to the top of the stack
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


// get ceil for division of two numbers
static size_t ceil_div(size_t f, size_t s) {
    return (f + s - 1) / s;
}

static bool same_connect_elems(connect_elem_t a, connect_elem_t b) {
    return a.connectedBit == b.connectedBit && a.connectedMachine == b.connectedMachine;
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

static void delete_node(connect_elem_t nodetodel, machines_stack_t *stack) {
    // if stack is empty we don't do anything
    node_t *curnode = stack->top, *usednode = NULL;
    if(curnode == NULL) {
        return;
    }  

    // if we delete top of the we just set the top to prev (might be null)
    if(same_connect_elems(curnode->element, nodetodel)) {
        stack->top = curnode->prev;
        free(curnode);
        return;
    }
    
    // we iterate over whole list looking for that node 
    while(curnode != NULL && 
          !same_connect_elems(curnode->element, nodetodel)) {
        node_t *ptr = curnode;
        curnode = curnode->prev;
        usednode = ptr;
    }

    // if we don't find the node we simply return
    if(curnode == NULL) return;

    // we unlink the pointer
    usednode->prev = curnode->prev;
    free(curnode);
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
static connect_elem_t *init_parent_machines_array(size_t n) {
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

static connect_elem_t init_connect_elem(size_t bit, moore_t *ma) {
    return (connect_elem_t){.connectedBit = bit, .connectedMachine = ma};
}

static node_t init_node(connect_elem_t elem) {
    return (node_t){.element = elem, .prev = NULL};
}

// function to initialize a new state struct
// uses : ceil_div() to get a upper bound for size of all blocks
static state_t init_state(size_t n) {
    int blocksize = ceil_div(n , BLOCK_SIZE);
    return (state_t){.array = (uint64_t*)calloc(blocksize, sizeof(uint64_t)), .blockSize = blocksize, .bitsSize = n};
}

// Calc the new output and then assign it to the array machine points to 
static void get_new_output(moore_t *ma) {
    ma->outputFunction(ma->outputState.array, ma->curState.array, ma->outputState.bitsSize, ma->curState.bitsSize);
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
        .parentMachines = init_parent_machines_array(n),  // structure to know 'parent' of every bit in inputState
        .childMachines = init_child_machines(m)     // structure to track 'child' for every bit in outputState

    };

    //UPD states

    // if n == 0 we set the pointer for input to null to avoid calloc malfunction
    if(n == 0) newmachine->inputState.array = NULL;
    // set cur state of machine
    memcpy(newmachine->curState.array, q, sizeof(uint64_t) * s);

    // check for correct memory allocation and update_output
    // if not correct - free all of the adresses 
    get_new_output(newmachine);
    if((!newmachine->inputState.array && n != 0) || 
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

// declare a identity function for ma_create_simple
static void identity_function(uint64_t *output, uint64_t const *state, size_t m , size_t some) {
    // mark as used
    (void)some;
    // calculate the number of 64-bit blocks
    int block = ceil_div(m, BLOCK_SIZE);
    // copy the correct number of bytes
    memcpy(output, state, block * sizeof(uint64_t));
}

// MAKE SURE IT WILL WORK CORRECTLY WITH SIMPLE ASSIGIN
moore_t *ma_create_simple(size_t n, size_t m, transition_function_t t) {
    // check if m == 0 - we don't want to calloc empty array
    if(m == 0) 
        PARAMETER_FAILED(NULL);

    uint64_t const *emptyarray = (uint64_t*)calloc(ceil_div(m, BLOCK_SIZE), sizeof(uint64_t));
    return ma_create_full(n, m, m, t, identity_function, emptyarray);
}

/* CONNECTING/DISCONNECTING */

// return -1 if malloc failed else 0 
// we connect two bits and machines together
static int connect_machines_bits(size_t parbit, moore_t *par, size_t chilbit, moore_t *chil){
    connect_elem_t parelem = init_connect_elem(parbit, par);
    connect_elem_t chilelem = init_connect_elem(chilbit, chil);

    // we check if we add the same connection twice
    if(same_connect_elems(chil->parentMachines[chilbit], parelem)) return 0;   

    // get a pointer to a new node and initialize 
    node_t *newnode = (node_t *)malloc(sizeof(node_t));
    if(newnode == NULL) return -1;                  // return -1 if allocation failed
    *newnode = init_node(chilelem);

    // update parent array and add node to list
    chil->parentMachines[chilbit] = parelem;
    push_node(newnode, &(par->childMachines[parbit]));

    return 0;
}

// function unlinks the connection between two bits
// sets the pointer to parent on child machine to null
// and frees the memory allocated on queue 
static void disconnect_machines_bits(size_t parbit, moore_t *par, size_t chilbit, moore_t *chil){
    connect_elem_t chilelem = init_connect_elem(chilbit, chil);

    chil->parentMachines[chilbit] = EMPTY_CONNECT;
    delete_node(chilelem, &(par->childMachines[parbit]));
}

int ma_disconnect(moore_t *a_in, size_t in, size_t num)
{
    bool badin = (in + num) >= a_in->inputState.bitsSize;
    if(badin || !a_in || !num)
        PARAMETER_FAILED(-1);
    
    for(size_t i = 0; i < num; i++) {
        size_t child = in + num;
        moore_t *prevout = a_in->parentMachines[child].connectedMachine;
        size_t prevbit = a_in->parentMachines[child].connectedBit;

        if(prevout != NULL) disconnect_machines_bits(prevbit, prevout, child, a_in);
    }
    return 0;
}

int ma_connect(moore_t *a_in, size_t in, moore_t *a_out, size_t out, size_t num) {
   
    if(!a_out || !a_in || !num)
        PARAMETER_FAILED(-1);
    
    // we define when range is bad
    bool badin = (in + num) >= a_in->inputState.bitsSize;
    bool badout = (out + num) >= a_out->outputState.bitsSize;
    if(badin || badout) 
        PARAMETER_FAILED(-1);
    
    // we disconnect all bits on this range 
    ma_disconnect(a_in, in, num);

    // for every bit we erase prev connection and add a new one 
    for(size_t i = 0; i < num; i++) {          
        size_t child = in + num, par = out + num;
        // add a new connection here 
        if(connect_machines_bits(par, a_out, child, a_in) < 0) MEMORY_FAILED(-1, NULL);
    }

    return 0;
}

/** DELETING **/

void ma_delete(moore_t *a) {
    if(a == NULL) return;

    if(a->inputState.bitsSize > 0) ma_disconnect(a, 0, a->inputState.bitsSize);  // disconnect all parents from child
    
    // disconnect all children 
    for(size_t i = 0; i < a->outputState.bitsSize; i++) {
        machines_stack_t *s = &(a->childMachines[i]);
        node_t *topnode = s->top;
        while(topnode != NULL) {
            size_t childbit = topnode->element.connectedBit;
            moore_t *child = topnode->element.connectedMachine;
            disconnect_machines_bits(i, a, childbit, child);
            topnode = topnode->prev;
        }
    }
    
    // once we disconnected everything we can free the pointers
    free(a->inputState.array);
    free(a->curState.array);
    free(a->outputState.array);
    free(a->childMachines);
    free(a->parentMachines);
    free(a);
}

/** SETTING **/

int ma_set_input(moore_t *a, uint64_t const *input){
    if(!a || !a->inputState.bitsSize || !input) PARAMETER_FAILED(-1);

    memcpy(a->inputState.array, input, sizeof(uint64_t) * a->inputState.blockSize);

    return 0;
}

int ma_set_state(moore_t *a, uint64_t const *state){
    if(!a || !state) PARAMETER_FAILED(-1);

    memcpy(a->curState.array, state, sizeof(uint64_t) * a->curState.blockSize);
    get_new_output(a);      // upd output

    return 0;
}

uint64_t const * ma_get_output(moore_t const *a){
    if(!a) PARAMETER_FAILED(NULL);

    return a->outputState.array;
}

uint64_t get_bit(state_t st, size_t bit) {
    uint64_t blockind = ceil_div(bit, BLOCK_SIZE), bitind = bit % BLOCK_SIZE;
    return (st.array[blockind] >> bitind) & 1;
}

int ma_step(moore_t *at[], size_t num)
{
    if(!at || !num) PARAMETER_FAILED(-1);

    for(size_t i = 0; i < num; i++) {
        moore_t *cur = at[i];
        if(!cur || !num) PARAMETER_FAILED(-1);

        uint64_t *inputarray = calloc(cur->inputState.blockSize, sizeof(uint64_t));
        if(inputarray == NULL) MEMORY_FAILED(-1, NULL);

        for(size_t bit = 0; bit < cur->inputState.bitsSize; bit++) {
            moore_t *machine = cur->parentMachines[bit].connectedMachine;
            uint64_t mybit;
            if(machine == NULL) mybit = get_bit(cur->inputState, bit);
            else mybit = get_bit(machine->outputState, cur->parentMachines[bit].connectedBit);
            inputarray[ceil_div(bit, BLOCK_SIZE)] |= (mybit << (bit % BLOCK_SIZE));
        }
        cur->transitionFunction(cur->curState.array, inputarray, cur->curState.array, 
        cur->inputState.bitsSize,cur->curState.bitsSize);
        free(inputarray);
    }

    for(size_t i = 0; i < num; i++) {
        moore_t *cur = at[i];
        get_new_output(cur);
    }
    return 0;
}
