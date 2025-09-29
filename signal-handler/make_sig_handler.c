#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdint.h>
#include <dlfcn.h>

/*
 * Type for a signal handler function pointer.
 * The expression `typeof(void (int)) *` is a GNU extension that makes
 * `sighandler_t` equivalent to `void (*)(int)`. 
 */
typedef typeof(void (int)) *sighandler_t;

/*
 * writer_template:
 *
 * A small x86-64 machine-code function used as a template for a custom,
 * per-signal handler. At runtime we:
 *  - copy this template into an mmap'd region,
 *  - patch three placeholders:
 *      1) the length/count to write (mov edx, <length>)
 *      2) the pointer to the string buffer (movabs rax, <string>)
 *      3) the address of libc's write() function (movabs rax, <write>)
 *  - make the region executable and cast it to a function pointer.
 *
 * The runtime function (after patching) effectively performs:
 *      write(1, <string>, <length>);
 * and then returns.
 *
 * Important notes:
 *  - The template includes an `endbr64` instruction (F3 0F 1E FA).
 *    This is used for Intel CET (Control-flow Enforcement Technology)
 *    to annotate valid indirect-branch targets; harmless on CPUs that
 *    ignore it.
 *  - Offsets used to patch the template in generate_dynamically_executable()
 *    are hard-coded (they must match the byte layout of this array).
  *  - The template relies on absolute 64-bit addresses patched into
 *    `movabs` instructions. This requires the binary to be compiled
 *    with `-no-pie -fno-pie -mcmodel=large` so that the process
 *    layout is compatible with absolute addressing.
 */
uint8_t writer_template[] = {
        0xf3, 0x0f, 0x1e, 0xfa,             // endbr64
        0x55,                               // push rbp
        0x48, 0x89, 0xe5,                   // mov rbp,rsp
        0x48, 0x83, 0xec, 0x10,             // sub rsp,0x10
        0x89, 0x7d, 0xfc,                   // mov [rbp-0x4],edi

        0xba, 0x00, 0x00, 0x00, 0x00,       // mov edx,<length> (patch later)

        0x48, 0xb8,                         // movabs rax,<string>
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,             // (8-byte placeholder for string address)

        0x48, 0x89, 0xc6,                   // mov rsi,rax
        0xbf, 0x01, 0x00, 0x00, 0x00,       // mov edi,1

        0x48, 0xb8,                         // movabs rax,<write>
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,             // (8-byte placeholder for write address)

        0xff, 0xd0,                         // call rax
        0x90,                               // nop
        0xc9,                               // leave
        0xc3                                // ret
};

/*
 * Open libc and resolve the address of the 'write' function with dlsym().
 * We return the numeric address (uint64_t) so that the template can be patched
 * to call that address directly.
 *
 * Notes:
 *  - We open "libc.so.6" — on some systems the libc SONAME or path may differ.
 *    If this fails, the program prints an error and exits.
 */
uint64_t get_write_adress() {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if (!handle) { 
                perror("dlopen"); 
                exit(1); 
        }

        void *sym = dlsym(handle, "write");
        if (!sym) { 
                perror("dlsym"); 
                exit(1); 
        }

        return (uint64_t)sym;
}

/*
 * Create a dynamically generated function that, when called, writes the
 * provided string to stdout and returns.
 * Returns:
 *  - on success: function pointer cast to sighandler_t
 *  - on failure: NULL and errno set
 *
 * Portability notes:
 *  - On some platforms the offsets used here depend on the exact template
 *    layout and must be kept in sync with writer_template[].
 */
sighandler_t generate_dynamically_executable(char* converted_int) 
{
        const size_t len = sizeof(writer_template);

        uint8_t *p = mmap(NULL, len, PROT_READ|PROT_WRITE, 
                MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
        if (p==MAP_FAILED) {
                errno = ENOMEM;
                return NULL;
        }

        /* 3) patch placeholders */
        /* NOTE: these offsets correspond to positions in writer_template:
         *  - mov edx, imm32   starts at offset 16  (0-based)
         *  - movabs rax,<str> imm64 starts at offset 22 (immediate starts at 22+2)
         *  - movabs rax,<write> imm64 starts at offset 40
         *
         * The exact offsets must match the template layout above.
         */
        memcpy(p, writer_template, len);

        uint32_t size_val = (uint32_t)strlen(converted_int);
        memcpy(&p[16], &size_val, sizeof(size_val));

        uint64_t str_val = (uint64_t)converted_int;
        memcpy(&p[22], &str_val, sizeof(str_val));
        
        uint64_t write_val = get_write_adress();
        memcpy(&p[40], &write_val, sizeof(write_val));

        /* Now make it execute-only */
        if (mprotect(p, len, PROT_EXEC) < 0) {
                munmap(p, len);
                errno = EPROTO;
                return NULL;
        }

        return (sighandler_t)p;
}

/*

 * Convert a non-negative integer to a null-terminated string.
 * - We use mmap() to allocate the character buffer to avoid malloc() (and
 *   potential issues inside signal contexts). The returned buffer is null-terminated.
 * - Negative sys_num are converted as if positive (we simply take absolute).
 *
 * Returns:
 *  - pointer to an mmap'd, NUL-terminated string on success
 *  - NULL on failure (errno set to ENOMEM)
 *
 * The caller is responsible for calling munmap() on the returned buffer when
 * it is no longer needed.
 */
char* generate_string_from_int(int sys_num) 
{
        if(sys_num < 0) 
                sys_num = -sys_num;


        int h = sys_num;
        int size = 0;
        while(h > 0) {
                h /= 10;
                size++;
        }
        if(size == 0) 
                size++;
        
        /* allocate size + 1 for null terminated string */
        char *ptr = mmap(NULL, size + 1, PROT_READ | PROT_WRITE,
                MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);                
        if(ptr == MAP_FAILED) {
                errno = ENOMEM;
                return NULL;
        }

        h = sys_num;
        size = 0;
        while(h > 0) {
                ptr[size] = (char)('0' + (h % 10));
                h /= 10;
                size++;
        }
        if(sys_num == 0) {
                size = 1;
                ptr[0] = '0';
        }

        for(int i = 0; i < size/2; i++) {
                int h = ptr[i];
                ptr[i] = ptr[size - 1 - i];
                ptr[size - 1 - i] = h;
        }

        ptr[size] = '\0';                                    
        return ptr;
}

/*
 * Create and register a signal handler for signal number `sys_num`.
 * Steps:
 *  - convert sys_num -> decimal string
 *  - generate an executable function that will write that string to stdout
 *  - register the generated function as the signal handler with signal()
 *
 * Returns:
 *  - returned value of signal() (previous handler) on success
 *  - SIG_ERR on error
 *
 * Important behavior/caveats:
 *  - We allocate per-signal resources (string via mmap, executable page via mmap).
 *    This code does not free those allocations; if you create handlers repeatedly
 *    you will leak memory. The programmer must take
 *    care of freeing both the string buffer and the executable page
 *    with munmap() when they are no longer needed.
 */
sighandler_t make_signal_handler(int sys_num) 
{
        char* s = generate_string_from_int(sys_num);
        if(s == NULL) 
                return SIG_ERR;
        
        sighandler_t fptr = generate_dynamically_executable(s);
        if(fptr == NULL)
                return SIG_ERR;
        
        return signal(sys_num, fptr);
}


int main() 
{
        /* Generate new responses for signal 2 and signal 12 */
        make_signal_handler(2);
        make_signal_handler(12);
        for (;;) pause();
}