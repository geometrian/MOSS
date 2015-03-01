#pragma once

#include "../../../includes.h"


namespace MOSS { namespace Graphics { namespace VGA {


//The VGA has many registers.
//	The external registers have their own IO ports.
				#define MOSS_VGA_MISCOUTR      0x03CC
				#define MOSS_VGA_MISCOUTW      0x03C2

				#define MOSS_VGA_INPUTSTATUS0R 0x03C2
//				                                color    mono
				#define MOSS_VGA_INPUTSTATUS1R 0x03DA //0x03BA
//	The other registers do not.  I don't know of an official name for them, so I'll call them internal registers.
//		These must be indexed into groups: an address register for the group, and a data register for the group
//		(an offset within the register group):
//			Internal registers type 1: Sequencer, Graphics, and CRT Controller Registers
//				The Sequencer has 5 indexed registers
				#define MOSS_VGA_ADDR_SEQ      0x03C4
				#define MOSS_VGA_DATA_SEQ      0x03C5
//				The Graphics Controller has 9 indexed registers
				#define MOSS_VGA_ADDR_GC       0x03CE
				#define MOSS_VGA_DATA_GC       0x03CF
//				The CRTC has 19 indexed registers (although some say 25?)
//				                                color    mono
				#define MOSS_VGA_ADDR_CRTC     0x03D4 //0x03B4
				#define MOSS_VGA_DATA_CRTC     0x03D5 //0x03B5
//			Internal registers type 2: Attribute Registers (some claim there are 21)
				#define MOSS_VGA_ADDR_AC       0x03C0
				#define MOSS_VGA_DATAR_AC      0x03C1
				#define MOSS_VGA_DATAW_AC      0x03C0
//			Internal registers type 3: Color Registers
				#define MOSS_VGA_STATE_DAC     0x03C7 //DAC State Register
				#define MOSS_VGA_ADDRR_PEL     0x03C7 //PEL Address Read Mode Register
				#define MOSS_VGA_ADDRW_PEL     0x03C8 //PEL Address Write Mode Register
				#define MOSS_VGA_DATA_PEL      0x03C9 //PEL Data Register

//1. Input the value of the Address Register and save it for step 6
//2. Output the index of the desired Data Register to the Address Register.
//3. Read the value of the Data Register and save it for later restoration upon termination, if needed.
//4. If writing, modify the value read in step 3, making sure to mask off bits not being modified.
//5. If writing, write the new value from step 4 to the Data register.
//6. Write the value of Address register saved in step 1 to the Address Register.
uint8_t  read_internal_register_type1(uint16_t addr_reg,uint16_t data_reg, uint8_t data_reg_index);
void    write_internal_register_type1(uint16_t addr_reg,uint16_t data_reg, uint8_t data_reg_index,uint8_t data_reg_value);

//1. Input a value from the Input Status #1 Register (normally port 3DAh) and discard it.
//2. Read the value of the Address/Data Register and save it for step 7.
//3. Output the index of the desired Data Register to the Address/Data Register
//4. Read the value of the Data Register and save it for later restoration upon termination, if needed.
//5. If writing, modify the value read in step 4, making sure to mask off bits not being modified.
//6. If writing, write the new value from step 5 to the Address/Data register.
//7. Write the value of Address register saved in step 1 to the Address/Data Register.
//8. If you wish to leave the register waiting for an index, input a value from the Input Status #1 Register (normally port 3DAh) and discard it.
uint8_t  read_internal_register_type2(uint8_t data_reg_index);
void    write_internal_register_type2(uint8_t data_reg_index,uint8_t data_reg_value);


}}}