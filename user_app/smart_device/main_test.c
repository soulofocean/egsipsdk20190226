#include <stdio.h>
#include<string.h>
#include <stdlib.h>

int main_test()
{
	char s[10] = "1.23";
	double d = 0;
	d= atof(s);
	printf("%f\n",d);
	char* sss = (void*)0;
	free(sss);
	printf("freenull\n");
	return 0;
}
