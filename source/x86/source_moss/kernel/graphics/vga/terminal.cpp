#include "terminal.h"

#include "../../../mossc/cstdio"
#include "../../../mossc/cstring"

#include "../../io/io.h"

#include "../font.h"


namespace MOSS { namespace Graphics { namespace VGA {


Terminal::Terminal(Device* device) :
	_device(device),
	_buffer(reinterpret_cast<uint16_t*>(0xB8000))
{
	_x = 0;
	_y = 0;

	//This seems to be the default
	color_text = Color::LIGHT_GREY;
	color_bg   = Color::BLACK;
	_color = (static_cast<int>(color_bg)<<4) | static_cast<int>(color_text);

	#if 1
		//interface.set_use_font(Graphics::Font::font8x8);
		//interface.dump_registers();
		//interface.crtc.set_mode(Graphics::VGA::CathodeRayTubeController::Mode::text128x80);
	#else
		interface.set_use_font(Graphics::Font::font8x16);
		interface.crtc.set_mode(Graphics::VGA::CathodeRayTubeController::Mode::text128x48);
	#endif

	clear();

	/*//char const*  str80 = "<-|(((Hello_world; how_are_you?__My_name_is_Ian;_good_to_finally_meet_you!)))|->";
	//char const*  str90 = "<-|((Hello_world; how_are_you?__My_name_is_Ian;_it's_good_to_finally_get_to_meet_you!))|->";
	//char const* str132 = "<-|(((Hello_world; how_are_you?__My_name_is_Ian!__Wow this is exciting.__I'm_so_happy.__It_is_good_to_finally_get_to_meet_you!)))|->";
	char const* str132 = "*1 5*  10*  15*  20*  25*  30*  35*  40*  45*  50*  55*  60*  65*  70*  75*  80*  85*  90*  95* 100* 105* 110* 115* 120* 125* 130* |";
	for (int j=0;j<1;++j) {
		set_color_text(Graphics::VGA::Terminal::COLOR_RED  ); kernel->write("%s",str132);
		set_color_text(Graphics::VGA::Terminal::COLOR_GREEN); kernel->write("%s",str132);
		set_color_text(Graphics::VGA::Terminal::COLOR_BLUE ); kernel->write("%s",str132);
	}
	set_color_text(Graphics::VGA::Terminal::COLOR_RED  );
	set_pos(0,0);
	for (int i=0;i<80;++i) {
		kernel->write("%d\n",i);
	}
	while (1);*/
}

void Terminal::clear(void) {
	for (size_t y=0; y<_device->rows; ++y) {
		for (size_t x=0; x<_device->cols; ++x) {
			write(' ', x,y);
		}
	}
}

void Terminal::scroll(size_t lines) {
	for (size_t i=0;i<lines;++i) {
		for (size_t y=0; y<_device->rows-1; ++y) {
			for (size_t x=0; x<_device->cols; ++x) {
				size_t index0 =  y   *_device->cols + x;
				size_t index1 = (y+1)*_device->cols + x;
				_buffer[index0] = _buffer[index1];
			}
		}
	}
}

void Terminal::fill_line(size_t line_index, char with) {
	for (size_t x=0; x<_device->cols; ++x) {
		write(with, x,line_index);
	}
}

void Terminal::next_line(void) {
	_x = 0;
	if (++_y == _device->rows) {
		scroll(1);
		--_y;

		Color temp_color_text = color_text;
		Color temp_color_bg   = color_bg;
		set_color(Color::LIGHT_GREY,Color::BLACK);

		fill_line(_device->rows-1,' ');

		set_color(temp_color_text,temp_color_bg);
	}
}

void Terminal::set_color_text(Color color_text) {
	if (this->color_text!=color_text) {
		_color &= 0x00F0u;
		_color |= static_cast<uint8_t>(color_text);

		this->color_text = color_text;
	}
}
void Terminal::set_color_background(Color color_bg) {
	if (this->color_bg!=color_bg) {
		_color &= 0x000Fu;
		_color |= static_cast<uint8_t>(color_bg)<<4;

		this->color_bg = color_bg;
	}
}
void Terminal::set_color(Color color_text, Color color_bg) {
	set_color_text(color_text);
	set_color_background(color_bg);
	//_color = (color_bg<<4) | color_text;
}

void Terminal::write(char c, size_t x,size_t y) {
	static_assert(sizeof(char)==1,"Implementation error!");

	size_t index = y*_device->cols + x;

	uint16_t c16 = c;
	uint16_t color16 = _color;

	_buffer[index] = c16 | (color16 << 8);
}
void Terminal::write(char c) {
	#ifdef MOSS_DEBUG
	//Bochs debug: TODO: it doesn't do anything.
	//IO::send(0xE9,static_cast<uint8_t>(c));
	#endif

	if (c=='\r') {
		_x = 0;
	} else if (c=='\n') {
		next_line();
	} else {
		write(c, _x,_y);

		if (++_x == _device->cols) {
			//next_line();
		}
	}
}
void Terminal::write(char const* format, va_list args) {
	#ifdef MOSS_DEBUG
	int len = MOSSC::strlen(format);
	assert_term(len<1024,"Format string too long (%d)!  Not implemented!",len);
	#endif
	char buffer[1024]; //TODO: use vsnprintf or something!

	MOSSC::vsprintf(buffer,format,args);
	size_t i = 0;
	LOOP:
		char c = buffer[i];
		if (c!='\0') {
			write(c);
			++i;
			goto LOOP;
		}
}

void Terminal::write_test_pattern_res(void) {
	for (size_t i=0;i<_device->cols;++i) {
		set_pos_x(i);
		if (i>=9) {
			if (i>=99) {
				set_pos_y( (i+2) % 4 );
			} else {
				set_pos_y( (i+1) % 3 );
			}
		} else {
			set_pos_y(0);
		}
		write("%d",i+1);
	}
	for (size_t j=1;j<_device->rows;++j) {
		set_pos(0,j);
		write("%d",j+1);
	}
	set_pos(3,4); write("This window is %d*%d",static_cast<int>(_device->cols),static_cast<int>(_device->rows));
	//set_pos(3,5); write("The backing store is %d*%d",static_cast<int>(console.real.w),static_cast<int>(console.real.h));
	//set_pos(3,6); write("Hit ENTER to continue . . .");
}
void Terminal::write_test_pattern_lin(void) {
	Color colors[12] = {
		Color::RED, Color::LIGHT_RED,
		Color::BROWN, Color::YELLOW,
		Color::GREEN, Color::LIGHT_GREEN,
		Color::CYAN, Color::LIGHT_CYAN,
		Color::BLUE, Color::LIGHT_BLUE,
		Color::PURPLE, Color::MAGENTA
	};
	char const* chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	size_t index = 0;
	for (size_t i=0;i<12;++i) {
		char const* temp = chars;
		while (*temp!='\0') {
			uint16_t color16 = static_cast<uint16_t>(colors[i]);
			_buffer[index++] = *(temp++) | (color16 << 8);
		}
	}
}


}}}
