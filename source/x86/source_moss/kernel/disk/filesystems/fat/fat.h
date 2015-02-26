#pragma once

#include "../../../../includes.h"

#include "../filesystem.h"


namespace MOSS { namespace Disk { namespace FileSystem {


//See http://www.pjrc.com/tech/8051/ide/fat32.html

class FileSystemFAT;

/*class ObjectDirectoryFAT final {
	public:
		ObjectDirectoryFAT(void) {}
		~ObjectDirectoryFAT(void) {}
};*/

class DirectoryRecord final {
	public:
		uint8_t short_filename[11]; //(Microsoft: "DIR_Name")

		class AttribByte final {
			public:
				bool is_read_only             : 1;
				bool is_hidden                : 1;
				bool is_system                : 1;
				bool is_volume_ID             : 1;
				bool is_subdirectory          : 1;
				bool has_changed_since_backup : 1;
			private:
				uint8_t _unused_zero          : 2;
		} __attribute__((packed));
		static_assert(sizeof(AttribByte)==sizeof(uint8_t),"AttribByte is wrong size!");
		union {
			uint8_t attrib_value;
			AttribByte attrib; //(Microsoft: "DIR_Attr")
		} __attribute__((packed));

		uint8_t reserved_nt;

		class TimePrecise final { public:
			uint8_t tenths_second : 8;
			uint8_t hour          : 5;
			uint8_t minute        : 6;
			uint8_t second        : 5;
		} __attribute__((packed));
		class Time final { public:
			uint8_t hour          : 5;
			uint8_t minute        : 6;
			uint8_t second        : 5;
		} __attribute__((packed));
		class Date final { public:
			uint8_t year          : 7;
			uint8_t month         : 4;
			uint8_t day           : 5;
		} __attribute__((packed));

		TimePrecise creation_time;
		Date creation_date;
		Date lastaccessed_date;

		uint16_t first_cluster_high; //(Microsoft: "DIR_FstClusHI")

		Time lastmodification_time;
		Date lastmodification_date;

		uint16_t  first_cluster_low; //(Microsoft: "DIR_FstClusLO")

		uint32_t file_size; //(Microsoft: "DIR_FileSize")

	public:
		void print(void) const;

		char const* get_filename(void) const;
} __attribute__((packed));
static_assert(sizeof(DirectoryRecord)==32,"DirectoryRecord is wrong size!");

/*class Cluster final {
	public:
		FileSystemFAT*const filesystem;

		int const cluster_number;
		uint32_t const lba;

	public:
		Cluster(FileSystemFAT* filesystem, int cluster_number) :
			filesystem(filesystem),
			cluster_number(cluster_number),
			lba(filesystem->cluster_begin_lba + (cluster_number-2)*filesystem->sectors_per_cluster)
		{}
		inline ~Cluster(void) {}
};*/

class FileSystemFAT final : public FileSystemBase {
	public:
		uint8_t sectors_per_cluster; //Note: all values possible, evidently.  (Microsoft: "BPB_SecPerClus")
		uint32_t root_directory_first_cluster; //Note: Usually 0x00000002.  (Microsoft: "BPB_RootClus")

		uint32_t fat_begin_lba;
		uint32_t cluster_begin_lba;

	public:
		explicit FileSystemFAT(Partition* partition);
		inline virtual ~FileSystemFAT(void) {}

		ObjectBase* get_new_child(ObjectDirectory const* /*directory*/, MOSST::String const& /*path_child*/) const override {
			assert_term(false,"Not implemented!");
			return nullptr;
		}

	private:
		void _fill_directory(ObjectDirectory* directory, uint64_t cluster_number);
};


}}}
