#include <execinfo.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct school_info
{
     char name[8];
     int rooms_count;
} school_info;

int school_init(school_info *school, const char *name, int count)
{
   if (count > 40)
   {
       printf("%s has %d rooms beyond limit\n", name, count);
       return -1;
   }
   school->rooms_count = count;
   strcpy(school->name, name);

   return 0;
}

int main(int argc, char *argv[])
{
   int rev, i;
   const char *school_prefix = "SCHOOL";
   char school_name[6];
    // char school_name[8];
   school_info *schools = malloc(sizeof(school_info) * 10);
   
   for (i = 0; i < 10; i ++)
   {
       school_info *school;
       int room_count;
       school = &schools[i];
       room_count = i - 1;
       room_count /= 5;
       room_count += 1;
       room_count *= 5;
        
       sprintf(school_name, "%s%d", school_prefix, i + 1);
       rev = school_init(school, school_name, room_count);
       if (rev == -1) {
           printf("school init failed\n");
           exit(-1);
       }
    
       printf("[%s] has %d rooms\n", school->name,
              school->rooms_count);
   }

   free(schools);
   return 0;
}
