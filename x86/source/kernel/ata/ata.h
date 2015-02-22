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

		uint8_t buffer[512];

		bool got_irq;

	public:
		Controller(void) {
			current = None;
			got_irq = false;
		}
		~Controller(void) {
		}

		void handle_irq(void) {
			got_irq = true;
		}
		void reset_irq(void) {
			got_irq = false;
		}

		const uint8_t* read_sector(uint64_t lba) {
			for (int i=0;i<512;++i) buffer[i]=66; //debug value

			read_sectors(lba, buffer,1);

			return buffer;
		}
		void read_sectors(uint64_t lba, uint8_t* data_buffer,unsigned int num_sectors) {
			ASSERT(lba<=562949953421311ull,"LBA is larger than (2^49)-1!"); //Only have 48 bits (equals maximum (2^49)-1 addresses = 512TiB)
			ASSERT(num_sectors<=65536,"Number of sectors was larger than 65536!"); //Can't be unsigned short, since 65536 is valid (we have to pass it as zero though):
			unsigned short num_sectors_arg = num_sectors==65536 ? 0 : num_sectors;

			//http://wiki.osdev.org/ATA_PIO_Mode#x86_Directions

			//Send 0x40 for the "master" or 0x50 for the "slave" to port 0x01F6:
			IO::send<uint8_t>(0x01F6, 0x40);

			IO::send<uint8_t>(0x01F2, (num_sectors_arg&        0x0000FF00)>> 8); //high byte of sector count
			IO::send<uint8_t>(0x01F3, (            lba&0x00000000FF000000)>>24); //byte 4 of LBA
			IO::send<uint8_t>(0x01F4, (            lba&0x000000FF00000000)>>32); //byte 5 of LBA
			IO::send<uint8_t>(0x01F5, (            lba&0x0000FF0000000000)>>40); //byte 6 of LBA
			IO::send<uint8_t>(0x01F2,  num_sectors_arg&        0x000000FF     ); //low byte of sector count
			IO::send<uint8_t>(0x01F3,              lba&0x00000000000000FF     ); //byte 1 of LBA
			IO::send<uint8_t>(0x01F4, (            lba&0x000000000000FF00)>> 8); //byte 2 of LBA
			IO::send<uint8_t>(0x01F5, (            lba&0x0000000000FF0000)>>16); //byte 3 of LBA

			//Send the "READ SECTORS EXT" command (0x24) to port 0x01F7:
			IO::send<uint8_t>(0x01F7, 0x24);

			for (unsigned int i=0;i<num_sectors;++i) {
				//Wait for an IRQ or poll
				while (!got_irq);
				reset_irq();

				//Transfer 256 words, a word at a time, into your buffer from I/O port 0x01F0. (In assembler, REP INSW works well for this).
				for (int i=0;i<512;i+=2) {
					uint16_t word = IO::recv<uint16_t>(0x01F0);
					data_buffer[i  ] = (uint8_t)( word&0x00FF    );
					data_buffer[i+1] = (uint8_t)((word&0xFF00)>>8);
				}

				//Loop back to waiting for the next IRQ (or poll again -- see next note) for each successive sector.
			}
			//Note for polling PIO drivers: after transferring the last word of a PIO data block to the data IO port, give the drive a
			//400ns delay to reset its DRQ bit (and possibly set BSY again, while emptying/filling its buffer to/from the drive).
		}

		void drive_select(Drive drive) {
			current = drive;
		}
};


}}