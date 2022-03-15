#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <elf.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>

#define  NAME_LEN  128
#define  BUF_SZ    10000
#define debug if(s->debug_mode) fprintf

typedef struct state{
  int debug_mode;
  int currentfd;
  void* map_start;
  struct stat fd_stat; 
} state;

struct fun_desc {
  char *name;
  void (*fun)(state*);
};

/*-----------------------Help Functions-----------------------*/
Elf32_Shdr* get_shdr_idx(Elf32_Ehdr* hdr,int idx){
    return &((Elf32_Shdr*)((int)hdr + hdr->e_shoff))[idx];
}
//Can do this more efficiently
char* get_shdr_name(Elf32_Ehdr* hdr, Elf32_Shdr* secHdr){
    if(hdr->e_shstrndx == SHN_UNDEF)
        return NULL;
    char* strtab = (char*)hdr+get_shdr_idx(hdr,hdr->e_shstrndx)->sh_offset;
    if(!strtab)
        return NULL;
    return strtab + (secHdr->sh_name);
}

/*-----------------------Menu Functions-----------------------*/
void toggle_debug_mode(state* s){
    s->debug_mode = !s->debug_mode;
    if(s->debug_mode) fprintf(stderr, "Debug flag now on\n");
    else fprintf(stderr, "Debug flag now off\n");
}

void Examine_ELF_File(state* s){
    char input[NAME_LEN];
    char name[NAME_LEN];
    fprintf(stderr, "Enter file name:\n");
    fgets(input, 100, stdin);
    sscanf(input, "%s", name);
    
    if(s->currentfd != -1) close(s->currentfd);
    s->currentfd = open(name, O_RDONLY);

    if(s->map_start) munmap(s->map_start,s->fd_stat.st_size);

    if(s->currentfd < 0){
        perror("error in open");
        return;
    }

    if(fstat(s->currentfd, &s->fd_stat) != 0){
        perror("stat failed");
        close(s->currentfd);
        s->currentfd = -1;
        return;
    }

    if((s->map_start = mmap(0, s->fd_stat.st_size, PROT_READ | PROT_WRITE , MAP_PRIVATE, s->currentfd, 0)) == MAP_FAILED){
        perror("mmap failed");
        close(s->currentfd);
        s->currentfd = -1;
        return;
    }

    Elf32_Ehdr* header = (Elf32_Ehdr*) s->map_start;

    if(strncmp((char*)header->e_ident+1,"ELF",3) != 0){
        fprintf(stderr,"Error: File is not ELF file!\n");
        close(s->currentfd);
        s->currentfd = -1;
        munmap(s->map_start,s->fd_stat.st_size);
        s->map_start = NULL;
        return;
    }

    fprintf(stderr, "Magic:\t %x %x %x\n",header->e_ident[1],header->e_ident[2],header->e_ident[3]);
    if(header->e_ident[EI_DATA] == 1)
        fprintf(stderr, "Data:\t\t\t\t\t 2's complement, little endian\n");
    else if(header->e_ident[EI_DATA] == 2)
        fprintf(stderr, "Data:\t\t\t\t\t 2's complement, big endian\n");
    fprintf(stderr, "Entry point:\t\t\t\t %x\n", header->e_entry);
    fprintf(stderr, "Start of section headers:\t\t %d\n", header->e_shoff);
    fprintf(stderr, "Number of section headers:\t\t %d\n", header->e_shnum);
    fprintf(stderr, "Size of section headers:\t\t %d\n", header->e_shentsize);
    fprintf(stderr, "Start of program headers:\t\t %d\n", header->e_phoff);
    fprintf(stderr, "Number of program headers:\t\t %d\n", header->e_phnum);
    fprintf(stderr, "Size of program headers:\t\t %d\n\n", header->e_phentsize);
}

void Print_Section_Names(state* s){
    Elf32_Ehdr* header;
    Elf32_Shdr* sections;
    if((!s->map_start) | (s->currentfd < 0)){
        fprintf(stderr,"Error: file is not defined.\n");
        return;
    }
    header = (Elf32_Ehdr*) s->map_start;
    fprintf(stderr, "Section Headers:\n");
    fprintf(stderr, "[Nr] Name                 Addr      Off     Size    Type");
    debug (stderr, "      Name offset");
    fprintf(stderr, "\n");

    for(int i = 0 ; i < header->e_shnum ; i++){
        sections = get_shdr_idx(header, i);
        fprintf(stderr, "[%2d] %-20s %08x  %06x  %06x  %-8x",i,get_shdr_name(header,sections),sections->sh_addr, sections->sh_offset,sections->sh_size,sections->sh_type);
        debug (stderr, "  %06x",sections->sh_name);
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
}

void Print_Symbols(state* s){
    fprintf(stderr, "not implemented yet\n");
}

void Relocation_Tables(state* s){
    fprintf(stderr, "not implemented yet\n");
}

void quit(state* s){
    debug (stderr, "quitting\n");
    free(s);
    exit(0);
}

struct fun_desc menu[] = { { "Toggle Debug Mode", toggle_debug_mode }, { "Examine ELF File", Examine_ELF_File },
 { "Print Section Names", Print_Section_Names }, { "Print Symbols", Print_Symbols }, 
 { "Relocation Tables", Relocation_Tables }, { "Quit", quit }, { NULL, NULL } };

int main(int argc, char** argv){
    state* s = malloc(sizeof(state));
    s->debug_mode = 0;
    s->currentfd = -1;
    s->map_start = NULL;
    char input[100];
    int option;
    while(1){
        fprintf(stderr, "Choose action:\n");
        for (int i = 0; i < 6; i++)
            fprintf(stderr, "%d-%s\n", i, menu[i].name);
        fgets(input, 100, stdin);
        sscanf(input, "%d", &option);
        if((option < 0) | (option > 7)){
            fprintf(stderr, "Out of bounds\n");
            break;
        }
        menu[option].fun(s);
    }
    free(s);
}