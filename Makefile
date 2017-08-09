all:
	gcc -m32 vkern.c -o vkern
	gcc -m32 -c rt.c -o rt.o
	gcc -m32 -c hello.c -o hello.o

	ld -melf_i386 hello.o rt.o -o hello
clean:
	rm -f vkern hello *.o
