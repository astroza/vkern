#ifndef RT_H
#define RT_H

#define SYSCALL __asm__ ("int $0x80")
void print(char *message);

#endif
