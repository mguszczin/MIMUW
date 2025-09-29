# Moore Machine Library Interface

### Key Types
- `typedef struct moore moore_t`
  Type representing a Moore machine. 

- `typedef void (*transition_function_t)(uint64_t *next_state, uint64_t const *input, uint64_t const *state, size_t n, size_t s)`  
  Transition function type. Computes the next state from the current state and input signals. Programmer must provide this function.  
  Parameters:
    - next_state - pointer to bit array where the new state will be stored
    - input - pointer to bit array containing input signals
    - state - pointer to current state bits
    - n - number of input signals
    - s - number of state bits

- `typedef void (*output_function_t)(uint64_t *output, uint64_t const *state, size_t m, size_t s)`
  Output function type. Computes output signals based on the current state. Programmer must provide this function.  
  Parameters:
    - output - pointer to bit array where output signals will be stored
    - state - pointer to current state bits
    - m - number of output signals
    - s - number of state bits

### Creation Functions

- `moore_t * ma_create_full(size_t n, size_t m, size_t s, transition_function_t t, output_function_t y, uint64_t const *q)`
  Creates a full Moore machine with custom transition and output functions.  
  Parameters:
    - n - number of input signals
    - m - number of output signals
    - s - number of state bits
    - t - transition function
    - y - output function
    - q - pointer to initial state bit array

  Returns:
    - pointer to the new machine
    - NULL if parameters are invalid (m == 0, s == 0, pointer is NULL, EINVAL is set) or memory allocation fails (ENOMEM is set)
- `moore_t * ma_create_simple(size_t n, size_t s, transition_function_t t)`  
  Creates a simple Moore machine where outputs equal the state, initial state is zero, and unused state bits are zero.  
  Parameters:
    - n - number of input signals
    - s - number of state bits (also number of outputs)
    - t - transition function

  Returns:
    - pointer to the new machine
    - NULL if parameters are invalid (m == 0, s == 0, pointer is NULL EINVAL is set) or memory allocation fails (ENOMEM is set)

### Deletion
- `void ma_delete(moore_t *a)` 
  Frees all memory associated with a Moore machine. Does nothing if a is NULL.
  Parameters:
    - a - pointer to machine 

### Connection Functions

- `ma_connect(moore_t *a_in, size_t in, moore_t *a_out, size_t out, size_t num)`  
  Connects num input signals of a_in to output signals of a_out, starting at positions in and out. Automatically disconnects existing connections if necessary.
  Parameters 
    - a_in - pointer to machine with input 
    - a_out - pointer to machine with output 
    - in - starting index of input to connect
    - out - starting index of output to connect
    - num - number of bits to connect 

 
  Returns:
    - 0 on success
    - -1 on failure (NULL pointer, invalid range, num == 0, EINVAL is set)  or memory allocation failure (ENOMEM is set)

- `ma_disconnect(moore_t *a_in, size_t in, size_t num)` 
  Disconnects num consecutive inputs of a_in starting from position in. Unconnected inputs remain in an undefined state.
   Parameters:
    - a_in - pointer to machine which input bits we want to disconnect
    - in - position we start to disconnect
    - num - number of bits we want to disconnect

  Returns:
    - 0 on success
    - -1 on failure (NULL pointer, invalid range, num == 0 EINVAL is set)

### Structure Setters

- `int ma_set_input(moore_t *a, uint64_t const *input)`  
  Sets values for unconnected input signals. Connected inputs are ignored.  
  Parameters:
    - a - pointer to machine 
    - input - pointer to new input array

  Returns:
    - 0 on success
    - -1 if a is NULL or has no inputs (errno = EINVAL)

- `ma_set_state(moore_t *a, uint64_t const *state)`  
  Sets the current state of the machine.  
  Parameters:
    - a - pointer to machine 
    - state - pointer to new state of array

  Returns:
     - 0 on success
    - -1 if a is NULL (errno = EINVAL)

### Output Functions

- `uint64_t const * ma_get_output(moore_t const *a)`  
  Returns a pointer to the machine's output signals. The pointer remains valid until ma_delete is called.  
  Parameters: 
    - a -  pointer to machine 

  Returns:
    - pointer to output signals
    - NULL if a is NULL (errno = EINVAL)

### Step Function

- `int ma_step(moore_t *at[], size_t num)`  
  Performs one synchronous computation step on an array of machines. All machines are updated in parallel.  
  Parameters:
    - at - array of pointers to Moore machines
    - num - size of the array

  Returns:
    - 0 on success
    - -1 on failure (NULL pointer, invalid array, num == 0 EINVAL is set) or memory allocation failure (ENOMEM is set)

## Notes: 
- Bit sequences and signal values are stored in arrays of type uint64_t. 
- Each element stores 64 consecutive bits starting from the least significant bit. If the total length is not a multiple of 64, the higher bits of the last element remain unused.
- Inputs can be set via ma_set_input or by connecting to another machine's output
- Undefined input states remain undefined until set or connected
- Disconnection resets the input to an undefined state

# Moore Machine Library Interface

### Key Types
- `typedef struct moore moore_t`
  Type representing a Moore machine. 

- `typedef void (*transition_function_t)(uint64_t *next_state, uint64_t const *input, uint64_t const *state, size_t n, size_t s)`  
  Transition function type. Computes the next state from the current state and input signals. Programmer must provide this function.  
  Parameters:
    - next_state - pointer to bit array where the new state will be stored
    - input - pointer to bit array containing input signals
    - state - pointer to current state bits
    - n - number of input signals
    - s - number of state bits

- `typedef void (*output_function_t)(uint64_t *output, uint64_t const *state, size_t m, size_t s)`
  Output function type. Computes output signals based on the current state. Programmer must provide this function.  
  Parameters:
    - output - pointer to bit array where output signals will be stored
    - state - pointer to current state bits
    - m - number of output signals
    - s - number of state bits

### Creation Functions

- `moore_t * ma_create_full(size_t n, size_t m, size_t s, transition_function_t t, output_function_t y, uint64_t const *q)`
  Creates a full Moore machine with custom transition and output functions.  
  Parameters:
    - n - number of input signals
    - m - number of output signals
    - s - number of state bits
    - t - transition function
    - y - output function
    - q - pointer to initial state bit array

  Returns:
    - pointer to the new machine
    - NULL if parameters are invalid (m == 0, s == 0, pointer is NULL, EINVAL is set) or memory allocation fails (ENOMEM is set)
- `moore_t * ma_create_simple(size_t n, size_t s, transition_function_t t)`  
  Creates a simple Moore machine where outputs equal the state, initial state is zero, and unused state bits are zero.  
  Parameters:
    - n - number of input signals
    - s - number of state bits (also number of outputs)
    - t - transition function

  Returns:
    - pointer to the new machine
    - NULL if parameters are invalid (m == 0, s == 0, pointer is NULL EINVAL is set) or memory allocation fails (ENOMEM is set)

### Deletion
- `void ma_delete(moore_t *a)` 
  Frees all memory associated with a Moore machine. Does nothing if a is NULL.
  Parameters:
    - a - pointer to machine 

### Connection Functions

- `ma_connect(moore_t *a_in, size_t in, moore_t *a_out, size_t out, size_t num)`  
  Connects num input signals of a_in to output signals of a_out, starting at positions in and out. Automatically disconnects existing connections if necessary.
  Parameters 
    - a_in - pointer to machine with input 
    - a_out - pointer to machine with output 
    - in - starting index of input to connect
    - out - starting index of output to connect
    - num - number of bits to connect 

 
  Returns:
    - 0 on success
    - -1 on failure (NULL pointer, invalid range, num == 0, EINVAL is set)  or memory allocation failure (ENOMEM is set)

- `ma_disconnect(moore_t *a_in, size_t in, size_t num)` 
  Disconnects num consecutive inputs of a_in starting from position in. Unconnected inputs remain in an undefined state.
   Parameters:
    - a_in - pointer to machine which input bits we want to disconnect
    - in - position we start to disconnect
    - num - number of bits we want to disconnect

  Returns:
    - 0 on success
    - -1 on failure (NULL pointer, invalid range, num == 0 EINVAL is set)

### Structure Setters

- `int ma_set_input(moore_t *a, uint64_t const *input)`  
  Sets values for unconnected input signals. Connected inputs are ignored.  
  Parameters:
    - a - pointer to machine 
    - input - pointer to new input array

  Returns:
    - 0 on success
    - -1 if a is NULL or has no inputs (errno = EINVAL)

- `ma_set_state(moore_t *a, uint64_t const *state)`  
  Sets the current state of the machine.  
  Parameters:
    - a - pointer to machine 
    - state - pointer to new state of array

  Returns:
     - 0 on success
    - -1 if a is NULL (errno = EINVAL)

### Output Functions

- `uint64_t const * ma_get_output(moore_t const *a)`  
  Returns a pointer to the machine's output signals. The pointer remains valid until ma_delete is called.  
  Parameters: 
    - a -  pointer to machine 

  Returns:
    - pointer to output signals
    - NULL if a is NULL (errno = EINVAL)

### Step Function

- `int ma_step(moore_t *at[], size_t num)`  
  Performs one synchronous computation step on an array of machines. All machines are updated in parallel.  
  Parameters:
    - at - array of pointers to Moore machines
    - num - size of the array

  Returns:
    - 0 on success
    - -1 on failure (NULL pointer, invalid array, num == 0 EINVAL is set) or memory allocation failure (ENOMEM is set)

## Notes: 
- Bit sequences and signal values are stored in arrays of type uint64_t. 
- Each element stores 64 consecutive bits starting from the least significant bit. If the total length is not a multiple of 64, the higher bits of the last element remain unused.
- Inputs can be set via ma_set_input or by connecting to another machine's output
- Undefined input states remain undefined until set or connected
- Disconnection resets the input to an undefined state

