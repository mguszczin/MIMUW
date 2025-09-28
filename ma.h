#ifndef MA_H
#define MA_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/** TYPES  **/

/* Transition function type: computes next state from input and current state */
typedef void (*transition_function_t)(uint64_t *next_state, uint64_t const *input,
                                      uint64_t const *state, size_t n, size_t s);
/* Output function type: computes output from current state */
typedef void (*output_function_t)(uint64_t *output, uint64_t const *state,
                                  size_t m, size_t s);
/* Opaque type representing a Moore machine */
typedef struct moore moore_t;

/** CREATION **/

/* Create a full Moore machine with custom transition, output, and initial state */
moore_t * ma_create_full(size_t n, size_t m, size_t s, transition_function_t t,
                         output_function_t y, uint64_t const *q);

/* Create a simple Moore machine: output = state, zero initial state */
moore_t * ma_create_simple(size_t n, size_t s, transition_function_t t);

/** DELETION **/

/* Destroy a Moore machine and free its resources */
void ma_delete(moore_t *a);

/** CONNECTION **/

/* Connect outputs of one machine to inputs of another */
int ma_connect(moore_t *a_in, size_t in, moore_t *a_out, size_t out, size_t num);

/* Disconnect inputs from outputs */
int ma_disconnect(moore_t *a_in, size_t in, size_t num);

/** STRUCTURE SETTERS **/

/* Set values of unconnected inputs */
int ma_set_input(moore_t *a, uint64_t const *input);

/* Set the current state of the machine */
int ma_set_state(moore_t *a, uint64_t const *state);

/** OUTPUT GETTER **/

/* Get current outputs of the machine */
uint64_t const * ma_get_output(moore_t const *a);

/** STEP OPERATION **/

/* Perform one synchronous step on an array of machines */
int ma_step(moore_t *at[], size_t num);

#endif
