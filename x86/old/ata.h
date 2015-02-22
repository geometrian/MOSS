#pragma once
#include "../../includes.h"

#include "../io/io.h"


namespace MOSS { namespace ATA {


class Controller {
	public:
		enum Drive {
			None   = -1,
			Drive1 =  0,
			Drive2 =  1
		};
	private:
		Drive current;

	public:
		Controller(void) {
			current = None;
		}
		~Controller(void) {
		}

		void drive_select(Drive drive) {
			current = drive;
		}
};


extern "C" void ata_lba_read(int lba, unsigned short sector_count, char* data);

const char* read(void) {
	static char buffer[512];
	for (int i=0;i<512;++i) buffer[i] = 66;

	ata_lba_read(0,1,buffer);

	return buffer;
}


}}