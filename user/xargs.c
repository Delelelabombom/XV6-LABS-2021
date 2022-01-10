#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

int
main(int argc, char *argv[])
{
    char buffer[32];
    char newbuffer[32];
    int indexofnewbuffer = 0;
    char *pointofarg = newbuffer;
    char *newargv[32];
    
    

    int index = 0;
    for (; index < argc - 1; index++)
    {
        newargv[index] = argv[index + 1];
    }
    
    
    int lenth;
    while ((lenth = read(0, buffer, sizeof(buffer))) > 0)
    {
        for (int i = 0; i < lenth; i++)
        {
            if (buffer[i] == ' ')
            {
                newbuffer[indexofnewbuffer] = 0;
                indexofnewbuffer++;
                newargv[index] = pointofarg;
                
                //printf("//newargv=%s\n", newargv[index]);
                index++;
                pointofarg = &newbuffer[indexofnewbuffer];
                
            
            } else if (buffer[i] == '\n')
            {
                newbuffer[indexofnewbuffer] = 0;
                indexofnewbuffer = 0;
                newargv[index] = pointofarg;
                newargv[index + 1] = 0;
                index = argc - 1;
                pointofarg = newbuffer;
                if (fork() == 0)
                {
                    exec(argv[1], newargv);
                }
                wait(0);

            }else {
                newbuffer[indexofnewbuffer] = buffer[i];
                indexofnewbuffer++;
            }
            

            
            
        }
        
        
    }
    
    
    
    exit(0);
}


// #include "kernel/types.h"
// #include "user/user.h"

// int main(int argc, char *argv[]){
//     int i;
//     int j = 0;
//     int k;
//     int l,m = 0;
//     char block[32];
//     char buf[32];
//     char *p = buf;
//     char *lineSplit[32];
//     for(i = 1; i < argc; i++){
//         lineSplit[j++] = argv[i];
//     }
//     while( (k = read(0, block, sizeof(block))) > 0){
//         for(l = 0; l < k; l++){
//             if(block[l] == '\n'){
//                 buf[m] = 0;
//                 m = 0;
//                 lineSplit[j++] = p;
//                 p = buf;
//                 lineSplit[j] = 0;
//                 j = argc - 1;
//                 if(fork() == 0){
//                     exec(argv[1], lineSplit);
//                 }                
//                 wait(0);
//             }else if(block[l] == ' ') {
//                 buf[m++] = 0;
//                 lineSplit[j++] = p;
//                 p = &buf[m];
//             }else {
//                 buf[m++] = block[l];
//             }
//         }
//     }
//     exit(0);
// }
