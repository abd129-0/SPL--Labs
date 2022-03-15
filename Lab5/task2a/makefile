all: task2

task2: task2.o LineParser.o
	gcc -m32 -g -Wall -o myshell task2.o LineParser.o
	rm *.o

task2.o: task2.c
	gcc -m32 -g -Wall -c -o task2.o task2.c

LineParser.o: LineParser.c
	gcc -m32 -g -Wall -c -o LineParser.o LineParser.c

.PHONY: clean
clean:
	rm -f *.o main