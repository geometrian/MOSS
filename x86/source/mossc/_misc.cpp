#include "_misc.h"


void delay(int ms) { //approximately correct on VirtualBox
	ms /= 2;
	volatile int counter = 0;
	for (int i=0;i<1000;++i) {
		for (int j=0;j<1000;++j) {
			for (int k=0;k<ms;++k) {
				++counter;
			}
		}
	}
}