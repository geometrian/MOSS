#include "components.h"

#include "../../../mossc/string/copying.h"

#include "../../io/io.h"
#include "../../kernel.h"

#include "../font.h"


namespace MOSS { namespace Graphics { namespace VGA {


CathodeRayTubeController::CathodeRayTubeController(Interface* interface) : _interface(interface) {
	mode = text80x25;
	cols = 80;
	rows = 25;
	font_height = 16;
	//TODO: should probably check
}

void CathodeRayTubeController::set_mode(Mode mode) {
	switch (mode) {
		case  text80x25: cols= 80; rows=25; break;
		case  text80x50: cols= 80; rows=50; break;
		case  text80x60: cols= 80; rows=60; break;
		case text128x48: cols=128; rows=48; break;
		case text128x80: cols=128; rows=80; break;
		case text132x25: cols=132; rows=25; break;
		case text132x43: cols=132; rows=43; break;
		case text132x50: cols=132; rows=50; break;
		case text132x60: cols=132; rows=60; break;
		default: assert_term(false,"Implementation error!");
	}
	this->mode = mode;

	//Refer to this diagram:
	//	http://wiki.osdev.org/VGA_Hardware#The_CRT_Controller

	//Disable display
	//	TODO: this!

	//Unlock CRTC
	unlock_registers();
	//Load registers
	_interface->regs.read_all_registers();

	//	External:
	//		Clock select:
	//			00 is 25MHz clock (for 320* or 640* modes)
	//			01 is 28MHz clock (for 360* or 720* modes)
	_interface->fields.ClockSelect = 0b01; //Note: always 01 because only 720* modes are supported
	//		Clocking mode (note: the 8-dot modes aren't technically supported by VGA according to that table, but
	//			I can't imagine why it wouldn't work).  Note: if set to 9-bit mode, should set the line graphics
	//			enable bit, which copies in the last column of the line graphics characters into the 9th column
	//			where other characters would have a gap.
	_interface->fields.DotMode = true; //default 0

	//	Internal:
	//		On a VGA monitor, the horizontal refresh rate should equal 31,250Hz.
	//			Horizontal Refresh Rate = Clock Frequency (in Hz) / total pixels horizontally = 28MHz / p = 31,250Hz
	//			Vertical Refresh Rate = Horizontal Refresh Rate / total scan lines = 31,250Hz / total scan lines
	//		So the width should be a virtual 896 pixels (99 5/9 characters).

	//_interface->fields.DisplayEnableSkew = 0b00;

	_interface->fields.MaximumScanLine = font_height - 1;

	uint8_t horiz_index_displayend   =                      cols - 1; //==cols- 1; default  79
	uint8_t horiz_index_blankstart   =   horiz_index_displayend +  1; //==cols;    default  80
	uint8_t horiz_index_retracestart =   horiz_index_blankstart +  5; //==cols+ 5; default  85
	uint8_t horiz_index_retraceend   = horiz_index_retracestart + 11; //==cols+16; default  96
	uint8_t horiz_index_blankend     =   horiz_index_retraceend +  2; //==cols+18; default  98
	uint8_t horiz_index_total        =                           150; //           default 100
	assert_term(horiz_index_total>=horiz_index_blankend,"Implementation error!  Horizontal total %u less than blanking end %u!",horiz_index_total,horiz_index_blankend);

	//Supposedly, this is supposed to be Width/(MemoryAddressSize*2).  The default is 20=80/(2*sizeof(uint16_t)),
	//	but this doesn't seem to work, especially for larger sizes.  Instead, use cols/sizeof(uint16_t).
	_interface->fields.Offset = cols / sizeof(uint16_t);

	_interface->fields.HorizontalTotal         = horiz_index_total - 5;         //default 0x5F=95
	_interface->fields.HorizontalDisplayEnd    = horiz_index_displayend;        //default 0x4F=79
	_interface->fields.HorizontalBlankingStart = horiz_index_blankstart;        //default 0x50=80
	_interface->fields.HorizontalBlankingEnd   = horiz_index_blankend & 0x3F;   //default 0x22=34
	_interface->fields.HorizontalRetraceStart  = horiz_index_retracestart;      //default 0x55=85
	_interface->fields.HorizontalRetraceEnd    = horiz_index_retraceend & 0x1F; //default 0x01

	uint16_t vert_scanline_displayend   =           rows*font_height - 1; //default 0x018F       =399
	uint16_t vert_scanline_blankstart   =   vert_scanline_displayend + 2; //default 0x0196       =406
	uint16_t vert_scanline_retracestart =   vert_scanline_blankstart + 2; //default 0x019C       =412
	uint16_t vert_scanline_retraceend   = vert_scanline_retracestart + 2; //default 0x???E=0x019E=414 ?
	uint16_t vert_scanline_blankend     =   vert_scanline_retraceend + 2; //default 0x??39=0x01A9=425 ?
	uint16_t vert_scanline_total        =                            967; //default 0x01BF       =447
	assert_term(vert_scanline_total>=vert_scanline_blankend,"Implementation error!  Vertical total %u less than blanking end %u!",vert_scanline_total,vert_scanline_blankend);

	_interface->fields.VerticalTotal         = vert_scanline_total;             //default 0x01BF=447
	_interface->fields.VerticalDisplayEnd    = vert_scanline_displayend;        //default 0x018F=399
	_interface->fields.VerticalBlankingStart = vert_scanline_blankstart;
	_interface->fields.VerticalBlankingEnd   = vert_scanline_blankend & 0x7F;
	_interface->fields.VerticalRetraceStart  = vert_scanline_retracestart;
	_interface->fields.VerticalRetraceEnd    = vert_scanline_retraceend & 0x0F;
	_interface->fields.LineCompare           = 0x03FF; //(requesting no horizontal division)

	_interface->regs.write_all_registers();

	//Clear the screen contents

	//For text mode, load fonts.  Note that this may need to alter GC
	//	settings, so be sure to restore those after that.

	//Lock CRTC (technically optional)
	lock_registers();

	//Enable display
	//	TODO: this!
}

void CathodeRayTubeController::unlock_registers(void) {
	_interface->fields.ProtectCRTC.load_regs();
	_interface->fields.ProtectCRTC = false;
	_interface->fields.ProtectCRTC.save_regs();
}
void CathodeRayTubeController::lock_registers(void) {
	_interface->fields.ProtectCRTC.load_regs();
	_interface->fields.ProtectCRTC = true;
	_interface->fields.ProtectCRTC.save_regs();
}


void GraphicsController::set_plane(int plane_index) {
	assert_term(plane_index>=0&&plane_index<4,"Invalid plane index \"%d\"!",plane_index);

	//Set read plane
	_interface->fields.ReadMapSelect.load_regs();
	_interface->fields.ReadMapSelect = plane_index;
	_interface->fields.ReadMapSelect.save_regs();

	//Set write plane
	_interface->fields.WritePlaneEnable.load_regs();
	_interface->fields.WritePlaneEnable = 1 << plane_index;
	_interface->fields.WritePlaneEnable.save_regs();
}


void Interface::_set_use_font(uint8_t const* font_buffer, int font_height) {
	//http://compbio.cs.toronto.edu/repos/snowflock/xen-3.0.3/xen/drivers/video/vga.c
	assert_term(font_height==8||font_height==16,"Invalid font height %d!  Must be 8 or 16.",font_height);
	crtc.font_height = font_height;

	#if 1
		//Unlock CRTC
		crtc.unlock_registers(); //TODO: necessary?
		//Load registers
		regs.read_all_registers();

		//Turn off even/odd addressing
		fields.OEHostMem=0b110; fields.OEHostMem.save_regs();

		//Select font plane
		_gc.set_plane(2);

		//Write font
		/*uint8_t const blah[16] = {
			0b11111111,
			0b10000001,
			0b11100001,
			0b10000001,
			0b10000001,
			0b10000001,
			0b10000001,
			0b11100111,

			0b11100111,
			0b10000001,
			0b10000001,
			0b10000001,
			0b10000111,
			0b10000001,
			0b11111111,
		};*/
		for (int i=0;i<256;++i) {
			MOSSC::memcpy(
				//Note: font slots other than 0 might not be respected on AMD cards
				reinterpret_cast<void*>(0xB800*16 /*+ 16384*font_slot*/ + i*32),
				font_buffer + font_height*i,
				//blah,
				font_height
			);
		}

		//Select normal plane
		_gc.set_plane(0);

		//Turn even/odd adressing back on
		fields.OEHostMem = 0b001;

		regs.write_all_registers();
		crtc.lock_registers();
	#else
		//http://wiki.osdev.org/VGA_Fonts#Get_from_VGA_RAM_directly
		//	Clear even/odd mode
		IO::send<uint8_t>(0x03CE,0x05);
		//	Map VGA memory to 0x000A0000
		IO::send<uint16_t>(0x03CE,0x0406);
		//	Set bitplane 2
		IO::send<uint16_t>(0x03C4,0x0402);
		//	Clear even/odd mode (the other way, don't ask why)
		IO::send<uint16_t>(0x03C4,0x0604);
		//	Copy charmap
		uint8_t const blah[16] = {
			0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
			0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA
		};
		for (int i=0;i<256;++i) {
			MOSSC::memcpy(
				reinterpret_cast<void*>(0x000A0000 + i*32),
				//font_buffer + font_height*i,
				blah,
				font_height
			);
		}
		//	Restore VGA state to normal operation
		IO::send<uint16_t>(0x03C4,0x0302);
		IO::send<uint16_t>(0x03C4,0x0204);
		IO::send<uint16_t>(0x03CE,0x1005);
		IO::send<uint16_t>(0x03CE,0x0E06);
	#if 0
		//Unlock CRTC
		crtc.unlock_registers(); //TODO: necessary?
		//Load registers
		regs.read_all_registers();

		/* CPU writes only to map 2 */
		vga_wseq(vgabase, VGA_SEQ_PLANE_WRITE, 0x04);
		/* Sequential addressing */
		vga_wseq(vgabase, VGA_SEQ_MEMORY_MODE, 0x07);
		/* Clear synchronous reset */
		vga_wseq(vgabase, VGA_SEQ_RESET, 0x03);

		/* Now, the graphics controller, select map 2 */
		vga_wgfx(vgabase, VGA_GFX_PLANE_READ, 0x02);
		/* disable odd-even addressing */
		vga_wgfx(vgabase, VGA_GFX_MODE, 0x00);
		/* map start at A000:0000 */
		vga_wgfx(vgabase, VGA_GFX_MISC, 0x00);

		if ( font )
		{
			unsigned i, j;
			const uint8_t *data = font->data;
			uint8_t *map;

			map = ioremap(0xA0000 + font_slot*2*CHAR_MAP_SIZE, CHAR_MAP_SIZE);

			for ( i = j = 0; i < CHAR_MAP_SIZE; )
			{
				writeb(j < font->count * fontheight ? data[j++] : 0, map + i++);
				if ( !(j % fontheight) )
					while ( i & (FONT_HEIGHT_MAX - 1) )
						writeb(0, map + i++);
			}

			iounmap(map);
		}

		/* First, the sequencer, Synchronous reset */
		vga_wseq(vgabase, VGA_SEQ_RESET, 0x01);
		/* CPU writes to maps 0 and 1 */
		vga_wseq(vgabase, VGA_SEQ_PLANE_WRITE, 0x03);
		/* odd-even addressing */
		vga_wseq(vgabase, VGA_SEQ_MEMORY_MODE, 0x03);
		/* Character Map Select: The default font is kept in slot 0. */
		vga_wseq(vgabase, VGA_SEQ_CHARACTER_MAP,
				 font ? font_slot | (font_slot << 2) : 0x00);
		/* clear synchronous reset */
		vga_wseq(vgabase, VGA_SEQ_RESET, 0x03);

		/* Now, the graphics controller, select map 0 for CPU */
		vga_wgfx(vgabase, VGA_GFX_PLANE_READ, 0x00);
		/* enable even-odd addressing */
		vga_wgfx(vgabase, VGA_GFX_MODE, 0x10);
		/* map starts at b800:0 */
		vga_wgfx(vgabase, VGA_GFX_MISC, 0x0e);

		regs.write_all_registers();
		crtc.lock_registers();
	#endif
	#endif

	//Note need to reset mode; the height will change based on a font height change
	crtc.set_mode(crtc.mode);
}
void Interface::set_use_font(struct Font:: Character8x8 const* font) {
	_set_use_font(reinterpret_cast<uint8_t const*>(font),8);
}
void Interface::set_use_font(struct Font::Character8x16 const* font) {
	_set_use_font(reinterpret_cast<uint8_t const*>(font),16);
}

/*void Interface::dump_fields(void) const {
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
}*/


}}}