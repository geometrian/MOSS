#pragma once

#include "../../includes.h"

#include "bus.h"


namespace MOSS { namespace ATA {


class Drive final {
	private:
		Bus*const _bus;
	public:
		//0: master
		//1: slave
		int const index;

		bool valid;

		bool supports_lba;
		bool supports_dma;

		uint32_t num_cylinders;
		uint32_t num_heads;
		uint32_t num_sectors;
		uint32_t capacity; //in sectors?

		char model[40];
		char serial[20];
		char firmware[8];

	private:
		class StatusByte final { public:
			union {
				struct {
					//Note: if bsy is set, the other fields are technically invalid.
					bool     err : 1; //0: An error occurred.  Send new command to clear it (or nuke it with a Software Reset).
					bool         : 1; //1: unused/deprecated?
					bool         : 1; //2: unused/deprecated?
					bool     drq : 1; //3: The drive has PIO data to transfer, or is ready to accept PIO data.
					bool     srv : 1; //4: Overlapped Mode Service Request
					bool      df : 1; //5: Drive fault error (does *not* set bit 0)
					bool     rdy : 1; //6: 0=drive is spun down/after an error, 1=otherwise
					bool     bsy : 1; //7: The drive is preparing to send/receive data (wait for it to clear).  If it never does, do a software reset.
				};
				uint8_t byte;
			};

			inline StatusByte(void) {}
			inline StatusByte(uint8_t value) : byte(value) {}
			inline ~StatusByte(void) {}
		} __attribute__((packed));
		static_assert(sizeof(StatusByte)==sizeof(uint8_t),"StatusByte is the wrong size!");

	public:
		Drive(Bus* bus, int index);
		inline ~Drive(void) {}

		//Initialize drive.  Note: the drive should be current on the bus.
		void init(void);

		//Select this drive on the bus
		void select(void) const;

		//Read a sector that was previously advertised as ready.  Note: the drive should be current on the bus.
		void read_ready_sector(uint8_t* data) const;

		//Returns the drive's status.  Note: the drive should be current on the bus.
		StatusByte get_status(void) const;

		//Updates the drive's control register.  Note: the drive must be current on the bus to accept changes!
		void update_control_register(void) const;

		void print(int indent) const;
};


}}