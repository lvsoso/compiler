#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>


int math_pow(int num_1, int num_2)
{
    int i, rev;
    rev = 1;
    for (i = 0; i < num_1; i ++)
    {
        rev *= num_2;
    }
    return rev;
}

int sub_thread()
{
    int num_1, num_2;
    printf("sub_thread pid=%d\n", (int)getpid());
    sleep(1);
    while(1) {
        num_1 = rand() % 10;
        num_2 = rand() % 10;
        printf("sub_thread %d pow %d=%d\n", num_1, num_2, math_pow(num_1, 
                 num_2));
        sleep(2);
    }
}
  
int main_thread()
{
    int num_1, num_2;
    printf("main thread pid=%d\n", (int)getpid());
    while(1) {
        num_1 = rand() % 10;
        num_2 = rand() % 10;
        printf("main thread %d x %d=%d\n", num_1, num_2, math_pow(num_1, num_2));
        sleep(2);
    }
    return 0;
}

void *pthread_entry(void *param)
{
    sub_thread();
    return NULL;
}

int main(int argc, char *argv[])
{
    int rev;
    pthread_t thread;
    
    rev = pthread_create(&thread, NULL, pthread_entry, NULL);
    if (rev != 0)
    {
        printf("faile to create pthread errno:%d\n", errno);
        return -1;
    }
    main_thread();
    return 0;
}