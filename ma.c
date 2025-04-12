#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h> // for debug
#include "ma.h"

#define BLOCK_SIZE 64

#define EMPTY_ARRAY NULL 
#define EMPTY_CONNECT (connect_elem_t){.connected_machine = NULL, .connected_bit = -1}
#define EMPTY_STACK (machines_stack_t){.top = NULL}

/** ERROR HANDLING **/

#define PARAMETER_FAILED(x) { errno = EINVAL; return x; }

#define MEMORY_FAILED(x)  { errno = ENOMEM; return x; }

/** STRUCTURES DEFINITIONS **/

// struct representing current state of bit array
// array is split into 64 blocks
typedef struct state {
    // bit array splited into 64 blocks
    uint64_t *array;

    // size of blocks in array 
    size_t block_count;

    // amount of bits the array really has
    size_t bits_count;

} state_t;

/* ARRAY/LIST ELEMENT DEFINITION */

// struct to represent the bit on machine that we are connected to
// uses : moore_t
typedef struct connect_elem {
    // pointer to connected machine
    moore_t *connected_machine;

    // index of bit it's connected to
    size_t connected_bit;

} connect_elem_t;

/* LIST DEFINITION */

// single node of our stack
// uses connect_elem
typedef struct node {
    // data stored in this node
    connect_elem_t element;

    // pointer to previous node
    struct node *prev;  

} node_t;

// stack
// uses : node
typedef struct machines_stack {
    // pointer to the top of the stack
    node_t *top;

} machines_stack_t;

/* MAIN STRUCT DEFINITION */

// structure used to represent moore machine 
// uses: struct state, transition_function_t, output_function_t 
// for every bit in out machine we will rememeber the bits  
typedef struct moore {
    // all of the possible states of machine
    state_t input_state, current_state, output_state;

    // function to get newState 
    transition_function_t transition_function;

    // function to get new output state
    output_function_t output_function;

    // machines connected to our input
    connect_elem_t *parent_machines;

    // machines our output is connected to 
    machines_stack_t *child_machines;

} moore_t;

// get ceil for division of two numbers
static size_t calculate_block_count(size_t f) {
    return (f + BLOCK_SIZE - 1) / BLOCK_SIZE;
}

// check if connect_elem_t are the same 
static bool same_connect_elems(connect_elem_t a, connect_elem_t b) {
    return a.connected_bit == b.connected_bit && a.connected_machine == b.connected_machine;
}

/** MACHINES_STACK HANDLING **/

// delete the node from stack
// uses : same_connect_elem for comparing two connect_elem_t structs
static void delete_node(connect_elem_t node_to_delete, machines_stack_t *stack) {
    // if stack is empty we don't do anything
    node_t *current_node = stack->top, *used_node = NULL;
    if(current_node == NULL) {
        return;
    }  

    // if we delete top of the we just set the top to prev (might be null)
    if(same_connect_elems(current_node->element, node_to_delete)) {
        stack->top = current_node->prev;
        free(current_node);
        return;
    }

    
    // we iterate over whole list looking for that node 
    while(current_node != NULL && 
        !same_connect_elems(current_node->element, node_to_delete)) {
        node_t *ptr = current_node;
        current_node = current_node->prev;
        used_node = ptr;
    }

    // if we don't find the node we simply return
    if(current_node == NULL) return;

    // unlink the pointer
    used_node->prev = current_node->prev;
    free(current_node);
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
    node_t *prev_node = stack->top;
    stack->top = node;
    node->prev = prev_node;
}


/** INITIALIZATION **/

// function to initialize parents array 
// initially we set each bit to NULL - because we don't have any parents
static connect_elem_t *init_parent_machines_array(size_t n) {
    // return empty we don't want to change calloc initialize for zero
    if(n == 0) return EMPTY_ARRAY;

    connect_elem_t *parent_array = (connect_elem_t *)malloc(n * sizeof(connect_elem_t));
    if(parent_array == NULL) return EMPTY_ARRAY;

    for(size_t i = 0; i < n; i++) {
        parent_array[i] = EMPTY_CONNECT;
    }
    
    return parent_array;
}

// function intializes array of arrays of machines, sets every machine to be empty 
static machines_stack_t* init_child_machines(size_t m) {
    machines_stack_t *children_array = (machines_stack_t *)calloc(m, sizeof(machines_stack_t));
    if(children_array == NULL) return EMPTY_ARRAY;

    for(size_t i = 0; i < m; i++) {
        children_array[i] = EMPTY_STACK;
    }

    return children_array;
}

static connect_elem_t init_connect_elem(size_t bit, moore_t *ma) {
    return (connect_elem_t){.connected_bit = bit, .connected_machine = ma};
}

static node_t init_node(connect_elem_t elem) {
    return (node_t){.element = elem, .prev = NULL};
}

// function to initialize a new state struct
static state_t init_state(size_t n) {
    size_t block_count = calculate_block_count(n);
    return (state_t){.array = (uint64_t*)calloc(block_count, sizeof(uint64_t)), .block_count = block_count, .bits_count = n};
}

// Calc the new output and then assign it to the array machine points to 
static void update_output(moore_t *ma) {
    ma->output_function(ma->output_state.array, ma->current_state.array, ma->output_state.bits_count, ma->current_state.bits_count);
}

// get new state, declare prevState to avoid overwriting
static int update_state(moore_t *ma, uint64_t *inputarray) {
    uint64_t *new_state = calloc(ma->input_state.block_count, sizeof(uint64_t));
    if (!new_state) 
        MEMORY_FAILED(-1);
        
    ma->transition_function(new_state, inputarray, ma->current_state.array,
        ma->input_state.bits_count, ma->current_state.bits_count);
        
    memcpy(ma->current_state.array, new_state,ma->current_state.block_count * sizeof(uint64_t));
        
    free(new_state);
    return 0;
}

moore_t *ma_create_full(size_t n, size_t m, size_t s, transition_function_t t, output_function_t y, uint64_t const *q){
    // check if the parameters are correct
    if(!s || !m || !t || !y || !q) 
        PARAMETER_FAILED(NULL);

    // create a pointer to our new machine
    moore_t *new_machine = (moore_t*)malloc(sizeof(moore_t));
    
    if(new_machine == NULL) 
        MEMORY_FAILED(NULL);

    // define a new machine
    *new_machine = (moore_t){
        .input_state = init_state(n),                      // set state of input for n 
        .current_state = init_state(s),                        // set state of current state for s
        .output_state = init_state(m),                     // set state of output for m
        .transition_function = t,                          // set both functions
        .output_function = y,
        .parent_machines = init_parent_machines_array(n),  // structure to know 'parent' of every bit in inputState
        .child_machines = init_child_machines(m)           // structure to track 'child' for every bit in outputState

    };

    // check for correct memory allocation and update_output
    // if not correct - free all of the adresses 
    if((!new_machine->input_state.array && n != 0) || 
    !new_machine->current_state.array || !new_machine->output_state.array ||
    (!new_machine->parent_machines && n != 0) || !new_machine->child_machines) {
        free(new_machine->input_state.array);    
        free(new_machine->current_state.array);
        free(new_machine->output_state.array);
        free(new_machine->parent_machines);
        free(new_machine->child_machines);
        free(new_machine);
        errno = ENOMEM;
        return NULL;
    }

    //UPD states

    // if n == 0 we set the pointer for input to null to avoid calloc malfunction
    if(n == 0) new_machine->input_state.array = NULL;
    // set cur state of machine
    memcpy(new_machine->current_state.array, q, sizeof(uint64_t) * new_machine->current_state.block_count);
    // if everything went well generate output
    update_output(new_machine);

    return new_machine;
}

// declare a identity function for ma_create_simple
static void identity_function(uint64_t *output, uint64_t const *state, size_t m , size_t some) {
    // mark as used
    (void)some;
    // calculate the number of 64-bit blocks
    size_t block = calculate_block_count(m);
    // copy the correct number of bytes
    memcpy(output, state, block * sizeof(uint64_t));
}

// create a simpler version of machine 
// uses : ma_create_full() 
moore_t *ma_create_simple(size_t n, size_t m, transition_function_t t) {
    // check if m == 0 - we don't want to calloc empty array
    if(!m) 
        PARAMETER_FAILED(NULL);

    uint64_t *empty_array = calloc(calculate_block_count(m), sizeof(uint64_t));
    if (!empty_array) 
        MEMORY_FAILED(NULL);
    
    // cast emptyarray to const when calling ma_create_full.
    moore_t *ma = ma_create_full(n, m, m, t, identity_function, (const uint64_t *)empty_array);
    
    free(empty_array);
    return ma;
}

/* CONNECTING/DISCONNECTING */

// we connect two bits and machines together
static int connect_machines_bits(size_t parent_bit, moore_t *parent, size_t child_bit, moore_t *child){
    connect_elem_t parent_element = init_connect_elem(parent_bit, parent);
    connect_elem_t child_element = init_connect_elem(child_bit, child);   

    // get a pointer to a new node and initialize 
    node_t *new_node = (node_t *)malloc(sizeof(node_t));
    if(new_node == NULL) return -1;                  // return -1 if allocation failed
    *new_node = init_node(child_element);

    // update parent array and add node to list
    child->parent_machines[child_bit] = parent_element;
    push_node(new_node, &(parent->child_machines[parent_bit]));

    return 0;
}

// function unlinks the connection between two bits
// sets the pointer to parent on child machine to null
// and frees the memory allocated on queue (using delete_node())
static void disconnect_machines_bits(size_t parent_bit, moore_t *parent, size_t child_bit, moore_t *child){
    connect_elem_t chilelem = init_connect_elem(child_bit, child);

    child->parent_machines[child_bit] = EMPTY_CONNECT;
    delete_node(chilelem, &(parent->child_machines[parent_bit]));
}

// disconnects 'num' input bits starting from index 'in' in the given machine 'a_in'.
// if any of the bits are connected to another machine, it properly unlinks them.
// returns 0 on success or -1 if parameters are invalid.
int ma_disconnect(moore_t *a_in, size_t in, size_t num)
{
    // check if input range is out of bounds or if parameters are invalid
    bool bad_in = (in + num - 1) >= a_in->input_state.bits_count;
    if(bad_in || !a_in || !num)
        PARAMETER_FAILED(-1);
    
    // now our function disconnects 'num' input bits starting at 'in' from their connected machines
    for(size_t i = 0; i < num; i++) {
        size_t child_bit = in + i;

        // get the machine and bit this input is connected to
        moore_t *prev_out = a_in->parent_machines[child_bit].connected_machine;
        size_t prev_bit = a_in->parent_machines[child_bit].connected_bit;

        // if a connection exists, disconnect the input bit from the output bit of the other machine
        if(prev_out != NULL)
            disconnect_machines_bits(prev_bit, prev_out, child_bit, a_in);
    }

    // return success after all disconnections are complete
    return 0;
}

// we will connect two ranges of bits together
int ma_connect(moore_t *a_in, size_t in, moore_t *a_out, size_t out, size_t num) {
   
    if(!a_out || !a_in || !num)
        PARAMETER_FAILED(-1);
    
    // we define when range is bad
    bool bad_in = (in + num - 1) >= a_in->input_state.bits_count;
    bool bad_out = (out + num - 1) >= a_out->output_state.bits_count;
    if(bad_in || bad_out) 
        PARAMETER_FAILED(-1);
    
    //disconnect all bits on this range 
    ma_disconnect(a_in, in, num);

    // for every bit add a new connection in the specified range
    for(size_t i = 0; i < num; i++) {          
        size_t child = in + i, par = out + i;
        // add a new connection here 
        if(connect_machines_bits(par, a_out, child, a_in) < 0) 
            MEMORY_FAILED(-1);
    }

    return 0;
}

/** DELETING **/

// first we need to delete all of our children - all elements in stack (uses disconnect_machines_bits())
// then we can free the structure without any complications
void ma_delete(moore_t *a) {
    if(a == NULL) return;

    if(a->input_state.bits_count > 0) ma_disconnect(a, 0, a->input_state.bits_count);  // disconnect all parents from child
    
    // disconnect all children 
    for(size_t cur_bit = 0; cur_bit < a->output_state.bits_count; cur_bit++) {
        machines_stack_t *s = &(a->child_machines[cur_bit]);
        node_t *top_node = s->top;
        // delete all children from list one by one 
        while(top_node != NULL) {
            node_t *prev = top_node->prev;  // save it before it might be freed
            size_t child_bit = top_node->element.connected_bit;
            moore_t *child = top_node->element.connected_machine;
            // we forget this connection and free topnode 
            disconnect_machines_bits(cur_bit, a, child_bit, child);
            top_node = prev;  // safe access
        }
    }
    
    // once we disconnected everything we can free the pointers
    free(a->input_state.array);
    free(a->current_state.array);
    free(a->output_state.array);
    free(a->child_machines);
    free(a->parent_machines);
    free(a);
}

/** SETTING **/

// get out new input 
int ma_set_input(moore_t *a, uint64_t const *input){
    if(!a || !a->input_state.bits_count || !input) PARAMETER_FAILED(-1);

    memcpy(a->input_state.array, input, sizeof(uint64_t) * a->input_state.block_count);

    return 0;
}

// get new state and update output
int ma_set_state(moore_t *a, uint64_t const *state){
    if(!a || !state) PARAMETER_FAILED(-1);

    memcpy(a->current_state.array, state, sizeof(uint64_t) * a->current_state.block_count);
    update_output(a);      // upd output

    return 0;
}

// return pointer to outputState array
uint64_t const * ma_get_output(moore_t const *a){
    if(!a) PARAMETER_FAILED(NULL);

    return a->output_state.array;
}

// get the n-th bit from the array of uint64
uint64_t get_bit(state_t st, size_t n) {
    uint64_t block_index = n / BLOCK_SIZE, bit_index = n % BLOCK_SIZE;
    return (st.array[block_index] >> bit_index) & 1;
}

// first we get our new state by checking to which machine each bit is connected to
// when every state is updated we update the out
int ma_step(moore_t *at[], size_t num)
{
    if(!at || !num) PARAMETER_FAILED(-1);

    for(size_t i = 0; i < num; i++) {
        if(!at[i]) 
            PARAMETER_FAILED(-1);
    }

    for(size_t i = 0; i < num; i++) {
        moore_t *cur = at[i];

        uint64_t *input_array = calloc(cur->input_state.block_count, sizeof(uint64_t));
        if(input_array == NULL) 
            MEMORY_FAILED(-1);

        for(size_t bit = 0; bit < cur->input_state.bits_count; bit++) {
            // get the machine we are connected to 
            moore_t *machine = cur->parent_machines[bit].connected_machine;
            uint64_t mybit;
            // get the value of bit (0 or 1)
            if(machine == NULL) mybit = get_bit(cur->input_state, bit);
            else mybit = get_bit(machine->output_state, cur->parent_machines[bit].connected_bit);
            // asign 
            input_array[bit / BLOCK_SIZE] |= (mybit << (bit % BLOCK_SIZE));
        }
        if(update_state(cur, input_array) < 0) 
            MEMORY_FAILED(-1);
        free(input_array);
    }

    for(size_t i = 0; i < num; i++) {
        moore_t *cur = at[i];
        update_output(cur);
    }
    return 0;
}
