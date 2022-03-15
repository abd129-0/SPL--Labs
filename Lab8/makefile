all: main

main: main.o
	gcc -m32 -fno-pie -g -Wall -o myElf main.o
	rm *.o

main.o: task1.c
	gcc -m32 -fno-pie -g -Wall -c -o main.o task1.c

.PHONY: clean
clean:
	rm -f *.o main