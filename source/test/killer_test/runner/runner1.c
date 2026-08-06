#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	while(1)
	{
		int i;
		for(i=1; i<argc; i++)
		{
			printf("argv[%d]:%s\n", i, argv[i]);
			sleep(2);
		}				
	}
}
