#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "proc_maps.h"

#define MAX_LINE 4096
int rand_number;
  
void suspend(void)
{
   int *wild_p;*wild_p = NULL;
}
 
void show_rand_number(void)
{
   char *buf = alloca(MAX_LINE);
   sprintf(buf, "rand_number is %d", rand_number);
   puts(buf);
   suspend();
}
  
char g_array[16 * 1024 * 1024] = "This area record static global data.";

int main(int argc, char *argv[])
{

    do {
       rand_number = rand() % 100;
       show_rand_number();
       sleep(1);
    } while(1);
   
    return 0;
}