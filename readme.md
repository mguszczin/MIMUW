# Moore Machine Library

This project implements a **dynamically loaded C library** for simulating **Moore machines**.

---

## What is a Moore Machine?

A **Moore machine** is a type of deterministic finite automaton often used in synchronous digital circuits.  
It can be represented as a 6-tuple:

$⟨ X, Y, Q, t, y, q_0 ⟩$

- $X$ – set of input signals  
- $Y$ – set of output signals  
- $Q$ – set of internal states  
- $t : X × Q → Q$ – transition function (computes next state from current state and input)  
- $y : Q → Y$ – output function (computes outputs based only on the current state)  
- $q_0 \in Q$ – initial state  

---

## How does a Moore Machine work?

- At each **step**, the transition function `t` updates the machine's **state** based on:
  - current state, and
  - input signals.  
- The output function `y` then computes the **output signals** from the new state.  
- In this library we consider **binary Moore machines**:
  - Inputs: `n` one-bit signals → $X = \\{0,1\\}^{n}$  
  - Outputs: `m` one-bit signals → $Y = \\{0,1\\}^m$  
  - State: `s` bits → $Q = \\{0,1\\}^s$

Bit sequences are stored in arrays of `uint64_t`, with 64 bits per element (LSB-first).  
Unused higher bits in the last word are ignored.

---
## How to compile
To build the library, simply run:
```c
make
```
This will generate dynamically loaded file `libma.so`. 

---

## Interface of the Moore Library

You can see the interface of the library in [`ma.h`](./ma.h).  
For a more detailed breakdown, check out [`interface details`](./documentation/interface_details.md).




