#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


// is prime then return 1, else return 0;
int isprime(int num);

void left2right(int count, int ns[34]);

int
main(int argc, char *argv[])
{
    int nums[34];
    
    for (int i = 2; i < 36; i++)
    {
        nums[i-2] = i;
    }

    left2right(0, nums);
    exit(0);
}

int isprime(int num){
    for (int i = 2; i < num; i++)
    {
        if (num%i == 0)
        {
            return 0;
        }
        
    }
    return 1;
}

void left2right(int count, int nums[34]){
    printf("prime %d\n", nums[count]);

    if (nums[count] == 31)
    {
        return;
    }
    
    
    int p[2];
 
    pipe(p);
    if (fork() == 0){
        //close(p[0]);
        write(p[1], nums + count, 34 - count);
        
        exit(0);
        
    } 
    close(p[1]);

    if (fork() == 0){
        int *buffer = malloc(34 - count);
        int readlen = read(p[0], buffer, 34 - count);
        count++;
        if (readlen != 0)
        {
            while (!isprime(nums[count]))
            {
                count++;
            }
            
        }
        left2right(count, nums);
        exit(0);
        
    } 
    wait(0);
    wait(0);

}
