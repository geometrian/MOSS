#pragma once


namespace MOSS { namespace Boot {


class multiboot_memory_map_t final {
	public:
		unsigned int size;

		union {
			struct { unsigned int base_addr_low,base_addr_high; };
			unsigned long long int base_addr;
		};

		union {
			struct { unsigned int length_low,length_high; };
			unsigned long long int length;
		};

		unsigned int type;
};


}}
