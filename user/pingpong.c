#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    int p2c[2];
    int c2p[2];
    int child_pid;
    int parent_pid;
    char *child_buf = malloc(1);
    char *parent_buf = malloc(1);
    
    pipe(p2c);
    pipe(c2p);
    if(fork() == 0) {
        close(p2c[1]);
        close(c2p[0]);
        
        read(p2c[0], child_buf, 1);
        child_pid = getpid();
        printf("%d: received ping\n", &child_pid);
        write(c2p[1], "0", 1);
    } else {
        close(p2c[0]);
        close(c2p[1]);

        write(p2c[1], "0", 1);
        read(c2p[0], parent_buf, 1);
        parent_pid = getpid();
        printf("%d: received pong\n", &parent_pid);
    }
    exit(0);
}