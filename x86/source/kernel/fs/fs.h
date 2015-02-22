#pragma once
#include "../../includes.h"

#include "../../mosst/string.h"


namespace MOSS { namespace FS {


class File {
	private:
		MOSST::String path;
		MOSST::Vector<uint8_t> data;

	public:
		File(const char* path) {
			this->path = path;
		}
		~File(void) {
		}
};

class InterfaceFileSystemBase {
	public:
		InterfaceFileSystemBase(void) {
		}
		~InterfaceFileSystemBase(void) {
		}

		//TODO:
		//virtual File* read_new(const char* path) = 0;
		//virtual void write(const File* file) = 0;
};


}}