#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define  NAME_LEN  128
#define  BUF_SZ    10000
#define debug if(s->debug_mode) fprintf

typedef struct state{
  int debug_mode;
  char file_name[NAME_LEN];
  int unit_size;
  unsigned char mem_buf[BUF_SZ];
  size_t mem_count;
} state;

struct fun_desc {
  char *name;
  void (*fun)(state*);
};

/*-----------------------Help Functions-----------------------*/
char* unit_to_format(int unit, int hex) {
    static char* hex_format[] = {"%01hhX\n", "%01hX\n", "No such unit", "%01X\n"};
    static char* dec_format[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};
    if(hex) return hex_format[unit-1];
    else return dec_format[unit-1];
}

void print_units(FILE* output, char* buffer, int count, state* s, int hex) {
    char* end = buffer + s->unit_size*count;
    while (buffer < end) {
        //print ints
        int var = *((int*)(buffer));
        fprintf(output, unit_to_format(s->unit_size, hex), var);
        buffer += s->unit_size;
    }
}

void overwrite(FILE* file, state* s, char* buffer, int target, int length){
    int file_size;
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if(target < file_size){
        fseek(file, target, SEEK_SET);
        fwrite(buffer, length, s->unit_size, file);
    } else {
        fprintf(stderr, "Error: target bigger than file size!\n");
    }
}

/*-----------------------Menu Functions-----------------------*/
void toggle_debug_mode(state* s){
    s->debug_mode = !s->debug_mode;
    if(s->debug_mode) fprintf(stderr, "Debug flag now on\n");
    else fprintf(stderr, "Debug flag now off\n");
}
void set_file_name(state* s){
    char input[NAME_LEN];
    fprintf(stderr, "Enter file name:\n");
    fgets(input, NAME_LEN, stdin);
    strncpy(s->file_name, input, strlen(input)-1);
    strncpy(s->file_name + strlen(input) - 1, "\0", 1);
    debug (stderr, "file name set to %s\n", s->file_name);
}
void set_unit_size(state* s){
    char input[10];
    int number;
    fprintf(stderr, "Enter unit size:\n");
    fgets(input, 10, stdin);
    sscanf(input, "%d", &number);
    if((number != 1) & (number != 2) & (number != 4))
        fprintf(stderr, "Can't set the unit size to %s", input);
    else{
        s->unit_size = number;
        debug (stderr, "Debug: set size to %d\n", s->unit_size);
    }
}
void load_into_memory(state* s){
    FILE* file;
    char input[10];
    int location, length;
    if(strcmp(s->file_name, "") == 0)
        fprintf(stderr, "Error: file name is empty!\n");
    else {
        if((file = fopen(s->file_name, "r")) == NULL)
            fprintf(stderr, "Error: couldn't open file!\n");
        else { //file name not empty and opened
            fprintf(stderr, "Please enter <location> <length>\n");
            fgets(input, 10, stdin);
            sscanf(input, "%x %d", &location, &length);
            debug (stderr, "File name: %s\nlocation: %d\nlength: %d", s->file_name, location, length);
            fseek(file, location, SEEK_SET);
            fread(s->mem_buf, length, s->unit_size, file);
            s->mem_count += length ;
            fclose(file);
            fprintf(stderr, "Loaded %d units into memory\n", length);
        }
    }
}
void memory_display(state* s){
    char input[10];
    int address, length;
    fprintf(stderr, "Enter address and length\n");
    fgets(input, 10, stdin);
    sscanf(input, "%x %d", &address, &length);
    if(address == 0){
        address = (int) s->mem_buf;
        fprintf(stderr, "Hexadecimal\n===========\n");
        print_units(stdout, (char*) s->mem_buf, length, s, 1);
        fprintf(stderr, "Decimal\n=======\n");
        print_units(stdout, (char*) s->mem_buf, length, s, 0);
    } else {
        unsigned char* ptr = (unsigned char*) &address; //pointer to memory
        fprintf(stderr, "Hexadecimal\n===========\n");
        print_units(stdout, (char*) ptr, length, s, 1);
        fprintf(stderr, "\nDecimal\n=======\n");
        print_units(stdout, (char*) ptr, length, s, 0);
    }
}
void save_into_file(state* s){
    FILE* file;
    char input[100];
    int address, target, length;
    fprintf(stderr, "Please enter <source-address> <target-location> <length>\n");
    fgets(input, 100, stdin);
    sscanf(input, "%x %x %d", &address, &target, &length);
    if(strcmp(s->file_name, "") == 0)
        fprintf(stderr, "Error: file name is empty!\n");
    else {
        if((file = fopen(s->file_name, "r+")) == NULL)
            fprintf(stderr, "Error: couldn't open file!\n");
        else {
            if(address == 0){
                address = (int) s->mem_buf;
                overwrite(file, s, (char*) s->mem_buf, target, length);
            } else {
                unsigned char* ptr = (unsigned char*) &address; //pointer to memory
                overwrite(file, s, (char*) ptr, target, length);
            }
            fclose(file);
        }
    }
}
void memory_modify(state* s){
    char input[100];
    int location, val;
    fprintf(stderr, "Please enter <location> <val>\n");
    fgets(input, 100, stdin);
    sscanf(input, "%x %x", &location, &val);
    debug (stderr, "location: %d\nVal = %x", location, val);
    memcpy(s->mem_buf+location, &val, s->unit_size);
}

void quit(state* s){
    debug (stderr, "quitting\n");
    free(s);
    exit(0);
}

struct fun_desc menu[] = { { "Toggle Debug Mode", toggle_debug_mode }, { "Set File Name", set_file_name },
 { "Set Unit Size", set_unit_size }, { "Load Into Memory", load_into_memory }, 
 { "Memory Display", memory_display }, { "Save Into File", save_into_file }, 
 { "Memory Modify", memory_modify }, { "Quit", quit }, { NULL, NULL } };

int main(int argc, char** argv){
    state* s = malloc(sizeof(state));
    s->debug_mode = 0;
    char input[100];
    int option;
    while(1){
        debug (stderr, "unit size: %d\nfile name: %s\nmem count: %d\n", s->unit_size, s->file_name, s->mem_count);
        fprintf(stderr, "Choose action:\n");
        for (int i = 0; i < 8; i++)
            fprintf(stderr, "%d-%s\n", i, menu[i].name);
        fgets(input, 100, stdin);
        sscanf(input, "%d", &option);
        if((option < 0) | (option > 7)){
            fprintf(stderr, "Out of bounds\n");
            break;
        }
        menu[option].fun(s);
    }
/*Choose action:
0-Toggle Debug Mode
1-Set File Name
2-Set Unit Size
3-Load Into Memory
4-Memory Display
5-Save Into File
6-Memory Modify
7-Quit*/
    free(s);
}