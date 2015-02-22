#pragma once

#include "../stdinc.h"
#include "../lib/String.h"
#include "../lib/Mutex.h"

/*
My file system:

Each file is described as a series of contiguous blocks of size 512B.  Each file has at least one sector dedicated to
information about the file.  These information sectors are effectively a singly linked list.

Each information sector starts with a positive long pointing to the next information sector, (NULL if there isn't one).

The sector continues with the name, (or any previous data from the last information sector).

Then, there's a pointer to the first data sector (represented as a long), and the length of the contiguous block it
corresponds to (how many sectors long it is).  This start/length pair may be repeated if the file has more than one
contiguous block.  If the information sector runs out of space with such pairs, the next information sector (pointed to
by the first long in the information sector) will continue the information.

The file system constantly rearranges itself to maximize the contiguity of the blocks.
*/

#define BASE_HD_MEM ((char*)(0x00100000))

class Sector {
	public:
		char data[512];
};

class INode {
	public:
		String name;
		DynamicArray<long> sectors;
		INode* next;
	public:
		INode(const Sector* new_sector, HardDisk* hard_disk) {
			char* address_next = (char*)(  ((long*)(new_sector))[0]  );
			if (address_next==NULL) {
				next = NULL;
			} else {
				next = new INode(hard_disk->load(address_next),hard_disk);
			}

			delete new_sector;
		}
		INode(String name) : name(name) {}
		~INode(void) {
			if (next!=NULL) delete next;
		}
};
class File {
	public:
		INode* first_descriptor;
	private:
		Mutex mutex;
	public:
		File(void) {
		}
		~File(void) {
		}

		String read(void) {
			mutex.acquire();
			String result();
			mutex.release();
		}
		void write(const char* data) {
			mutex.acquire();
			mutex.release();
		}
};

class HardDisk {
	private:
		Mutex mutex;
		INode root;
	public:
		HardDisk(void) : root(load_new(BASE_HD_MEM)) {}
		~HardDisk(void) {}

		void write(File* file) {
			mutex.acquire();
			//file
			mutex.release();
		}

		Sector* load_new(char* address) const {
			Sector* data = new Sector();
			return data;
		}

		File* read(const String& file_path) {
			if (file_path.contains("/")) {
				DynamicArray<DynamicArray<char>> split = file_path.split(String("/"));
				split[0]
			}

			INode* first = new





			File* result;
			mutex.acquire();
			result = new File(file_path);
			mutex.release();
		}
};