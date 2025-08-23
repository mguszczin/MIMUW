#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdint.h>
#include <dlfcn.h>

struct pointer_to_converted_int {
	char *ptr;
	int size;
};

#define EMPTY_POINTER (struct pointer_to_converted_int){.ptr = NULL, size = 0}

typedef typeof(void (int)) *sighandler_t;

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

sighandler_t generate_dynamically_executable(struct pointer_to_converted_int s) 
{
	const size_t len = sizeof(writer_template);

	/* mmap a region for our code */
	uint8_t *p = mmap(NULL, len, PROT_READ|PROT_WRITE,  /* No PROT_EXEC */
		MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	if (p==MAP_FAILED) {
		errno = ENOMEM;
		return NULL;
	}

	/* Copy it in (still not executable) */
	memcpy(p, writer_template, len);

	uint32_t size_val = (uint32_t)s.size;
	memcpy(&p[16], &size_val, sizeof(size_val));

	uint64_t str_val = (uint64_t)s.ptr;
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

struct pointer_to_converted_int generate_string_from_int(int sys_num) 
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
	char *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if(ptr == MAP_FAILED) {
		errno = ENOMEM;
		return EMPTY_POINTER;
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
	struct pointer_to_converted_int r = {
		.ptr = ptr, 
		.size = size
	};

	return r;
}

sighandler_t make_signal_handler(int sys_num) 
{
	struct pointer_to_converted_int s = generate_string_from_int(sys_num);
	if(s.ptr == NULL) 
		return SIG_ERR;
	
	sighandler_t fptr = generate_dynamically_executable(s);
	if(fptr == NULL)
		return SIG_ERR;
	
	return signal(sys_num, fptr);
}


int main() 
{
	make_signal_handler(2);
	make_signal_handler(12);
	for (;;) pause();
}