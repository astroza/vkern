/* (c) felipe@astroza.cl
 */

#include <stdio.h>
#include <sys/ptrace.h>
#include <asm/ptrace.h>

#include <sys/wait.h>
#include <sys/types.h>

#include <signal.h>
#include <unistd.h>

/* print limited to 128 chars
 */
void sys_print(pid_t pid, long addr)
{
	int message[32];
	unsigned int i = 0, zero_offset = 0;
	char *b;
	while(i < sizeof(message)/4) {
		message[i] = ptrace(PTRACE_PEEKDATA, pid, addr + i*4, NULL);
		b = (char *)&message[i];
		while(zero_offset < 4) {
			if(*(b + zero_offset) == 0)
				break;

			zero_offset++;
		}
		if(zero_offset < 4)
			break;
		i++;
		zero_offset = 0;
	}
	printf("\t%d: print syscall: ", pid);
	fflush(stdout);
	write(1, (void *)message, i*4 + zero_offset);
}

int main(int c, char **v)
{
	pid_t pid;
	int status, syscall_nro;
	long addr;
	char data[5];
	data[4] = 0;

	if(c < 2) {
		fprintf(stderr, "Use: %s <program>\n", v[0]);
		return 1;
	}

	if(fork() == 0) { /* Child process */
		/* Request be traced by parent process. From now, every syscall will be captured. */
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		/* Replace process image by our program */
		if(execv(v[1], v + 1) == -1) {
			fprintf(stderr, "Invalid program image\n");
			return 1;
		}

	} else { /* Parent process */
		do {
			pid = wait(&status);
			if(pid == -1 || WIFEXITED(status)) {
				puts("+ Vkern instance finished");
				return 0;
			}
			if(WSTOPSIG(status) == SIGTRAP) {
				syscall_nro = ptrace(PTRACE_PEEKUSER, pid, ORIG_EAX * sizeof(long), NULL);
				printf("+ Syscall number: %d:\n", syscall_nro);
				switch(syscall_nro) {
					case 0:
						ptrace(PTRACE_KILL, pid, NULL, NULL);
						printf("\t%d: exiting process..\n", pid);
						break;
					case 1:
						addr = ptrace(PTRACE_PEEKUSER, pid, EBX * sizeof(long), NULL);
						sys_print(pid, addr);
						break;
					case 11:
						// First execve syscall
						puts("\tIgnored");
						break;
					default:
						puts("\tSyscall not supported");
						break;
				}
				/* Mark syscall was emulated, so it will not managed as regular linux call
				*/
				ptrace(PTRACE_SYSEMU, pid, NULL, NULL);
			}
		} while(1);
	}
	return 0;
}
