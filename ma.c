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

/* Struct representing current state of bit array.
 * Array is split into 64-bit blocks.
 */
typedef struct state {
    // Bit array split into 64-bit blocks
    uint64_t *array;

    // Number of blocks in the array 
    size_t block_count;

    // Actual number of bits represented (can be less than block_count * 64)
    size_t bits_count;
} state_t;

/* Struct representing a connection to a bit on another machine.
 * Used for defining machine input/output relationships.
 */
typedef struct connect_elem {
    // Pointer to connected machine
    moore_t *connected_machine;

    // Index of the bit it's connected to
    size_t connected_bit;
} connect_elem_t;

/* Node in a singly linked stack.
 * Each node stores a connection element.
 */
typedef struct node {
    // Data stored in this node
    connect_elem_t element;

    // Pointer to previous node
    struct node *prev;  
} node_t;

/* Stack of machine connections (e.g. child machines listening to outputs) */
typedef struct machines_stack {
    // Pointer to the top of the stack
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

/* Returns the ceiling of division (ceil(f / BLOCK_SIZE)) */
static size_t calculate_block_count(size_t f) {
    return (f + BLOCK_SIZE - 1) / BLOCK_SIZE;
}


/* Checks whether two connect_elem_t structs refer to the same machine and bit */
static bool same_connect_elems(connect_elem_t a, connect_elem_t b) {
    return a.connected_bit == b.connected_bit && a.connected_machine == b.connected_machine;
}

/* Deletes a node from the stack if it matches the given connect_elem_t.
 *
 * - Uses same_connect_elems() for comparison.
 * - If the element is at the top of the stack, updates the top pointer.
 * - Frees the memory of the removed node.
 * - Does nothing if the element is not found.
 */
static void delete_node(connect_elem_t node_to_delete, machines_stack_t *stack) {
    // if stack is empty we don't do anything
    node_t *current_node = stack->top, *used_node = NULL;
    if (!current_node)
        return;

    // if we delete top of the we just set the top to prev (might be null)
    if (same_connect_elems(current_node->element, node_to_delete)) {
        stack->top = current_node->prev;
        free(current_node);
        return;
    }

    // we iterate over whole list looking for that node 
    while (current_node != NULL &&
           !same_connect_elems(current_node->element, node_to_delete)) {
        node_t *ptr = current_node;
        current_node = current_node->prev;
        used_node = ptr;
    }

    // if we don't find the node we simply return
    if (!current_node)
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

/* Computes the machine's output by calling its output function.
 * The result is stored in the machine's output_state array.
 */
static void update_output(moore_t *ma) {
    // calculate the output based on the current state and assign it
    ma->output_function(ma->output_state.array, ma->current_state.array, 
        ma->output_state.bits_count, ma->current_state.bits_count);
}

/* Updates the machine's internal state.
 * Calls the machine's transition function and copies the new state into current_state.
 */
static void update_state(moore_t *machine, uint64_t *new_state) {
    // Get the new state without overwriting the old one directly
    machine->transition_function(new_state, machine->input_state.array, machine->current_state.array,
        machine->input_state.bits_count, machine->current_state.bits_count);

    // Copy the new state into the machine's current_state
    memcpy(machine->current_state.array, new_state,
        machine->current_state.block_count * sizeof(uint64_t));
}
/** INITIALIZATION **/

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

/** Returns pointer to moore machine **/
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

/** Returns pointer to 'simple' moore machine
 * uses : ma_create_full()
 **/
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

/* 
 * Connects a specific output bit of a parent machine to a specific input bit of a child machine.
 * 
 * The function:
 * - Updates the parent_machines array in the child
 * - Adds the child_node to the corresponding stack in the parent
 * 
 */
static int connect_machines_bits(size_t parent_bit, moore_t *parent, size_t child_bit, moore_t *child, node_t *child_node) {
    connect_elem_t parent_element = init_connect_elem(parent_bit, parent);
    connect_elem_t child_element = init_connect_elem(child_bit, child);   

    // Initialize the node representing the connection from parent to child
    *child_node = init_node(child_element);

    // Set the child's reference to the parent machine
    child->parent_machines[child_bit] = parent_element;

    // Add the child node to the parent's stack for that output bit
    push_node(child_node, &(parent->child_machines[parent_bit]));

    return 0;
}

/* 
 * Disconnects a previously established connection between a parent machine and a child machine at specific bit indices.
 * 
 * The function:
 * - Clears the reference in the child's parent_machines array
 * - Removes the corresponding node from the parent's child_machines stack
 */
static void disconnect_machines_bits(size_t parent_bit, moore_t *parent, size_t child_bit, moore_t *child) {
    // Create a connection element representing the child
    connect_elem_t child_element = init_connect_elem(child_bit, child);

    // Unlink the child from its parent
    child->parent_machines[child_bit] = EMPTY_CONNECT;

    // Remove the corresponding node from the parent's child stack
    delete_node(child_element, &(parent->child_machines[parent_bit]));
}

/* 
 * Disconnects a range of input bits in a Moore machine.
 * 
 * For each input bit in the specified range, the function:
 * - Checks if the bit is connected to a parent machine
 * - If connected, it properly unlinks the input from the parent's output bit
 */
int ma_disconnect(moore_t *a_in, size_t in, size_t num)
{
    // check if input range is out of bounds or if parameters are invalid
    if (!a_in || !num || 
        in > UINT64_MAX - (num - 1) || (in + num - 1) >= a_in->input_state.bits_count)
        PARAMETER_FAILED(-1);

    // iterate through the input bits to be disconnected
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

/* 
 * Allocates an array of node_t pointers, each of which will point to a newly 
 * allocated `node_t` object. The array will hold connections between bits 
 * of two Moore machines.
 */
static node_t **allocate_node_array(size_t num) {
    // allocate memory for an array of node_t* pointers
    node_t **nodes = malloc(num * sizeof(node_t *));
    if (!nodes)
        return NULL;

    // iterate through and allocate each node_t pointer
    for (size_t i = 0; i < num; i++) {
        nodes[i] = malloc(sizeof(node_t));
        if (!nodes[i]) {
            // free previously allocated nodes on failure
            for (size_t j = 0; j < i; j++)
                free(nodes[j]);
            free(nodes);
            return NULL;
        }
    }

    return nodes;
}

/*
 * Connects a range of bits from the output of one Moore machine (`a_out`)
 * to the input of another Moore machine (`a_in`).
 * 
 * For each bit in the range, this function:
 * - Allocates memory for a `node_t` connection
 * - Disconnects any previous connection at the child's input bit
 * - Connects the parent's output bit to the child's input bit
 * - Stores the connection in the parent's child list and in the child's parent map
 */
int ma_connect(moore_t *a_in, size_t in, moore_t *a_out, size_t out, size_t num) {
    // check for invalid pointers or zero-length range
    if (!a_out || !a_in || !num)
        PARAMETER_FAILED(-1);

    // prevent unsigned overflow in range calculations
    if (in > UINT64_MAX - (num - 1) || out > UINT64_MAX - (num - 1))
        PARAMETER_FAILED(-1);

    // check that the requested input and output ranges are within machine bounds
    bool in_bounds = (in + num - 1) < a_in->input_state.bits_count;
    bool out_bounds = (out + num - 1) < a_out->output_state.bits_count;

    if (!in_bounds || !out_bounds)
        PARAMETER_FAILED(-1);

    // allocate memory for temporary node_t* array used to hold connection nodes
    node_t **new_nodes = allocate_node_array(num);
    if (!new_nodes)
        MEMORY_FAILED(-1);

    // disconnect any previous connections at the target input bits
    ma_disconnect(a_in, in, num);

    // perform the new connections one by one
    for (size_t i = 0; i < num; i++) {
        size_t child_bit = in + i;
        size_t parent_bit = out + i;

        // connect parent machine's output bit to child's input bit
        connect_machines_bits(parent_bit, a_out, child_bit, a_in, new_nodes[i]);
    }

    // we don’t free new_nodes[i] because the nodes are stored inside the machine's stack
    free(new_nodes);  // free only the array of pointers, not the nodes themselves

    return 0;
}

/** DELETING **/

/*
 * Deletes the Moore machine, disconnecting it from its parent and child machines.
 * The machine's state arrays, connection structures, and the machine itself are freed.
 *
 * Steps:
 * - Disconnect all the parent connections (input bits).
 * - Disconnect all the child connections (output bits).
 * - Free the memory associated with the machine.
 */
void ma_delete(moore_t *a) {
    // if the machine is null, there's nothing to delete
    if (!a) 
        return;

    // first, disconnect all the parent connections for this machine
    // disconnect all input bits from any parent machines
    if (a->input_state.bits_count > 0) 
        ma_disconnect(a, 0, a->input_state.bits_count);  // disconnect all parents from the child

    // now disconnect all children: process each output bit one by one
    for (size_t cur_bit = 0; cur_bit < a->output_state.bits_count; cur_bit++) {
        // get the stack of child machines connected to the current output bit
        machines_stack_t *s = &(a->child_machines[cur_bit]);
        node_t *top_node = s->top;

        // traverse the stack and disconnect all child machines
        while (top_node != NULL) {
            // save the previous node before freeing the current one
            node_t *prev = top_node->prev;

            // get the child bit and machine from the current top node
            size_t child_bit = top_node->element.connected_bit;
            moore_t *child = top_node->element.connected_machine;

            // disconnect the child machine and free the current node
            disconnect_machines_bits(cur_bit, a, child_bit, child);

            // move to the previous node in the stack (safe access)
            top_node = prev;
        }
    }
    
    // once all connections are deleted, we can safely free the allocated memory
    // free the state arrays
    free(a->input_state.array);
    free(a->current_state.array);
    free(a->output_state.array);

    // free the connection structures
    free(a->child_machines);
    free(a->parent_machines);

    // finally, free the moore machine structure itself
    free(a);
}


/** SETTING **/

/** Get new input **/
int ma_set_input(moore_t *a, uint64_t const *input){
    if(!a || !a->input_state.bits_count || !input) 
        PARAMETER_FAILED(-1);

    memcpy(a->input_state.array, input, sizeof(uint64_t) * a->input_state.block_count);

    return 0;
}

/** Get set new state **/
int ma_set_state(moore_t *a, uint64_t const *state){
    if(!a || !state) 
        PARAMETER_FAILED(-1);

    memcpy(a->current_state.array, state, sizeof(uint64_t) * a->current_state.block_count);
    update_output(a);     

    return 0;
}

/** Return pointer to output **/
uint64_t const * ma_get_output(moore_t const *a){
    if(!a) 
        PARAMETER_FAILED(NULL);

    return a->output_state.array;
}

/** Get the n-th bit from array of uint64 **/
uint64_t get_bit(state_t st, size_t n) {
    uint64_t block_index = n / BLOCK_SIZE, bit_index = n % BLOCK_SIZE;
    return (st.array[block_index] >> bit_index) & 1; 
}

/** Set the n-th bit to value(0 or 1) **/
void set_bit_to(state_t *state, size_t bit, uint64_t value) {
    size_t index = bit / BLOCK_SIZE;  // determine which block the bit belongs to
    size_t offset = bit % BLOCK_SIZE; // determine the position of the bit within the block
    if (value)
        state->array[index] |= (1ULL << offset);  // set the bit to 1 using OR operation
    else
        state->array[index] &= ~(1ULL << offset); // set the bit to 0 using AND operation with negation
}

/*
 * Validates if the provided machines array is valid.
 * Checks if the array is not NULL and that none of the machine pointers are NULL.
 * Returns 0 if valid, or -1 if any of the machines or the array itself is invalid.
 */
static int validate_machines(moore_t *at[], size_t num) {
    if (!at || !num) 
        return -1; // invalid if array or num is zero

    for (size_t i = 0; i < num; i++) 
        if (!at[i]) 
            return -1; // invalid if any machine is NULL

    return 0; // all machines are valid
}

/*
 * Allocates memory for the new states of the machines.
 * For each machine, it allocates memory for the current state array.
 * Returns a pointer to an array of pointers to uint64_t states, or NULL on failure.
 */
static uint64_t **allocate_new_states(moore_t *at[], size_t num) {
    // allocate memory for an array of pointers to uint64_t arrays (new states)
    uint64_t **states = malloc(sizeof(uint64_t *) * num);
    if (!states) return NULL; // return NULL if allocation fails

    // allocate memory for each machine's new state array
    for (size_t i = 0; i < num; i++) {
        states[i] = calloc(at[i]->current_state.block_count, sizeof(uint64_t)); // allocate for each machine's state array
        if (!states[i]) { // on failure, clean up previously allocated arrays
            for (size_t j = 0; j < i; j++)
                free(states[j]);
            free(states);
            return NULL; // return NULL if allocation fails
        }
    }
    return states; // return pointer to allocated states array
}

/*
 * Updates the input states of the given machine.
 * For each input bit, the corresponding parent bit is retrieved and the machine's input state is updated accordingly.
 */
static void update_machine_inputs(moore_t *machine) {
    // iterate over all input bits
    for (size_t bit = 0; bit < machine->input_state.bits_count; bit++) {
        moore_t *parent = machine->parent_machines[bit].connected_machine; // get connected parent machine
        size_t parent_bit = machine->parent_machines[bit].connected_bit; // get the bit index on parent machine

        // retrieve the value of the bit: from parent machine's output or the machine itself if no parent
        uint64_t val = parent
            ? get_bit(parent->output_state, parent_bit) // get bit value from parent if it exists
            : get_bit(machine->input_state, bit); // otherwise, get the value from the current machine's input state

        // set the bit value to the machine's input state
        set_bit_to(&machine->input_state, bit, val);
    }
}

/*
 * Frees the allocated memory for the new states of all machines.
 * Takes care of freeing each machine's state array and then the array of pointers to these states.
 */
static void cleanup_new_states(uint64_t **states, size_t num) {
    for (size_t i = 0; i < num; i++) 
        free(states[i]); // free each machine's state array
    free(states); // free the array of state pointers
}

/*
 * Performs a step in the Moore machine operation.
 * - Updates the inputs for each machine,
 * - Computes new states for all machines,
 * - Updates output for each machine.
 */
int ma_step(moore_t *at[], size_t num) {
    // validate machines array and check if all machines are valid
    if (validate_machines(at, num) < 0)
        PARAMETER_FAILED(-1); // fail if validation fails

    // allocate memory for the new states of the machines
    uint64_t **newstates = allocate_new_states(at, num);
    if (!newstates) 
        MEMORY_FAILED(-1); // fail if memory allocation fails

    // update inputs and states for each machine
    for (size_t i = 0; i < num; i++) {
        update_machine_inputs(at[i]); // update machine's input state
        update_state(at[i], newstates[i]); // compute and update the new state
    }

    // update the output state of each machine based on the new state
    for (size_t i = 0; i < num; i++) 
        update_output(at[i]);

    // cleanup: free memory allocated for the new states
    cleanup_new_states(newstates, num);

    return 0; // return success
}
