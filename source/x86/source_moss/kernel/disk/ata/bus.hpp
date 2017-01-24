#pragma once

#include "../../../includes.hpp"

#include "../disk.hpp"

#include "ata.hpp"


namespace MOSS { namespace Disk { namespace ATA {


class Controller;
class Device;

//ATA Bus/Channel
class Bus final {
	friend class Device;
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
		uint8_t _current_command;
		uint8_t* _current_command_buffer;

		//7.15.6
		class _StatusByte final { public:
			union {
				struct {
					//Note: if bsy is set, the other fields are technically invalid.
					bool     err : 1; //0:   7.15.6.6:   Error:          An error occurred during the execution of the previous command.
					bool         : 1; //1:                               Obsolete
					bool         : 1; //2:                               Obsolete
					bool     drq : 1; //3:   7.15.6.4:   Data Request:   The device is ready to send/receive a word.
					bool     srv : 1; //4:                               Command dependent meaning.  OSDev: Overlapped Mode Service Request
					bool      df : 1; //5:                               Command dependent meaning.  OSDev: Drive fault error (does *not* set bit 0)
					bool    drdy : 1; //6:   7.15.6.2:   Device Ready:   0=power-on, hardware/software reset, etc., 1=ready
					bool     bsy : 1; //7:   7.15.6.1:   Busy:           The device is busy.
				};
				uint8_t byte;
			};

			inline _StatusByte(void) {}
			inline _StatusByte(uint8_t value) : byte(value) {}
			inline ~_StatusByte(void) {}

			void print(void) const;
		} __attribute__((packed));
		static_assert(sizeof(_StatusByte)==sizeof(uint8_t),"StatusByte is the wrong size!");

	public:
		//User can read but should not change.
		Device*restrict devices[2];
		Device* current;

	public:
		Bus(Controller* controller, int index);
		~Bus(void);

	private:
		void _transfer_block_into(uint8_t* data) const;

		void _select_device(int device_index);
		void _select_device_as_necessary(int device_index);
		void _proto_pio_data_in(uint8_t const* end);
	public:
		//void command_resetdevice(int device_index);
		//void command_flushcache(int device_index);
		bool command_identify(int device_index, uint8_t data_buffer[512]);
		//Read up to 256 sectors starting at "lba" into "data_buffer".
		void command_readsectors(int device_index, uint8_t* data_buffer, AbsoluteLBA lba,int num_sectors);
		//void command_writesectors(int device_index);

		void handle_irq(void);

	private:
		uint8_t    _read_uint8( Read::RegisterOffset offset               ) const;
		uint16_t  _read_uint16( Read::RegisterOffset offset               ) const;
		void      _write_uint8(Write::RegisterOffset offset,  uint8_t data) const;
		void     _write_uint16(Write::RegisterOffset offset, uint16_t data) const;

	public:
		void print(int level) const;
};


}}}
