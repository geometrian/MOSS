//#define SIMPLE

#ifndef SIMPLE
#include <stdio.h>
#include <string.h>
#endif

#include "../source/mossc/cstdio"


int main(int argc, char* argv[]) {
	char buffer[1024];

	//int num_printed = sprintf(buffer,"Hello|%5.3d|there!",-61);
	//int num_printed = sprintf(buffer,"%03u",67);
	//int num_printed = sprintf(buffer,"%c",'H');
	int num_printed = sprintf(buffer,"|%4s|","He");
	//int num_printed = sprintf(buffer,"%%");
	//int num_printed = sprintf(buffer,"%d%d%d",1,2,3);
	//int num_printed = sprintf(buffer,"%f",3.14f);

	//printf("%d\n",printf("_"));
	//printf("\"%-+4c\"\n",'H');

	#ifndef SIMPLE
	printf("\"%s\"(%d)\n",buffer,num_printed);

	getchar();
	#endif

	return 0;
}