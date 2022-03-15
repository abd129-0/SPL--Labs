#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int digit_cnt(char* str){
    int n = 0, i = 0;
    while(str[i]){
        if((str[i] >= '0') & (str[i] <= '9'))
            n++;
        i++;
    }
    return n;
}

int main(int argc, char** argv){
    if(argc > 1)
        fprintf(stderr, "The number of digits in the string is: %d\n", digit_cnt(argv[1]));
    return 0;
}