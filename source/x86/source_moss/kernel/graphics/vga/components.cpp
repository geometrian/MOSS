#include "components.h"

#include "../../io/io.h"
#include "../../kernel.h"


namespace MOSS { namespace Graphics { namespace VGA {


CathodeRayTubeController::CathodeRayTubeController(void) {
	mode = text80x25;
	cols = 80;
	rows = 25;
	//TODO: should probably check
}

void CathodeRayTubeController::set_mode(Mode mode) {
	//Refer to this diagram:
	//	http://wiki.osdev.org/VGA_Hardware#The_CRT_Controller

	//Disable display
	//	TODO: this!

	//Unlock CRTC
	unlock_registers();

	//Load registers
	//	External:
	//		Clock select:
	//			00 is 25MHz clock (for 320* or 640* modes)
	//			01 is 28MHz clock (for 360* or 720* modes)
	//			Note: always 01 because only 720* modes are supported
	uint8_t misc = IO::recv<uint8_t>(MOSS_VGA_MISCOUTR); //default 0x67=0b01100111
	IO::send<uint8_t>(MOSS_VGA_MISCOUTW, (misc&0b11110011)|0b00000100);
	//		Clocking mode (a bletcherous 9 dots per character, the 8-dot modes aren't supported by VGA)
	write_internal_register_type1(MOSS_VGA_ADDR_SEQ,MOSS_VGA_DATA_SEQ, 0x01,0b00000000); //default 0x00
	//	Internal:
	//		On a VGA monitor, the horizontal refresh rate should equal 31,250Hz.
	//			Horizontal Refresh Rate = Clock Frequency (in Hz) / total pixels horizontally = 28MHz / p = 31,250Hz
	//			Vertical Refresh Rate = Horizontal Refresh Rate / total scan lines = 31,250Hz / total scan lines
	//		So the width should be a virtual 896 pixels (99 5/9 characters).

	switch (mode) {
		case  text80x25: cols= 80; rows=25; break;
		case  text80x50: cols= 80; rows=50; break;
		case  text80x60: cols= 80; rows=60; break;
		case text132x25: cols=132; rows=25; break;
		case text132x43: cols=132; rows=43; break;
		case text132x50: cols=132; rows=50; break;
		case text132x60: cols=132; rows=60; break;
		default: assert_term(false,"Implementation error!");
	}

	uint16_t font_height = 16;

	uint8_t horiz_index_displayend   =                      cols - 1; //==cols- 1; default  79
	uint8_t horiz_index_blankstart   =   horiz_index_displayend +  1; //==cols;    default  80
	uint8_t horiz_index_retracestart =   horiz_index_blankstart +  5; //==cols+ 5; default  85
	uint8_t horiz_index_retraceend   = horiz_index_retracestart + 11; //==cols+16; default  96
	uint8_t horiz_index_blankend     =   horiz_index_retraceend +  2; //==cols+18; default  98
	uint8_t horiz_index_total        =                           150; //           default 100
	assert_term(horiz_index_total>=horiz_index_blankend,"Implementation error!");
	//		Horizontal Total
	write_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x00,horiz_index_total-5); //default 0x5F=95
	//		Horizontal Display End
	write_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x01,horiz_index_displayend); //default 0x4F=79
	//		Horizontal Blanking Start
	write_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x02,horiz_index_blankstart); //default 0x50=80
	//		Horizontal Display Skew (none), Horizontal Blanking End
	write_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x03,0x80|(horiz_index_blankend&0x1F)); //default 0x82=130
	//		Horizontal Retrace Start
	write_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x04,horiz_index_retracestart); //default 0x55=85
	//		Horizontal Blanking End, Horizontal Retrace End
	write_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x05,((horiz_index_blankend<<2)&0x80) | (horiz_index_retraceend&0x1F)); //default 0x81

	uint16_t vert_scanline_displayend   =           rows*font_height - 1; //default 0x018F       =399
	uint16_t vert_scanline_blankstart   =   vert_scanline_displayend + 2; //default 0x0196       =406
	uint16_t vert_scanline_retracestart =   vert_scanline_blankstart + 2; //default 0x019C       =412
	uint16_t vert_scanline_retraceend   = vert_scanline_retracestart + 2; //default 0x???E=0x019E=414 ?
	uint16_t vert_scanline_blankend     =   vert_scanline_retraceend + 2; //default 0x??39=0x01A9=425 ?
	uint16_t vert_scanline_total        =                            447; //default 0x01BF       =447
	assert_term(vert_scanline_total>=vert_scanline_blankend,"Implementation error!");
	//		Vertical Total
	write_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x06,vert_scanline_total&0xFF); //default 0xBF
	//		V.Retr.St., V.Disp.End, V.Total, V.Blank.St., V.Retr.St., V.Disp.End, V.Total
	write_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x07,
		((vert_scanline_retracestart&0x0200)>>2) | //default 0
		((  vert_scanline_displayend&0x0200)>>3) | //default 0
		((       vert_scanline_total&0x0200)>>4) | //default 0
		0b00010000 | //line compare register         default 1
		((  vert_scanline_blankstart&0x0100)>>5) | //default 1
		((vert_scanline_retracestart&0x0100)>>6) | //default 1
		((  vert_scanline_displayend&0x0100)>>7) | //default 1
		((       vert_scanline_total&0x0100)>>8)   //default 1
	); //default 0x1F
	//		Vertical Blanking Start
	write_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x09,
		0b01000000 | //9th bit of line compare 0x03FF
		((vert_scanline_blankstart&0x0200)>>4) | //9th bit of vertical blanking start
		(font_height - 1)
	); //default 0x4F=0b01001111
	//		Vertical Retrace Start
	write_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x10,vert_scanline_retracestart&0xFF); //default 0x9C=156
	//		Vertical Retrace End
	//			Note not setting 0x80; the registers should be unprotected now.
	write_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x11,vert_scanline_retraceend&0x0F); //default 0x8E
	//		Vertical Display End
	write_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x12,vert_scanline_displayend&0xFF); //default 0x8F
	//		Vertical Blanking Start
	write_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x15,vert_scanline_blankstart&0xFF); //default 0x96
	//		Vertical Blanking End
	uint8_t reg = read_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x16); //default 0xB9=0b10111001 (which is weird since bit 7 doesn't exist)
	write_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x16,(reg&0x80)|(vert_scanline_blankend&0x7F));

	//		Line Compare Register (no horizontal division; set to 0x03FF), with bit 8 in overflow register and bit 9 in maximum scan line register
	write_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x18,0xFF); //default 0x??

	//Clear the screen contents

	//For text mode, load fonts.  Note that this may need to alter GC
	//	settings, so be sure to restore those after that.

	//Lock CRTC (technically optional)
	lock_registers();

	//Enable display
	//	TODO: this!

	this->mode = mode;
}

void CathodeRayTubeController::unlock_registers(void) {
	uint8_t value;

	value = read_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x11);
	write_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x11,value&~0x80);
}
void CathodeRayTubeController::lock_registers(void) {
	uint8_t value;

	value = read_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x11);
	write_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x11,value|0x80);
}


void GraphicsController::set_plane(int plane_index) {
	assert_term(plane_index>=0&&plane_index<4,"Invalid plane index \"%d\"!",plane_index);

	//Set read plane
	IO::send<uint8_t>(MOSS_VGA_ADDR_GC, 0x04);
	IO::send<uint8_t>(MOSS_VGA_DATA_GC, plane_index);

	//Set write plane
	IO::send<uint8_t>(MOSS_VGA_ADDR_SEQ, 0x02);
	IO::send<uint8_t>(MOSS_VGA_DATA_SEQ, 1<<plane_index);
}


void Interface::dump_registers(void) const {
	kernel->write("Miscellaneous Output Register:      %X\n\n",IO::recv<uint8_t>(MOSS_VGA_MISCOUTR));

	kernel->write("Clocking Mode Register:             %X\n\n",read_internal_register_type1(MOSS_VGA_ADDR_SEQ,MOSS_VGA_DATA_SEQ, 0x01));

	kernel->write("Horizontal Total Register:          %X\n",  read_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x00));
	kernel->write("End Horizontal Display Register:    %X\n",  read_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x01));
	kernel->write("Start Horizontal Blanking Register: %X\n",  read_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x02));
	kernel->write("End Horizontal Blanking Register:   %X\n",  read_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x03));
	kernel->write("Start Horizontal Retrace Register:  %X\n",  read_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x04));
	kernel->write("End Horizontal Retrace Register:    %X\n\n",read_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x05));

	kernel->write("Vertical Total Register:            %X\n",  read_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x06));
	kernel->write("Overflow Register:                  %X\n",  read_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x07));
	kernel->write("Maximum Scan Line Register:         %X\n",  read_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x09));
	kernel->write("Vertical Retrace Start Register:    %X\n",  read_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x10));
	kernel->write("Vertical Retrace End Register:      %X\n",  read_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x11));
	kernel->write("Vertical Display End Register:      %X\n",  read_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x12));
	kernel->write("Start Vertical Blanking Register:   %X\n",  read_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x15));
	kernel->write("End Vertical Blanking:              %X\n\n",read_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x16));

	kernel->write("Line Compare Register:              %X\n",  read_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x18));
}


}}}