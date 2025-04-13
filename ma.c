#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include "ma.h"


#define BLOCK_SIZE 64
#define EMPTY_ARRAY NULL 
#define EMPTY_CONNECT (connect_elem_t){.connected_machine = NULL, .connected_bit = -1}
#define EMPTY_STACK (machines_stack_t){.top = NULL}

/** ERROR HANDLING MACROS **/

#define PARAMETER_FAILED(x) do { errno = EINVAL; return (x); } while (0)

#define MEMORY_FAILED(x) do { errno = ENOMEM; return (x); } while (0)

/** STRUCTURES DEFINITIONS **/

/** STATE DEFINITION **/

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

/* STACK DEFINITION */

// uses : node
typedef struct machines_stack {
    // pointer to the top of the stack
    node_t *top;

} machines_stack_t;

/* MAIN STRUCT DEFINITION */

/**
 * Represents a Moore machine.
 *
 * Each machine maintains three bit arrays (states):
 * - input_state: Bits gathered from other machines it's connected to (parents)
 * - current_state: The internal state of this machine
 * - output_state: Bits that are passed on to other machines (children)
 *
 * The machine forms a bit-level network with other machines using connections:
 *
 * - Each bit in the input_state can be connected to a specific bit of another machine's output.
 *   These connections are stored in the `parent_machines` array, where:
 *     - `parent_machines[i]` describes the machine and bit index that feed into `input_state[i]`.
 *
 * - Each bit in the output_state can have one or more machines "listening" to it.
 *   These are stored in the `child_machines` stack:
 *     - Each node in the stack (`node_t`) describes a child machine and the bit in its input_state
 *       that this output bit is connected to.
 *     - Multiple children can listen to the same output bit.
 *
 * - `transition_function`: Computes the new internal state based on the input_state
 * - `output_function`: Computes the output_state from the current_state
 */
typedef struct moore {
    // all of the possible states of machine
    state_t input_state, current_state, output_state;

    // function to get new state 
    transition_function_t transition_function;

    // function to get new output state
    output_function_t output_function;

    // machines connected input
    connect_elem_t *parent_machines;

    // machines output is connected to 
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
    if(!current_node)
        return;

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
    if(!current_node) 
        return;

    // unlink the pointer
    used_node->prev = current_node->prev;
    free(current_node);
}

// we add node to the back of the list 
static void push_node(node_t *node, machines_stack_t *stack) {
    // case where stack is empty
    if(!stack->top){
        stack->top = node;
        node->prev = NULL;  // make sure last element in stack points to null 
        return;
    }

    // if stack isn't empty  
    node_t *prev_node = stack->top;
    stack->top = node;
    node->prev = prev_node;
}

/** UPDATING STATES **/

// Calc the new output and then assign it to the array machine points to 
static void update_output(moore_t *ma) {
    ma->output_function(ma->output_state.array, ma->current_state.array, 
        ma->output_state.bits_count, ma->current_state.bits_count);
}

// get new state, declare prev_state to avoid overwriting
static int update_state(moore_t *machine) {
    uint64_t *new_state = calloc(machine->current_state.block_count, sizeof(uint64_t));
    if (!new_state) 
        MEMORY_FAILED(-1);
        
    machine->transition_function(new_state, machine->input_state.array, machine->current_state.array,
        machine->input_state.bits_count, machine->current_state.bits_count);
        
    memcpy(machine->current_state.array, new_state,
        machine->current_state.block_count * sizeof(uint64_t));
        
    free(new_state);
    return 0;
}

/** INITIALIZATION **/

// function to initialize parents array 
// initially we set each bit to NULL - because we don't have any parents
static connect_elem_t *init_parent_machines_array(size_t n) {
    // return empty we don't want to change calloc initialize for zero
    if(!n) return 
        EMPTY_ARRAY;

    connect_elem_t *parent_array = (connect_elem_t *)malloc(n * sizeof(connect_elem_t));
    if(!parent_array) 
        return EMPTY_ARRAY;

    for(size_t i = 0; i < n; i++) {
        parent_array[i] = EMPTY_CONNECT;
    }
    
    return parent_array;
}

// function intializes array of arrays of machines, sets every machine to be empty 
static machines_stack_t* init_child_machines(size_t m) {
    machines_stack_t *children_array = (machines_stack_t *)calloc(m, sizeof(machines_stack_t));
    if(!children_array) 
        return EMPTY_ARRAY;

    for(size_t i = 0; i < m; i++) {
        children_array[i] = EMPTY_STACK;
    }

    return children_array;
}

static connect_elem_t init_connect_elem(size_t bit, moore_t *ma) {
    return (connect_elem_t){
        .connected_bit = bit, 
        .connected_machine = ma
    };
}

static node_t init_node(connect_elem_t elem) {
    return (node_t){
        .element = elem, 
        .prev = NULL
    };
}

static state_t init_state(size_t n) {
    if(!n)                  // handle n == 0 , we don't want to calloc empty array
        return (state_t){0};

    size_t block_count = calculate_block_count(n);
    return (state_t){
        .array = (uint64_t*)calloc(block_count, sizeof(uint64_t)), 
        .block_count = block_count, 
        .bits_count = n
    };
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
        .current_state = init_state(s),                    // set state of current state for s
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
        MEMORY_FAILED(NULL);
    }

    //UPD states

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
    if(!m || !t) 
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

// we connect two bits together
static int connect_machines_bits(size_t parent_bit, moore_t *parent, size_t child_bit, moore_t *child){
    connect_elem_t parent_element = init_connect_elem(parent_bit, parent);
    connect_elem_t child_element = init_connect_elem(child_bit, child);   

    // get a pointer to a new node and initialize 
    node_t *child_node = (node_t *)malloc(sizeof(node_t));
    if(!child_node) 
        return -1;                  // return -1 if allocation failed
    *child_node = init_node(child_element);

    // update parent array and add node to list
    child->parent_machines[child_bit] = parent_element;
    push_node(child_node, &(parent->child_machines[parent_bit]));

    return 0;
}

// function unlinks the connection between two bits
// sets the pointer to parent on child machine to null
// and frees the memory allocated on queue (using delete_node())
static void disconnect_machines_bits(size_t parent_bit, moore_t *parent, size_t child_bit, moore_t *child){
    connect_elem_t child_element = init_connect_elem(child_bit, child);

    child->parent_machines[child_bit] = EMPTY_CONNECT;
    delete_node(child_element, &(parent->child_machines[parent_bit]));
}

// disconnects 'num' input bits starting from index 'in' in the given machine 'a_in'.
// if any of the bits are connected to another machine, it properly unlinks them.
// returns 0 on success or -1 if parameters are invalid.
int ma_disconnect(moore_t *a_in, size_t in, size_t num)
{
    // check if input range is out of bounds or if parameters are invalid
    if (!a_in || !num || 
        in > UINT64_MAX - (num - 1) || (in + num - 1) >= a_in->input_state.bits_count)
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

    return 0;
}

// we will connect two ranges of bits together
int ma_connect(moore_t *a_in, size_t in, moore_t *a_out, size_t out, size_t num) {
   
    if (!a_out || !a_in || !num)
        PARAMETER_FAILED(-1);

    // Prevent overflows: in + num - 1 and out + num - 1
    if (in > UINT64_MAX - (num - 1) || out > UINT64_MAX - (num - 1))
        PARAMETER_FAILED(-1);

    // Check if ranges are within bounds
    bool bad_in = (in + num - 1) >= a_in->input_state.bits_count;
    bool bad_out = (out + num - 1) >= a_out->output_state.bits_count;

    if (bad_in || bad_out)
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
    if(!a) 
        return;

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

// get new input
int ma_set_input(moore_t *a, uint64_t const *input){
    if(!a || !a->input_state.bits_count || !input) 
        PARAMETER_FAILED(-1);

    memcpy(a->input_state.array, input, sizeof(uint64_t) * a->input_state.block_count);

    return 0;
}

// get new state and update output
int ma_set_state(moore_t *a, uint64_t const *state){
    if(!a || !state) 
        PARAMETER_FAILED(-1);

    memcpy(a->current_state.array, state, sizeof(uint64_t) * a->current_state.block_count);
    update_output(a);     

    return 0;
}

// return pointer to outputState array
uint64_t const * ma_get_output(moore_t const *a){
    if(!a) 
        PARAMETER_FAILED(NULL);

    return a->output_state.array;
}

// get the n-th bit from the array of uint64
uint64_t get_bit(state_t st, size_t n) {
    uint64_t block_index = n / BLOCK_SIZE, bit_index = n % BLOCK_SIZE;
    return (st.array[block_index] >> bit_index) & 1;
}

void set_bit_to(state_t *state, size_t bit, uint64_t value) {
    size_t index = bit / BLOCK_SIZE;
    size_t offset = bit % BLOCK_SIZE;
    if (value)
        state->array[index] |= (1ULL << offset);
    else
        state->array[index] &= ~(1ULL << offset);
}

// first we get our new state by checking to which machine each bit is connected to
// when every state is updated we update out
int ma_step(moore_t *at[], size_t num)
{
    if(!at || !num) 
        PARAMETER_FAILED(-1);

    for(size_t i = 0; i < num; i++)
        if(!at[i]) 
            PARAMETER_FAILED(-1);

    for(size_t i = 0; i < num; i++) {
        moore_t *cur = at[i];

        for(size_t bit = 0; bit < cur->input_state.bits_count; bit++) {
            // get the machine we are connected to 
            moore_t *parent_machine = cur->parent_machines[bit].connected_machine;
            uint64_t mybit;
            // get the value of bit (0 or 1)
            if(!parent_machine) mybit = get_bit(cur->input_state, bit);
            else mybit = get_bit(parent_machine->output_state, cur->parent_machines[bit].connected_bit);
            // assign bit 
            set_bit_to(&(cur->input_state), bit, mybit);
        }
        int val = update_state(cur);
        if(val < 0) 
            MEMORY_FAILED(-1);
        
    }

    for(size_t i = 0; i < num; i++) {
        moore_t *cur = at[i];
        update_output(cur);
    }
    return 0;
}
