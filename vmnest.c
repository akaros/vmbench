#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <vmm/vmm.h>

volatile uint64_t logout;
struct virtual_machine vm = {.halt_exit = true, .logout = &logout};

static volatile int count, done, lim = 1000;
int mc;

static void vmcall(void *a)
{
	while (count < lim) {
		__asm__ __volatile__("vmcall\n\t");
		count++;
	}
	done = 1;
	while (1);
	__asm__ __volatile__("hlt\n\t");
}


int main(int argc, char **argv)
{
	int iter = 0;
	int d = 0;
	if (argc > 1)
		lim = strtoul(argv[1], 0, 0);
	if (argc > 2) d++;

	if (vthread_attr_init(&vm, 0) < 0)
		err(1, "vthread_attr_init: %r: ");
	vthread_create(&vm, 0, vmcall, NULL);

	while (! done) {
		logout |= 1;
		if (d) printf("lock.");
		while ((! done) && (logout & 1))
			;
		if (d) printf("out\n");
		iter++;
	}
	printf("host iter %d guest iter %d\n", iter, count);
	return 0;
}
