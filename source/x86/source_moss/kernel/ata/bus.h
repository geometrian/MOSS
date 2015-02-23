#pragma once

#include "../../includes.h"


namespace MOSS { namespace ATA {


class Controller;
class Drive;

//ATA Bus/Channel
class Bus final {
	friend class Drive;
	private:
		Controller*const _controller;
	public:
		//0: primary
		//1: secondary
		//2: tertiary
		//3: quaternary
		int const index;
		bool valid;
	private:
		uint16_t const _io_base;

		//pointer to the data buffer passed into the reading sectors method
		uint8_t* _irq_buffer;

	public:
		//User can read but should not change.
		Drive*restrict drives[2];
		Drive* current;

	public:
		Bus(Controller* controller, int index);
		~Bus(void);

		void drive_select(int drive_index);

		void handle_irq(void);
	private:
		void _read_sector_from_current_drive(void);

	public:
		//Read up to 65,536 sectors from the current drive
		void read_sectors_from_current_drive(uint64_t lba, uint8_t* data_buffer,int num_sectors);
};


}}