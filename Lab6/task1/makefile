all: task1

task1: task1.o LineParser.o
	gcc -m32 -g -Wall -o mypipeline task1.o LineParser.o
	rm *.o

task1.o: mypipeline.c
	gcc -m32 -g -Wall -c -o task1.o mypipeline.c

LineParser.o: LineParser.c
	gcc -m32 -g -Wall -c -o LineParser.o LineParser.c

.PHONY: clean
clean:
	rm -f *.o main