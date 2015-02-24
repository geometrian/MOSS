//#define SIMPLE

#ifndef SIMPLE
#include <stdio.h>
#include <string.h>
#endif

#include "../source/mossc/cstdio"


int main(int argc, char* argv[]) {
	char buffer[1024]; char* buffer2=buffer;

	for (int i=0;i<256;++i) {
		buffer2[0]=' ';
		++buffer2;
		buffer2 += MOSSC::sprintf(buffer2,"%u",i);
	}
	int num_printed = buffer2 - buffer;
	//int num_printed = MOSSC::sprintf(buffer,"Hello|%5.3d|there!",-61);
	//int num_printed = MOSSC::sprintf(buffer,"%+3d",4);
	//int num_printed = MOSSC::sprintf(buffer,"%03u",67);
	//int num_printed = MOSSC::sprintf(buffer,"%c",'H');
	//int num_printed = MOSSC::sprintf(buffer,"|%4s|","He");
	//int num_printed = MOSSC::sprintf(buffer,"%%");
	//int num_printed = MOSSC::sprintf(buffer,"%d%d%d",1,2,3);

	double a = 0.0;
	double b = -1.0;
	double c = 1.0;
	double ninf = b/a;
	double pinf = c/a;
	double nan = a/a;
	//int num_printed = MOSSC::sprintf(buffer,"0.1436=%f 1.0=%f 14.0=%f\n-13.9876357=%.1f 306.14=%f\n-INF=%f +INF=%f NaN=%f\n", 0.1436,1.0,14.0, -13.9876357,306.14, ninf,pinf,nan);
	//int num_printed = MOSSC::sprintf(buffer,"13.987=%f 9999.963984=%.4f 0.68438=%f", 13.987, 9999.963984, 0.68438);

	//int num_printed = MOSSC::sprintf(buffer,"%p",0xA1B2C3D4);

	//printf("%d\n",printf("_"));
	//printf("\"%-+4c\"\n",'H');

	#ifndef SIMPLE
	printf("\"%s\"(%d)\n",buffer,num_printed);

	getchar();
	#endif

	return 0;
}
