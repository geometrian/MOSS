#pragma once

#include "../../includes.h"


namespace MOSS { namespace ATA {


//Based on information here:
//	http://wiki.osdev.org/ATA_PIO_Mode
//		A lot of good information, although http://wiki.osdev.org/ATA_PIO_Mode#Primary.2FSecondary_Bus seems inaccurate.
//	http://forum.osdev.org/viewtopic.php?f=1&p=167798#p167798

class Controller;

class Drive;

class Channel final {
	private:
		Controller*const controller;
	public:
		enum ChannelType {
			Primary,
			Secondary
		} const type;
		enum DriveType {
			Master,
			Slave
		};

		volatile bool got_irq;
		uint8_t* irq_buffer;

		Drive* drive0;
		Drive* drive1;
		Drive* current;

	public:
		Channel(Controller* controller, ChannelType type);
		~Channel(void);

		void handle_irq(void);
	private:
		void _read_sector(void);
	public:
		void reset_irq(void);

		void drive_select(DriveType drive);
};

class Drive final {
	private:
		Channel*const channel;
	public:
		Channel::DriveType const type;
		uint16_t drive_iobase;

		class StatusByte { public:
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
		} __attribute__((packed));
		static_assert(sizeof(StatusByte)==1,"StatusByte is the wrong size!");

	private:
		bool interrupts;

	public:
		Drive(Channel* channel, Channel::DriveType type);
		~Drive(void);

		void set_interrupts(bool enable);
	private:
		void update_control_register(void);

	public:
		StatusByte get_status(void) const;

		void read_sectors(uint64_t lba, uint8_t* data_buffer,unsigned int num_sectors);
};

class Controller final {
	public:
		Channel* channel0;
		Channel* channel1;

	private:
		uint8_t buffer[512];

	public:
		Controller(void);
		~Controller(void);

		const uint8_t* read_sector(uint64_t lba);
		void read_sectors(uint64_t lba, uint8_t* data_buffer,unsigned int num_sectors);
};


}}