#include "manager.hpp"

#include "../../disk/filesystems/filesystem.hpp"
#include "../../input/mouse.hpp"
#include "../../kernel.hpp"

#include "../vesa/framebuffer.hpp"
#include "../color.hpp"

#include "frame.hpp"
#include "mouse.hpp"


namespace MOSS { namespace Graphics { namespace GUI {


Manager::_Image::_Image(int w,int h) : w(w), h(h) {
	data = new uint8_t[h*w*3];
}
Manager::_Image::~_Image(void) {
	delete [] data;
}


Manager::Manager(void) {
	_mouse = new Mouse();
	_bg = nullptr;
}
Manager::~Manager(void) {
	delete _mouse;
	if (_bg!=nullptr) delete _bg;
}

/*static void _parse_int(uint8_t const** data, int* result) {
	char c = (*data)[0];
	if (
}*/
void Manager::load_bg(char const* path) {
	if (_bg!=nullptr) delete _bg;

	Disk::FileSystem::ObjectFileBase* file = kernel->filesystem->open(path);
	//kernel->write("  Got file!  Loading data\n");

	MOSST::Vector<uint8_t>* data_vec = file->get_new_data();
	//kernel->write("  Got data!\n");

	uint8_t const* data = data_vec->get_backing();

	assert_term(data[0]=='P'&&data[1]=='6',"Only NetPBM type 6 supported (got %c%c)!",data[0],data[1]);
	data += 3; //Skip "P6\n"
	if (*data=='#') while (*(data++)!='\n'); //Skip comment
	//kernel->write("  Got P6 and comment!\n");

	//Parse width, height
	int w=0,h=0;
	while (*data!= ' ') { w*=10; w+=*data-'0'; ++data; } ++data;
	while (*data!='\n') { h*=10; h+=*data-'0'; ++data; } ++data;
	//kernel->write("  %d*%d\n",w,h);

	_bg = new _Image(w,h);

	//Skip number of possible values per channel
	while (*(data++)!='\n');
	for (int i=0;i<h*w*3;++i) {
		_bg->data[i] = data[i];
	}

	delete data_vec;
}

void Manager::update(void) {
	for (auto iter=_frames.cbegin(); iter!=_frames.cend(); ++iter) {
		if (!((*iter)->alive)) {
			delete *iter;
			~iter;
		}
	}
}

void Manager::handle_mouse(Input::Mouse::   EventMouseMove const& event) {
	_mouse->x = event.x;
	_mouse->y = event.y;

	for (auto iter=_frames.cbegin(); iter!=_frames.cend(); ++iter) {
		if ((*iter)->handle_mouse(event)) break;
	}
}
void Manager::handle_mouse(Input::Mouse::  EventMouseClick const& event) {
	_mouse->buttons[event.index] =  true;

	for (auto iter=_frames.cbegin(); iter!=_frames.cend(); ++iter) {
		if ((*iter)->handle_mouse(event)) break;
	}
}
void Manager::handle_mouse(Input::Mouse::EventMouseUnclick const& event) {
	_mouse->buttons[event.index] = false;

	for (auto iter=_frames.cbegin(); iter!=_frames.cend(); ++iter) {
		if ((*iter)->handle_mouse(event)) break;
	}
}

void Manager::add_frame(MOSST::String const& title, int x,int y,int w,int h) {
	Frame* frame = new Frame(nullptr, x,y,w,h);
	frame->set_title(title);

	_frames.insert_back(frame);
}

void Manager::draw(VESA::Framebuffer* framebuffer) const {
	framebuffer->draw_fill(Color(0,0,0));

	if (_bg!=nullptr) {
		for (int j=0;j<_bg->h;++j) {
			for (int i=0;i<_bg->w;++i) {
				uint8_t* pixel = _bg->data + 3*(j*_bg->w + i);
				Color color(pixel[0],pixel[1],pixel[2]);
				framebuffer->set_pixel(i,_bg->h-j-1,color);
			}
		}
	}

	for (auto iter=_frames.cbegin(); iter!=_frames.cend(); ++iter) {
		(*iter)->draw(framebuffer);
	}

	_mouse->draw(framebuffer);
}


}}}
