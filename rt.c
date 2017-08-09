#include "rt.h"

void print(char *message)
{
	__asm__ volatile("movl %0, %%ebx\n"
			"movl $1, %%eax" : "=m" (message));
	SYSCALL;
}

/* exit and _exit are already declared by XXX
 */
void __exit()
{
	__asm__ volatile("movl $0, %eax");
	SYSCALL;
}

void main();
void _start()
{
	main();
	__exit();
}
