#pragma once

#include "../../../includes.hpp"

#include "bus.hpp"


namespace MOSS { namespace Disk { namespace ATA {


class Device final {
	private:
		Bus*const _bus;
	public:
		//0: master
		//1: slave
		int const index;

		bool valid;

		bool supports_dma;
		bool supports_lba;

		uint32_t num_cylinders;
		uint32_t num_heads;
		uint32_t num_sectors;
		uint32_t capacity; //in sectors?

		char model[40];
		char serial[20];
		char firmware[8];

	public:
		Device(Bus* bus, int index);
		inline ~Device(void) = default;

		void fill_information(void);

		//Updates the drive's control register.  Note: the drive must be current on the bus to accept changes!
		//void update_control_register(void) const;

		void print(int level) const;
};


}}}
