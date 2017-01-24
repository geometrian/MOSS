#include "components.h"

#include "../../../mossc/string/copying.h"

#include "../../io/io.h"
#include "../../kernel.h"

#include "../font.h"

#include "terminal.h"


namespace MOSS { namespace Graphics { namespace VGA {


namespace Regs {


void RegisterBase::print(void) const {
	kernel->write("Register [");
	for (int i=0;i<4;++i) kernel->write("%c ",is_set(7-i)?'#':'.');
	for (int i=4;i<8;++i) kernel->write(" %c",is_set(7-i)?'#':'.');
	kernel->write("] (0x%p",this);
	#ifdef MOSS_DEBUG
	kernel->write(" \"%s Register\"",name);
	#endif
	kernel->write(")\n");
	/*for (int i=0;i<8;++i) {
		kernel->write("  bit %d (0x%p) parent reg 0x%p\n",i,bits+i,bits[i].reg);
	}*/
}


uint8_t ExternalRegisterBase::read_raw(void) const /*override*/ {
	if (!device->is_monochrome()) {
		return IO::recv<uint8_t>(ports_col.read);
	} else {
		return IO::recv<uint8_t>(ports_mono.read);
	}
}
void ExternalRegisterBase::write_raw(uint8_t value) /*override*/ {
	assert_term(writable,"Implementation error!  Cannot write read-only register!");
	if (!device->is_monochrome()) {
		IO::send<uint8_t>(ports_col.write, value);
	} else {
		IO::send<uint8_t>(ports_mono.write, value);
	}
}


uint8_t IndexedRegisterBase::read_raw(void) const /*override*/ {
	uint8_t io = device->_regs.misc_output.read().InputOutputAddressSelect;
	uint16_t port_addr = ports_addr.ports[io];
	uint16_t port_data = ports_data.ports[io];

	//1. Input the value of the Address Register and save it for step 6
	uint8_t old_addr_reg_value = IO::recv<uint8_t>(port_addr);
	//2. Output the index of the desired Data Register to the Address Register.
	IO::send<uint8_t>(port_addr, index);

	//3. Read the value of the Data Register and save it for later restoration upon termination, if needed.
	uint8_t value = IO::recv<uint8_t>(port_data);

	//4. If writing, modify the value read in step 3, making sure to mask off bits not being modified.
	//5. If writing, write the new value from step 4 to the Data register.

	//6. Write the value of Address register saved in step 1 to the Address Register.
	IO::send<uint8_t>(port_addr,old_addr_reg_value);

	return value;
}
void IndexedRegisterBase::write_raw(uint8_t value) /*override*/ {
	uint8_t io = device->_regs.misc_output.read().InputOutputAddressSelect;
	uint16_t port_addr = ports_addr.ports[io];
	uint16_t port_data = ports_data.ports[io];

	//1. Input the value of the Address Register and save it for step 6
	uint8_t old_addr_reg_value = IO::recv<uint8_t>(port_addr);
	//2. Output the index of the desired Data Register to the Address Register.
	IO::send<uint8_t>(port_addr, index);

	//3. Read the value of the Data Register and save it for later restoration upon termination, if needed.
	//uint8_t old_data_reg_value = IO::recv<uint8_t>(_port_data);

	//4. If writing, modify the value read in step 3, making sure to mask off bits not being modified.
	//5. If writing, write the new value from step 4 to the Data register.
	IO::send<uint8_t>(port_data,value);

	//6. Write the value of Address register saved in step 1 to the Address Register.
	IO::send<uint8_t>(port_addr,old_addr_reg_value);
}


SequencerRegisterBase::SequencerRegisterBase(Sequencer* sequencer, DEBUG_ONLY(char const* name COMMA) uint16_t index) :
	IndexedRegisterBase(sequencer->device,DEBUG_ONLY(name COMMA){0x03C4,0x03C4},{0x03C5,0x03C5},index), sequencer(sequencer)
{}


GraphicsRegisterBase::GraphicsRegisterBase(GraphicsController* graphics_controller, DEBUG_ONLY(char const* name COMMA) uint16_t index) :
	IndexedRegisterBase(graphics_controller->device,DEBUG_ONLY(name COMMA){0x03CE,0x03CE},{0x03CF,0x03CF},index), graphics_controller(graphics_controller)
{}


CRTC_RegisterBase::CRTC_RegisterBase(CRTC* crtc, DEBUG_ONLY(char const* name COMMA) uint16_t index) :
	IndexedRegisterBase(crtc->device,DEBUG_ONLY(name COMMA){0x03B4,0x03D4},{0x03B5,0x03D5},index), crtc(crtc)
{}


struct SequencerCharacterMapSelectRegister::Fields SequencerCharacterMapSelectRegister::read(void) const {
	uint8_t temp=read_raw(); union { struct Fields fields; uint8_t packed; } result; result.packed=0;
	result.fields.SelectCharacterSetA = ((temp>>2)&0b00000011) | ((temp>>3)&0b00000100);
	result.fields.SelectCharacterSetB = ( temp    &0b00000011) | ((temp>>2)&0b00000100);
	result.fields._unknown            = (temp>>6) & 0b00000011;
	return result.fields;
}
void SequencerCharacterMapSelectRegister::write(struct Fields value) {
	uint8_t temp = 0;
	temp |= ((value.SelectCharacterSetA&0b00000011)<<2) | ((value.SelectCharacterSetA&0b00000100)<<3);
	temp |= ( value.SelectCharacterSetB&0b00000011    ) | ((value.SelectCharacterSetA&0b00000100)<<2);
	temp |= value._unknown << 6;
	write_raw(temp);
}


}


GraphicsController::RegsGC::RegsGC(GraphicsController* graphics_controller) :
	set_reset(graphics_controller),
	set_reset_enable(graphics_controller),
	color_compare(graphics_controller),
	data_rotate(graphics_controller),
	read_map_select(graphics_controller),
	graphics_mode(graphics_controller),
	misc(graphics_controller),
	color_dont_care(graphics_controller),
	bitmask(graphics_controller)
{}


void GraphicsController::set_planes_read(uint8_t bitmask) {
	assert_term(bitmask<=0b00001111,"Invalid bitmask!");
	_set_ReadMapSelect(bitmask);
}


Sequencer::RegsSeq::RegsSeq(Sequencer* sequencer) :
	reset(sequencer),
	clocking_mode(sequencer),
	map_mask(sequencer),
	char_map_select(sequencer),
	memory_mode(sequencer)
{}


void Sequencer::set_planes_write(uint8_t bitmask) {
	assert_term(bitmask<=0b00001111,"Invalid bitmask!");
	_set_MemoryPlaneWriteEnable(bitmask);
}

void Sequencer::set_dot_mode(DotMode dot_mode) {
	if (this->dot_mode!=dot_mode) {
		switch (dot_mode) {
			case DotMode::DOTS_8:
				_set_DotMode_9_8(1);
				break;
			case DotMode::DOTS_9:
				_set_DotMode_9_8(0);
				break;
			default: assert_term(false,"Implementation error!");
		}
	}
}


CRTC::RegsCRTC::RegsCRTC(CRTC* crtc) :
	htotal(crtc),
	end_hdisplay(crtc),
	start_hblanking(crtc),
	end_hblanking(crtc),
	start_hretrace(crtc),
	end_hretrace(crtc),
	vtotal(crtc),
	overflow(crtc),
	preset_row_scan(crtc),
	max_scanline(crtc),
	start_cursor(crtc),
	end_cursor(crtc),
	start_addr_high(crtc),
	start_addr_low(crtc),
	cursor_location_high(crtc),
	cursor_location_low(crtc),
	start_vretrace(crtc),
	end_vretrace(crtc),
	end_vdisplay(crtc),
	offset(crtc),
	underline_location(crtc),
	start_vblanking(crtc),
	end_vblanking(crtc),
	mode_control(crtc),
	line_compare(crtc)
{}


void CRTC::set_timing(struct Timing const& timing) {
	//Figure out how many dots the clock frequency implies each scanline is
	size_t dots;
	switch (timing.clock_index) {
		case 0: dots=800; break; //25 MHz / 31.25KHz
		case 1: dots=896; break; //28 MHz / 31.25KHz
		case 2: //Fallthrough
		case 3:
			assert_term(false,"Hardware-specific clocks not implemented!");
		default:
			assert_term(false,"Implementation error!"); //Should have been caught earlier
	}

	//Figure out how many characters per scanline that corresponds to
	size_t char_width;
	switch (device->sequencer.dot_mode) {
		case Sequencer::DotMode::DOTS_8: char_width=8; break;
		case Sequencer::DotMode::DOTS_9: char_width=9; break;
		default: assert_term(false,"Implementation error!");
	}
	size_t max_chars = dots / char_width;
	assert_term(max_chars>5,"Scanline must be more than 5 characters long!");

	//Figure out how many scanlines we have
	assert_term(timing.vert.end_retrace>=2,"Must have at least one scanline!");
	size_t max_scanlines = timing.vert.end_retrace - 1;

	//TODO: checks!

	/*//0: end skew display
	//1: end display
	//2: start retrace
	//3: end retrace
	size_t scanline_times[];

	size_t clock = 0;

	assert_term(timing.horiz.skew_display<=0b11,"Skew too large!");
	clock += timing.horiz.skew_display;

	assert_term(timing.horiz.width>0,"Width must be positive!");
	clock += timing.horiz.width;

	assert_term(timing.horiz.skew_retrace+timing.horiz.start_retrace<max_chars,"Scanline must contain retrace!");

	assert_term(
		timing.horiz.end_blank>=timing.horiz.start_blank &&
		timing.horiz.end_blank-timing.horiz.start_blank<=0b00111111,
		"Horizontal blanking ending must be larger than horizontal blanking start, but not more than 2^6-1 away!"
	);
	assert_term(
		timing.horiz.end_retrace>=timing.horiz.start_retrace &&
		timing.horiz.end_retrace-timing.horiz.start_retrace<=0b00011111,
		"Horizontal retrace ending must be larger than horizontal blanking start, but not more than 2^5-1 away!"
	);


	size_t clock = 0;
	
	

			struct Horizontal final {
				//size_t skew_display;
				size_t width;
				size_t start_blank;
				//size_t skew_retrace;
				size_t start_retrace;
				size_t end_retrace;
				size_t end_blank;
			} horiz;
			struct Vertical final {
				size_t height;
				size_t start_blank;
				size_t start_retrace;
				size_t end_retrace;
				size_t end_blank;
			} vert;*/

	//If we've gotten this far, we have all the information we need and it's all validated.  Let's set the mode already.

	//Set the clock frequency
	device->set_clock(timing.clock_index);

	//Set scanline timings
	_set_HorizontalTotal(max_chars-5);
	//_set_DisplayEnableSkew(timing.horiz.skew_display);
	_set_EndHorizontalDisplay(timing.horiz.width-1);
	_set_StartHorizontalBlanking(timing.horiz.start_blank);
	_set_EndHorizontalBlanking(timing.horiz.end_blank & 0b00111111);
	_set_StartHorizontalRetrace(timing.horiz.start_retrace);
	_set_EndHorizontalRetrace(timing.horiz.end_retrace & 0b00011111);

	//Set vertical timings
	_set_VerticalTotal(max_scanlines-1);
	_set_EndVerticalDisplay(timing.vert.height);
	_set_StartVerticalBlanking(timing.vert.start_blank);
	_set_EndVerticalBlanking(timing.vert.end_blank & 0b01111111);
	_set_StartVerticalRetrace(timing.vert.start_retrace);
	_set_EndVerticalRetrace(timing.vert.end_retrace & 0b00001111);

	//Set stride between scanlines:
	//	Width/(MemoryAddressSize*2).  Default is 20=80/(2*sizeof(uint16_t)).
	_set_Offset( timing.horiz.width / (2*sizeof(uint8_t)) );

	//Set line compare to largest value so that it doesn't try to split the screen
	_set_LineCompare(0x03FF);
}

void CRTC::print_timing(size_t x, size_t y) const {
	size_t dots; char const* clk_str;
	switch (device->get_clock()) {
		case 0: dots=800; clk_str="25"; break; //25 MHz / 31.25KHz
		case 1: dots=896; clk_str="28"; break; //28 MHz / 31.25KHz
		case 2: //Fallthrough
		case 3:
			assert_term(false,"Hardware-specific clocks not implemented!");
		default:
			assert_term(false,"Implementation error!");
	}

	size_t char_width;
	switch (device->sequencer.dot_mode) {
		case Sequencer::DotMode::DOTS_8: char_width=8; break;
		case Sequencer::DotMode::DOTS_9: char_width=9; break;
		default: assert_term(false,"Implementation error!");
	}
	size_t max_chars = dots / char_width;

	kernel->terminal->set_pos(x,y);
	kernel->write("%s MHz clock -> %d dots >= %dx%d chars per scanline",clk_str,static_cast<int>(dots),static_cast<int>(char_width),static_cast<int>(max_chars));
	kernel->terminal->set_pos(x,y+1); kernel->write("Scanline stride: %d bytes",static_cast<int>(_get_Offset()));
	kernel->terminal->set_pos(x,y+2); kernel->write("                Horizontal:            Vertical:");
	kernel->terminal->set_pos(x,y+3); kernel->write("Total:                 %d                 %d",static_cast<int>(_get_HorizontalTotal())+5,static_cast<int>(_get_VerticalTotal())+1);
	kernel->terminal->set_pos(x,y+4); kernel->write("End Display:           %d                 %d",static_cast<int>(_get_EndHorizontalDisplay())+1,static_cast<int>(_get_EndVerticalDisplay()));
	kernel->terminal->set_pos(x,y+5); kernel->write("Start Blanking:        %d                 %d",static_cast<int>(_get_StartHorizontalBlanking()),static_cast<int>(_get_StartVerticalBlanking()));
	kernel->terminal->set_pos(x,y+6); kernel->write("End Blanking:          %d                 %d",static_cast<int>(_get_EndHorizontalBlanking()),static_cast<int>(_get_EndVerticalBlanking()));
	kernel->terminal->set_pos(x,y+7); kernel->write("Start Retrace:         %d                 %d",static_cast<int>(_get_StartHorizontalRetrace()),static_cast<int>(_get_StartVerticalRetrace()));
	kernel->terminal->set_pos(x,y+8); kernel->write("End Retrace:           %d                 %d",static_cast<int>(_get_EndHorizontalRetrace()),static_cast<int>(_get_EndVerticalRetrace()));
}


Device::Device(void) :
	_regs(this),
	graphics_controller(this),
	memory(this),
	sequencer(this),
	//color_logic(this),
	crtc(this)
{
	//TODO: should probably check
	mode = Mode::Text80x25;

	cols = 80;
	rows = 25;

	font_height = 16;
}

void Device::_prepare_change(void) {
	//Unlock CRTC, disable display and sync signals
	crtc.set_registers_locked(false);
	sequencer.set_display_enabled(false);
	crtc.set_sync_signals_enabled(false);
}
void Device::_finish_change(void) {
	//Enable display and sync signals, lock CRTC (technically optional)
	crtc.set_sync_signals_enabled(true);
	sequencer.set_display_enabled(true);
	crtc.set_registers_locked(true);
}

void Device::set_mode(Mode mode) {
	//Note: we do mode conversions unconditionally; things like font changes can cause the numbers to change.
	//if (this->mode!=mode); else return;

	switch (mode) {
		case Mode:: Text80x25: cols= 80; rows=25; break;
		case Mode:: Text80x30: cols= 80; rows=30; break;
		case Mode:: Text80x50: cols= 80; rows=50; break;
		case Mode:: Text80x60: cols= 80; rows=60; break;
		case Mode::Text128x48: cols=128; rows=48; break;
		case Mode::Text128x80: cols=128; rows=80; break;
		case Mode::Text132x25: cols=132; rows=25; break;
		case Mode::Text132x43: cols=132; rows=43; break;
		case Mode::Text132x50: cols=132; rows=50; break;
		case Mode::Text132x60: cols=132; rows=60; break;
		default: assert_term(false,"Implementation error!");
	}
	this->mode = mode;

	_prepare_change();

	/*
		Refer to this diagram: http://wiki.osdev.org/VGA_Hardware#The_CRT_Controller
		Refer to this explanation: http://www.osdever.net/FreeVGA/vga/vgacrtc.htm

		Clock Frequencies:
			0: 25 MHz clock
			1: 28 MHz clock
			2: Hardware-specific; may not exist
			3: Hardware-specific; may not exist

		Formulas:
			Horizontal Refresh Rate = Clock Frequency (in Hz) / total pixels horizontally = 31,250Hz
			Vertical Refresh Rate   = Horizontal Refresh Rate / total scan lines = 31,250Hz / total scan lines

		To draw a pixel ("dot"), you see that with a 25 MHz clock, you want 800 dots per scanline and for a
		28MHz clock, you want 896 dots per scanline.  These "dots" are handled internally in groups of 8 or 9,
		intended to correspond to the width of a character, and the hardware keeps a "character clock" instead
		of a "pixel clock".

		To render a scanline (typically):
			 1: At the beginning of the scanline, the character clock is set to 0 and begins incrementing.
			 2: It emits the overscan color until the display skew is reached.  The skew is typically zero, so this step is usually skipped.
			 3: It emits the scanline's data
			 4: The overscan color is emitted until the start of horizontal blanking.
			 5: No color is emitted while blanking is in effect
			 6: A separate horizontal skew (typically zero) can delay the next step
			 7: At the start of horizontal retrace, the electron beam begins slewing back to the left of the screen.
			 8: At the end of horizontal retrace (determined by the lower 5 bits of the end-retrace field matching those of the character clock), the beam should be done.
			 9: Horizontal blanking can be turned off
			10: Overscan is emitted until the scanline length is reached
		Note: Having blanking is optional (and can be disabled by making the start value larger than the scanline length).

		To render all scanlines (typically):
			1: At the first scanline, the scanline counter is set to 0 and begins incrementing after each scanline.
			2: At the end of the vertical total of scanlines, scanlines consisting of overscan color are emitted
			3: At vertical blanking start, no color is emitted in each scanline.
			4: At the beginning of vertical retrace start, the electron beam begins slewing back up to the top of the screen.
			5: At the end of vertical retrace (determined by the lower 4 bits of the end-retrace field matching those of the scanline clock), the beam should be done.
			6: At the end of vertical blanking, goto 1.
		Note: Apparently, having no vertical blanking is impossible, since the end of it triggers the reset of the scanline counter.
	*/

	crtc.set_character_height(font_height);
	sequencer.set_dot_mode(Sequencer::DotMode::DOTS_8);

	/*struct Timing final {
		size_t clock_index;
		struct Horizontal final {
			size_t skew_display;
			size_t width;
			size_t start_blank;
			size_t skew_retrace;
			size_t start_retrace;
			size_t end_retrace;
			size_t end_blank;
		} horiz;
		struct Vertical final {
			size_t height;
			size_t start_blank;
			size_t start_retrace;
			size_t end_retrace; //First scanline after retrace completes
			size_t end_blank;
		} vert;
	};*/
	crtc.set_timing({
		0,
		{
			0,
			cols,
			cols,
			0,
			cols+7,
			cols+7,
			cols+7
		},{
			font_height*rows,
			font_height*rows,
			font_height*rows,
			font_height*rows+5,
			font_height*rows+5
		}
	});


	/*//	External:
	//		Clock select:
	//			00 is 25MHz clock (for 320* or 640* modes)
	//			01 is 28MHz clock (for 360* or 720* modes)
	_set_ClockSelect(1); //Note: always 01 because only 720* modes are supported
	//		Clocking mode (note: the 8-dot modes aren't technically supported by VGA according to that table, but
	//			I can't imagine why it wouldn't work).  Note: if set to 9-bit mode, should set the line graphics
	//			enable bit, which copies in the last column of the line graphics characters into the 9th column
	//			where other characters would have a gap.
	sequencer.set_dot_mode(Sequencer::DotMode::DOTS_8);

	//	Internal:
	//		On a VGA monitor, the horizontal refresh rate should equal 31,250Hz.
	
	//		So the width should be a virtual 896 pixels (99 5/9 characters).

	//_interface->fields.DisplayEnableSkew = 0b00;

	crtc.set_character_height(font_height);

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
	//	TODO: this!

	//For text mode, load fonts.  Note that this may need to alter GC
	//	settings, so be sure to restore those after that.*/

	_finish_change();
}

void Device::_set_use_font(uint8_t const* font_buffer, size_t font_height) {
	//http://compbio.cs.toronto.edu/repos/snowflock/xen-3.0.3/xen/drivers/video/vga.c
	assert_term(font_height==8||font_height==16,"Invalid font height %d!  Must be 8 or 16.",static_cast<int>(font_height));
	this->font_height = font_height;

	_prepare_change();

	//Turn off even/odd addressing
	//	The read mode disable shouldn't be necessary AFAICT, but e.g. QEMU seems to require it.
	graphics_controller.set_host_oddeven_read_memory(false);
	sequencer.set_host_oddeven_write_memory(false);

	//Select font plane
	//graphics_controller.set_planes_read(0b0100);
	sequencer.set_planes_write(0b0100);

	//Set offset within data region to be 0 for both fonts A and B.
	sequencer.set_fonts_data_region_offsets(0,0);

	//Write font
	for (size_t i=0;i<256;++i) {
		MOSSC::memcpy(
			//0xB0000, 0xB8000, 0xA0000
			//Note: font slots other than 0 might not be respected on AMD cards
			reinterpret_cast<void*>(0xB800*16 /*+ 16384*font_slot*/ + i*32),
			//reinterpret_cast<void*>(0x000A0000 + i*32),
			//reinterpret_cast<void*>(0xA0000 + i*32),
			font_buffer + font_height*i,
			font_height
		);
	}

	//Select normal plane
	//graphics_controller.set_planes_read(0b0001);
	sequencer.set_planes_write(0b0011);

	//Turn even/odd addressing back on
	//fields.OEHostMem = 0b101;
	//sequencer.set_chain4(true);
	sequencer.set_host_oddeven_write_memory(true);
	graphics_controller.set_host_oddeven_read_memory(true);

	_finish_change();

	//Note need to reset mode; the height will change based on a font height change
	set_mode(mode);

	#if 1
		#if 0
		//Unlock CRTC
		crtc.unlock_registers(); //TODO: necessary?
		//Load registers
		regs.read_all_registers();

		//Turn off even/odd addressing
		//fields.OEHostMem.print();
		fields.OEHostMem=0b110; fields.OEHostMem.save_regs();
		//fields.OEHostMem.load_regs();
		//fields.OEHostMem.print();
		//assert_term(fields.OEHostMem.get_as<uint8_t>()==0b110,"Fail %d",fields.OEHostMem.get_as<uint8_t>());

		//Select font plane
		_gc.set_plane(2);

		//Write font
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
		fields.OEHostMem = 0b101;
		/*assert_warn(fields.OEHostMem.get_as<uint8_t>()==0b101,"Fail %d",fields.OEHostMem.get_as<uint8_t>());

		regs.gc_misc_reg.print();

		regs.gc_misc_reg.write();
		regs.gc_misc_reg.read();

		regs.gc_misc_reg.print();

		assert_term(fields.OEHostMem.get_as<uint8_t>()==0b101,"Fail %d",fields.OEHostMem.get_as<uint8_t>());*/


		//fields.OEHostMem.save_regs();
		//assert_warn(fields.OEHostMem.get_as<uint8_t>()==0b101,"Fail %d",fields.OEHostMem.get_as<uint8_t>());
		//fields.OEHostMem.load_regs(); assert_warn(fields.OEHostMem.get_as<uint8_t>()==0b101,"Fail %d",fields.OEHostMem.get_as<uint8_t>());

		regs.write_all_registers();
		//fields.OEHostMem.load_regs(); assert_warn(fields.OEHostMem.get_as<uint8_t>()==0b101,"Fail %d",fields.OEHostMem.get_as<uint8_t>());
		crtc.lock_registers();
		//fields.OEHostMem.load_regs(); assert_warn(fields.OEHostMem.get_as<uint8_t>()==0b101,"Fail %d",fields.OEHostMem.get_as<uint8_t>());
		#endif
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

	/*fields.OEHostMem.load_regs(); assert_warn(fields.OEHostMem.get_as<uint8_t>()==0b101,"Fail %d",fields.OEHostMem.get_as<uint8_t>());

	//Note need to reset mode; the height will change based on a font height change
	crtc.set_mode(crtc.mode);

	fields.OEHostMem.load_regs(); assert_warn(fields.OEHostMem.get_as<uint8_t>()==0b101,"Fail %d",fields.OEHostMem.get_as<uint8_t>());*/
}
void Device::set_use_font(Font:: Character8x8 const* font) {
	_set_use_font(reinterpret_cast<uint8_t const*>(font),8);
}
void Device::set_use_font(Font::Character8x16 const* font) {
	_set_use_font(reinterpret_cast<uint8_t const*>(font),16);
}


#if 0
void Interface::dump_registers(void) {
	regs.read_all_registers();
	#define DUMP_REG(INDENT,REG) for (int i=0;i<INDENT;++i) kernel->write("  "); regs.REG##_reg.print();

	kernel->write("External Registers:\n");
	DUMP_REG(1,miscellaneous_output)
	DUMP_REG(1,input_status0)
	DUMP_REG(1,input_status1)

	kernel->write("Internal Registers:\n");
	kernel->write("  Sequencer:\n");
	DUMP_REG(2,sequencer_clocking_mode)
	DUMP_REG(2,sequencer_map_mask)
	DUMP_REG(2,sequencer_memory_mode)
	kernel->write("  Graphics Controller:\n");
	DUMP_REG(2,gc_read_map_select)
	DUMP_REG(2,gc_mode)
	DUMP_REG(2,gc_misc)
	kernel->write("  Cathode Ray Tube Controller:\n");
	DUMP_REG(2,crtc_horizontal_total)
	DUMP_REG(2,crtc_end_horizontal_display)
	DUMP_REG(2,crtc_start_horizontal_blanking)
	DUMP_REG(2,crtc_end_horizontal_blanking)
	DUMP_REG(2,crtc_start_horizontal_retrace)
	DUMP_REG(2,crtc_end_horizontal_retrace)
	DUMP_REG(2,crtc_vertical_total)
	DUMP_REG(2,crtc_overflow)
	DUMP_REG(2,crtc_maximum_scan_line)
	DUMP_REG(2,crtc_vertical_retrace_start)
	DUMP_REG(2,crtc_vertical_retrace_end)
	DUMP_REG(2,crtc_vertical_display_end)
	DUMP_REG(2,crtc_offset)
	DUMP_REG(2,crtc_start_vertical_blanking)
	DUMP_REG(2,crtc_end_vertical_blanking)
	DUMP_REG(2,crtc_line_compare)

	#undef DUMP_REG
}
void Interface::dump_fields(void) {
	/*kernel->write("Miscellaneous Output Register:      %X\n\n",IO::recv<uint8_t>(MOSS_VGA_MISCOUTR));

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

	kernel->write("Line Compare Register:              %X\n",  read_internal_register_type1(MOSS_VGA_ADDR_CRTC,MOSS_VGA_DATA_CRTC, 0x18));*/
	assert_term(false,"Not implemented!");
}
#endif


}}}