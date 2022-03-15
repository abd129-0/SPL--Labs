#include "util.h"

#define SYS_WRITE 4
#define SYS_READ 3
#define SYS_CLOSE 6
#define SYS_OPEN 5
#define SYS_EXIT 1
#define SYS_LSEEK 19
#define STDIN 0
#define STDOUT 1
#define O_RDONLY 0
#define O_WRONLY 1
#define O_CREAT 64
extern int system_call();


int main (int argc , char* argv[], char* envp[]) {
    int input = STDIN, output = STDOUT;
    int i;
    char c = '\n';
    for (i = 1; i < argc; i++){
        if(strncmp(argv[i], "-i", 2) == 0)
            if((input = system_call(SYS_OPEN, argv[i]+2, O_RDONLY, 0777)) <= 0){
                system_call(SYS_WRITE, STDOUT, "FILE NOT FOUND! Using standard input!\n", strlen("FILE NOT FOUND! Using standard input!\n"));
                input = STDIN;
            }
        if(strncmp(argv[i], "-o", 2) == 0)
            output = system_call(SYS_OPEN, argv[i]+2, O_CREAT | O_WRONLY, 0777);
    }
    while (system_call(SYS_READ, input, &c, 1) > 0){
        if((c >= 'A') & (c<='Z'))
            c = c + 32;
        system_call(SYS_WRITE, output, &c, 1);
    }
    if(input != STDIN)
        system_call(SYS_CLOSE, input);
    if(output != STDOUT)
        system_call(SYS_CLOSE, output);
    return 0;
}