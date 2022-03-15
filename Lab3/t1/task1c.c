#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*-----------------------------------Structs-----------------------------------*/

typedef struct virus {
    unsigned short SigSize;
    char virusName[16];
    unsigned char* sig;
} virus;

typedef struct link link;
 
struct link {
    link *nextVirus;
    virus *vir;
};

struct fun_desc {
  char *name;
  void (*fun)();
};
/*------------------------------Global Variables------------------------------*/
link* viruses = NULL;
char* infectedName;
/*-----------------------------------Task1a-----------------------------------*/

virus* readVirus(FILE* input){
    virus* vir = (virus*) malloc(sizeof(virus));
    char size[2];
    if(fread(size, 2, 1, input) <= 0){
        free(vir);
        return NULL;
    }
    vir->SigSize = size[0] + size[1]*16*16;
    if(fread(vir->virusName, 16, 1, input) <= 0){
        free(vir);
        return NULL;
    }
    vir->sig = (unsigned char*) malloc(sizeof(unsigned char*) * (vir->SigSize));
    if(fread(vir->sig, vir->SigSize, 1, input) <= 0){
        free(vir->sig);
        free(vir);
        return NULL;
    }
    return vir;
}

void printVirus(virus* virus, FILE* output){
    fprintf(output, "Virus name: %s\nVirus size: %d\nsignature:\n", virus->virusName, virus->SigSize);
    for (int i = 0; i < virus->SigSize; i++){
        fprintf(output, "%02x ", virus->sig[i]);
    }
    fprintf(output, "\n\n");
}


/*-----------------------------------Task1b-----------------------------------*/
void list_print(link *virus_list, FILE* output){
    while(virus_list != NULL){
        printVirus(virus_list->vir, output);
        virus_list = virus_list->nextVirus;
    }
}
 
link* list_append(link* virus_list, virus* data){
    link* lastLink = (link*) malloc(sizeof(link));
    link* firstLink = virus_list;
    lastLink->vir = data;
    lastLink->nextVirus = NULL;
    if(virus_list == NULL) return lastLink;
    while (virus_list->nextVirus != NULL)
        virus_list = virus_list->nextVirus;
    virus_list->nextVirus = lastLink;
    return firstLink;
}

void list_free(link *virus_list){
    link* firstLink = virus_list;
    while(firstLink != NULL){
        virus_list = firstLink;
        firstLink = firstLink->nextVirus;
        free(virus_list->vir->sig);
        free(virus_list->vir);
        free(virus_list);
    }
}
/*----------------------------------Task1c----------------------------------*/

void detect_virus(char *buffer, unsigned int size, link *virus_list, FILE* output){
    while(virus_list != NULL){
        for (int i = 0; i < size-virus_list->vir->SigSize; i++) {
            if(memcmp(buffer + i, virus_list->vir->sig, virus_list->vir->SigSize) == 0)
                fprintf(stderr, "Virus detected!\nStarting byte: %d\nVirus name: %s\nSignature size: %d\n", i, virus_list->vir->virusName, virus_list->vir->SigSize);
        }
        virus_list = virus_list->nextVirus;
    }
}


/*-------------------------------Menu Functions-------------------------------*/
void Load_signatures(){
    char fileInput[100];
    char fileName[100];
    char endianLetters[4];
    FILE* input;
    virus* vir;
    fprintf(stderr, "Enter signatures file name:\n");
    fgets(fileInput, 100, stdin);
    sscanf(fileInput, "%s", fileName);
    if ((input = fopen(fileName, "r")) == NULL){
        fprintf(stderr, "File not found!\n");
        return;
    }
    fread(endianLetters, 4, 1, input);
    while((vir = readVirus(input)) != NULL)
        viruses = list_append(viruses, vir);
    fclose(input);
}

void Print_signatures(){
    list_print(viruses, stdout);
}

void Detect(){
    FILE* suspected;
    char buffer[10000];
    int size;
    if ((suspected = fopen(infectedName, "r")) == NULL){
        fprintf(stderr, "File not found\n");
        return;
    }
    fseek(suspected, 0, SEEK_END);
    size = ftell(suspected);
    fseek(suspected, 0, SEEK_SET);
    fread(buffer, size, 1, suspected);
    detect_virus(buffer, size, viruses, stdout);
    fclose(suspected);
}
struct fun_desc menu[] = { { "Load signatures", Load_signatures }, { "Print signatures", Print_signatures }, { "Detect viruses", Detect }, { NULL, NULL } };


int main(int argc, char** argv){
    int menuSize = sizeof(menu)/sizeof(menu[0])-1;
    char option[10];
    int function;
    if(argc > 1) infectedName = argv[1];
    while(1){
        for(int i = 0; i < menuSize; i++) fprintf(stderr, "%d) %s\n", i+1, menu[i].name);
        fprintf(stderr, "Option: ");
        fgets(option, 10, stdin);
        sscanf(option, "%d", &function);
        if((function < 0) | (function > menuSize)){
            list_free(viruses);
            exit(0);
        }
        menu[function-1].fun();
    }
}