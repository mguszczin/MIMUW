# Dynamic Signal Handlers with Trampolines

## Signal Handler

A signal handler is a function that the operating system calls automatically when a process receives a signal (for example, SIGINT when pressing Ctrl+C).

This project demonstrates how to generate signal handlers dynamically at runtime. Instead of writing one handler function per signal, we build a small piece of machine code (a trampoline) that directly calls write(1, "signum", length). This allows us to register many different handlers dynamically without writing them by hand.

## What problem does it solve?

If you want your program to print the signal number whenever it receives a signal, you quickly run into a problem: according to `man 7 signal-safety`, there is no async-signal-safe function to convert an integer to a string.

Normally, a programmer would have to write a separate handler for each signal, for example:
```c
void handler2(int s)  { write(1, "2", 1); }
void handler12(int s) { write(1, "12", 2); }
```
**This project solves that problem by generating these handlers dynamically at runtime** using
 ```c
make_signal_handler(num)
```
creating small executable trampolines that already contain the precomputed string and call only the async-signal-safe `write` function.

## How to compile

This project must be compiled with specific flags to ensure the generated trampolines work correctly:
```c
-no-pie -fno-pie -mcmodel=large
```
You can use them in your own compilation or just use the `make` file to compile the program.