#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main()
{
	printf("这是新应用 id = %d\n", getpid());
	return 0;
}