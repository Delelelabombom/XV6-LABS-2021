#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char*
fmtname(char *path)
{
  //static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;
  return p;
}



void 
find(char *path, char *target)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if((fd = open(path, 0)) < 0){
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
        printf("find: path too long\n");
        close(fd);
        return;
    }

    if (st.type == T_FILE)
    {
        char *temp = fmtname(path);
        // printf("---target:%s\n", target);
        // printf("--ss-temp:%s\n", temp);
        // printf("---target:%d\n", strlen(target));
        // printf("---target:%d\n", strlen(temp));
        if (strcmp(target, temp) == 0)
        {
            //printf("[[[[[[[[[[[[[[[[[[\n");
            printf("%s\n", path);
        }
    }
    else if (st.type == T_DIR)
    {
        strcpy(buf, path);
        p = buf+strlen(buf);
        *p++ = '/';
        while (read(fd, &de, sizeof(de)) == sizeof(de))
        {
            if(de.inum == 0)
                continue;
            if (strcmp(".", de.name) == 0)
            {
                continue;
            }

            if (strcmp("..", de.name) == 0)
            {
                continue;
            }
            
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if(stat(buf, &st) < 0){
                printf("find: cannot stat %s\n", buf);
                continue;
            }
            // printf("//////////////////////////\n");
            // printf("hhhhhhhhhhhhhh%s\n", buf);
            find(buf, target);
        }
    }
    close(fd); 

}

int
main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(2, "please specify the file and the path\n");
        exit(1);
    }
    find(argv[1], argv[2]);
    exit(0);
}

