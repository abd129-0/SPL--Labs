#include <stdlib.h>
#include <stdio.h>
#include <string.h>


char censor(char c){
  if (c == '!')
    return '.';
  else
    return c;
}
/* Gets a char c and returns its encrypted form by adding 3 to its value. 
    If c is not between 0x20 and 0x7E it is returned unchanged
*/
char encrypt(char c){
  if (c >= 32 && c <= 126)
    return c + 3;
  return c;
}

/* Gets a char c and returns its decrypted form by reducing 3 to its value. 
    If c is not between 0x20 and 0x7E it is returned unchanged
*/
char decrypt(char c){
  if (c >= 32 && c <= 126)
    return c - 3;
  return c;
}
/* dprt prints the value of c in a decimal representation followed by a 
  new line, and returns c unchanged.
*/
char dprt(char c){
  printf("%d\n", c);
  return c;
}

/* If c is a number between 0x20 and 0x7E, cprt prints the character of ASCII value c followed 
    by a new line. Otherwise, cprt prints the dot ('.') character. After printing, cprt returns 
    the value of c unchanged.
*/
char cprt(char c){
  if(c >= 32 && c <= 126)
    printf("%c\n", c);
  else
    printf(".\n");
  return c;
}

/* Ignores c, reads and returns a character from stdin using fgetc. */
char my_get(char c){
  return fgetc(stdin);
}

char* map(char *array, int array_length, char (*f)(char)){
  int i;
  char* mapped_array = (char *)(malloc(array_length * sizeof(char)));
  for(i = 0; i < array_length; i++){
    *(mapped_array + i) = f(*(array + i));
  }
  free(array);
  return mapped_array;
}

struct fun_desc {
  char *name;
  char (*fun)(char);
};

struct fun_desc menu[] = {{"Censor", censor}, {"Encrypt", encrypt}, {"Decrypt", decrypt},
                           {"Print string", cprt}, {"Get string", my_get},
                           {NULL, NULL}};

const size_t MENU_SIZE = sizeof(menu)/sizeof(menu[0]);
const size_t ARRAY_SIZE = 5;

int main(int argc, char **argv){
  char* carray = malloc(ARRAY_SIZE); 
  carray[0] = '\0';
  char received[1];
  char c;
  int i, j, funcToUse;
  
  while(1){
    puts("Please choose a function:");
    i = 0;
    for(i = 0 ; i < MENU_SIZE - 1 ; i++)
      printf("%d) %s\n",i,menu[i].name);
    
    printf("Option: ");
    j = 0;
    while((c = fgetc(stdin)) != '\n'){
      received[j] = c;
      j++;
    }
    funcToUse = atoi(received);

    if(funcToUse >= 0 && funcToUse <= MENU_SIZE -2)
      puts("Within bounds");
    else{
      puts("Not within bounds");
      break;
    }

    char * temp = carray; // temp stores the old carray
    carray = map(carray,ARRAY_SIZE,menu[funcToUse].fun);
    free(temp);
    puts("DONE.\n");
  }
  free(carray); 
  return 0;
}