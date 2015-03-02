#pragma once

#include "../../../../includes.h"

#include "../../disk.h"

#include "../filesystem.h"


namespace MOSS { namespace Disk { namespace FileSystem {


//The comments and implementation (including names and comments) in this module mostly sourced/derived from the
//	official Microsoft documentation:
//		http://download.microsoft.com/download/1/6/1/161ba512-40e2-4cc9-843a-923143f3456c/fatgen103.doc
//	See also:
//		http://www.pjrc.com/tech/8051/ide/fat32.html
//	TODO: rename stuff?

//A FAT file system is deivded into three or four sections:
//	1: Reserved
//		Sector 0: Boot sector (BS) and BIOS Parameter Block (BPB)
//	2: FAT Region
//	3: Root Directory Region (not on FAT32)
//	4: File and Directory Data Region

class FileSystemFAT;

class BS_BPB final { public:
	//Jump instruction to boot code.
	uint8_t BS_jmpBoot[3];

	//There are many misconceptions about this field.  It is only a name string.  Microsoft OSes do not pay any
	//	attention to it, but some FAT drivers do.  "MSWIN4.1", is the recommended setting, because it is the
	//	setting least likely to cause compatibility problems.
	char BS_OEMName[8];

	//Number of bytes per sector.  This legal values are "512", "1024", "2048", or "4096".  For maximum
	//	compatibility, only the value "512" should be used.  Note: even if the the device has smaller/larger
	//	sectors, these are still the only valid values.
	uint16_t BPB_BytsPerSec;

	//Number of sectors per allocation unit.  The legal values are "1", "2", "4", "8", "16", "32", "64", and
	//	"128".  Note that a value should never be used that results in a bytes/cluster value
	//	(i.e., BPB_BytsPerSec*BPB_SecPerClus) greater than "32,768".  Some versions of some systems allow
	//	"65,536" bytes/cluster.  This is wrong.
	uint8_t BPB_SecPerClus;

	//Number of reserved sectors in the Reserved region of the volume starting at the first sector of the
	//	volume.  Never zero.  For FAT12 and FAT16, this value is best left at "1".  For FAT32 volumes, this
	//	value is typically "32".
	uint16_t BPB_RsvdSecCnt;

	//The number of FAT data structures on the volume.  For compatibility, this field should contain the value
	//	"2" for any kind of FAT volume.  The reason this is larger than "1" is for redundancy.
	uint8_t BPB_NumFATs;

	//For FAT12 and FAT16 volumes, this field contains the number of 32-byte directory entries in the root
	//	directory.  When multiplied by "32", it should result in an even multiple of ".BPB_BytsPerSec".  For
	//	maximum compatibility, FAT16 volumes should use the value "512".  For FAT32 volumes, this field must be
	//	zero.
	uint16_t BPB_RootEntCnt;

	//For FAT12/FAT16, the old 16-bit total number of sectors in all regions of the volume.
	//	If zero, then ".BPB_TotSec32" must be positive.
	//	Otherwise, if the total sector count "fits" (i.e., is <= 0xFFFF), then ".BPB_TotSec32" is zero.
	//For FAT32 volumes, this field must be zero (and ".BPB_TotSec32" must be positive).
	uint16_t BPB_TotSec16;

	//The media type; (usually) no longer used.  The legal values for this field are "0xF0" (often, removable
	//	media), "0xF8" (standard value for non-removable media), "0xF9", "0xFA", "0xFB", "0xFC", "0xFD", "0xFE",
	//	and "0xFF".  This value must be the same as the low byte of the FAT[0] entry.
	uint8_t BPB_Media;

	//For FAT12/FAT16, the total number of sectors occupied by one FAT.  On FAT32 volumes this field must be zero,
	//	and "fat32.BPB_FATSz32" contains the FAT size count.
	uint16_t BPB_FATSz16;

	//Sectors per track for interrupt 0x13.  This field is only relevant for media that have a geometry (volume
	//	is broken down into tracks by multiple heads and cylinders) and are visible on interrupt 0x13. This field
	//	contains the sectors/track geometry value.
	uint16_t BPB_SecPerTrk;

	//Number of heads for interrupt 0x13.  This field is only relevant in the same way as ".BPB_SecPerTrk".
	//	Example: on a 1.44 MB 3.5-inch floppy drive, this value is "2".
	uint16_t BPB_NumHeads;

	//Number of hidden sectors preceding the partition that contains this FAT volume (always zero if not
	//	partitioned, although apparently being partitioned is not a guarantee that this is nonzero).
	//	Generally only relevant for media visible on interrupt 0x13.
	uint32_t BPB_HiddSec;

	//The new 32-bit total count of sectors in all regions of the volume.
	//	For FAT12/FAT16, if zero, then ".BPB_TotSec16" must be positive.
	//	For FAT32, must be positive.
	uint32_t BPB_TotSec32;

	//This next section varies depending on the FAT volume type.
	class FAT1216 final { public:
		//Interrupt 0x13 drive number (e.g. 0x80).  This field supports MS-DOS bootstrap and is set to the
		//	Interrupt 0x13 drive number of the media (0x00 for floppy disks, 0x80 for hard disks).  Claimed to
		//	be OS-specific.
		uint8_t BS_DrvNum;

		//Reserved (used by Windows NT).  Code that formats FAT volumes should always set this byte to 0.
		uint8_t BS_Reserved1;

		//Extended boot signature (0x29).  This is a signature byte that indicates that the following three
		//	fields in the boot sector are present.
		uint8_t BS_BootSig;

		//Volume serial number.  This field, together with BS_VolLab, supports volume tracking on removable
		//	media.  These values allow FAT file system drivers to detect that the wrong disk is inserted in a
		//	removable drive.  This ID is usually generated by simply combining the current date and time into a
		//	32-bit value.
		uint32_t BS_VolID;

		//Volume label.  This field matches the 11-byte volume label recorded in the root directory.
		//	Note: drivers must update this field when the volume label file in the root directory has its name
		//		changed or created.  If there is no volume label then the string is "NO NAME    ".
		char BS_VolLab[11];

		//One of the strings "FAT12   ", "FAT16   ", or "FAT     ".  This string is informational only and is
		//	not used by Microsoft file system drivers to determine FAT type because it is frequently not set
		//	correctly or is not present.  For maximum compatibility, drivers should make an effort to set it
		//	based on the FAT type, though.
		char BS_FilSysType[8];

		//Bytes 62 through 510 of sector.
		uint8_t _remainder[510-62];
	} __attribute__((packed));
	class FAT32 final { public:
		//32-bit count of sectors occupied by one FAT.  ".BPB_FATSz16" must be 0.  Presumably must be positive.
		uint32_t BPB_FATSz32;

		//Normally, the (logical) FAT is stored in every (physical) FAT on the volume.  This process is called "mirroring".
		//	See: http://www.ntfs.com/fat-mirrow.htm
		class BPB_ExtFlags_Type final { public:
			uint8_t active_FAT_index : 4; //Zero-based number of active FAT.  Only valid if mirroring is disabled.
			uint8_t        reserved1 : 3; //Reserved.
			bool        not_mirrored : 1; //0 means mirroring disabled.  1 means only one FAT is active; it is the one referenced in ".active_FAT_index".
			uint32_t      _reserved2 : 8; //Reserved.
		} __attribute__((packed));
		static_assert(sizeof(BPB_ExtFlags_Type)==sizeof(uint16_t),"BPB_ExtFlags_Type is wrong size!");
		BPB_ExtFlags_Type BPB_ExtFlags;

		//Version number of the FAT32 volume (for future standards).  Current version is 0.0.  Note: ought not
		//	to mount volume if newer.
		union {
			uint16_t BPB_FSVer;
			struct {
				uint8_t BPB_FSVerMinor; //0
				uint8_t BPB_FSVerMajor; //0
			} __attribute__((packed));
		} __attribute__((packed));

		//The cluster number of the first cluster of the root directory.  Often "2".  Disk utilities that change the
		//	location of the root directory should try to place the first cluster of the root directory in the first
		//	non-bad cluster on the drive (i.e., in cluster 2, unless it's marked bad).  This allows disk repair
		//	utilities to easily find the root directory if this field accidentally gets zeroed.
		uint32_t BPB_RootClus;

		//Sector number of FSINFO structure in the reserved area of the FAT32 volume.  Usually "1".
		//	Note: there will be a copy of the FSINFO structure in BackupBoot, but only the copy pointed to by this
		//	field will be kept up to date (i.e., both the primary and backup boot record will point to the same FSINFO
		//	sector).
		uint16_t BPB_FSInfo;

		//If non-zero, indicates in the reserved area of the volume the sector number of a copy of the boot record.
		//	Usually "6".  No other value is recommended.
		uint16_t BPB_BkBootSec;

		//Reserved for future standards.  Currently set to zero.
		uint8_t BPB_Reserved[12];

		//Same meaning as "FAT1216::BS_DrvNum".
		uint8_t BS_DrvNum;

		//Same meaning as "FAT1216::BS_Reserved1".
		uint8_t BS_Reserved1;

		//Same meaning as "FAT1216::BS_BootSig".
		uint8_t BS_BootSig;

		//Same meaning as "FAT1216::BS_VolID".
		uint32_t BS_VolID;

		//Same meaning as "FAT1216::BS_VolLab".
		uint8_t BS_VolLab[11];

		//Always set to "FAT32   ".  Informational only.  See note under "FAT1216::BS_FilSysType".
		uint8_t BS_FilSysType[8];

		//Bytes 90 through 510 of sector.
		uint8_t _remainder[510-90];
	} __attribute__((packed));
	union {
		FAT1216 fat1216;
		FAT32 fat32;
	} __attribute__((packed));

	//Magic numbers
	union {
		uint16_t magic_0xAA55;
		struct {
			uint8_t magic_0x55;
			uint8_t magic_0xAA;
		} __attribute__((packed));
	} __attribute__((packed));

	//If the sector size (i.e. ".BPB_BytsPerSec") is larger than 512, then more stuff might be here.
} __attribute__((packed));
static_assert(sizeof(BS_BPB)==512,"BS_BPB is wrong size!");

class FAT final {
	public:
		FileSystemFAT*const filesystem;

	public:
		inline explicit FAT(FileSystemFAT* filesystem) : filesystem(filesystem) {}
		inline ~FAT(void) {}

	private:
		uint16_t _get_next_cluster_number_fat12(LazySector*restrict fatsectors[2],uint32_t fatentry_offset, uint16_t cluster_number) const;
		void     _set_next_cluster_number_fat12(LazySector*restrict fatsectors[2],uint32_t fatentry_offset, uint16_t cluster_number, uint16_t next_cluster_number);
		uint16_t _get_next_cluster_number_fat16(LazySector*restrict fatsectors[2],uint32_t fatentry_offset) const;
		void     _set_next_cluster_number_fat16(LazySector*restrict fatsectors[2],uint32_t fatentry_offset, uint16_t next_cluster_number);
		uint32_t _get_next_cluster_number_fat32(LazySector*restrict fatsectors[2],uint32_t fatentry_offset) const;
		void     _set_next_cluster_number_fat32(LazySector*restrict fatsectors[2],uint32_t fatentry_offset, uint32_t next_cluster_number);

		void _get_required_sectors_for_access(RelativeLBA fatentry_lba,uint32_t fatentry_offset, LazySector*restrict fatsectors[2]) const;

	public:
		//Get the next cluster number.  If returns false, then the next cluster number represents the end of the cluster chain.
		bool get_next_cluster_number(uint32_t cluster_number, uint32_t* next_cluster_number) const;
		void set_next_cluster_number(uint32_t cluster_number, uint32_t  next_cluster_number);

		//Returns the LBA of the first sector of the FAT entry for the given cluster number
		void get_addr_cluster_fatentry(uint32_t cluster_number, RelativeLBA*restrict fatentry_lba,uint32_t*restrict fatentry_offset, int fat_index=0) const;
};

class DirectoryEntry final {
	public:
		//File attributes
		class Attr_Type final {
			public:
				bool is_read_only             : 1; //ATTR_READ_ONLY      := 0x01
				bool is_hidden                : 1; //ATTR_HIDDEN         := 0x02
				bool is_system                : 1; //ATTR_SYSTEM         := 0x04
				bool is_volume_ID             : 1; //ATTR_VOLUME_ID      := 0x08
				bool is_directory             : 1; //ATTR_DIRECTORY      := 0x10
				bool has_changed_since_backup : 1; //ATTR_ARCHIVE        := 0x20
				                                    //ATTR_LONG_NAME      := ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID = 0x0F
				                                    //ATTR_LONG_NAME_MASK := ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID | ATTR_DIRECTORY | ATTR_ARCHIVE = 0x2F
			private:
				//The upper two bits of the attribute byte are reserved, and should always be set to zero when a file is created and never modified or looked at after that.
				uint8_t _reserved             : 2;
		} __attribute__((packed));
		static_assert(sizeof(Attr_Type)==sizeof(uint8_t),"Attr_Type is wrong size!");

		union {
			//Short directory entries
			struct {
				//Short name ("alias", if using long names).
				//	Note: I'm keeping it as "uint8_t" because it is overloaded for other flags.
				union {
					uint8_t DIR_Name[11];
					struct {
						uint8_t DIR_NameMain[8];
						uint8_t DIR_NameExt[3];
					} __attribute__((packed));
				} __attribute__((packed));

				//File attributes
				union {
					uint8_t DIR_AttrValue;
					Attr_Type DIR_Attr;
				} __attribute__((packed));

				//Reserved for use by Windows NT.  Set to zero when a file is created and never modify or look at it after that.
				uint8_t DIR_NTRes;

				//Millisecond stamp at file creation time.  This field actually contains a count of tenths of a second.  The
				//	granularity of the seconds part of ".DIR_CrtTime" is two seconds, so this field is a count of tenths-of-a-second
				//	and its valid value range is [0,199].
				uint8_t DIR_CrtTimeTenth;

				//Optional timing data not covered in spec., but referenced in it.
				union {
					uint8_t _optional[20-14];
					struct {
						//Creation time
						union {
							uint16_t DIR_CrtTime;
							struct {
								uint8_t DIR_CrtTimeTS : 5; //Two-second count; valid range [0,29] maps to [0,58] seconds.
								uint8_t DIR_CrtTimeM  : 6; //Minutes
								uint8_t DIR_CrtTimeH  : 5; //Hours
							} __attribute__((packed));
						} __attribute__((packed));
						//Creation date
						union {
							uint16_t DIR_CrtDate;
							struct {
								uint8_t DIR_CrtDateD : 5; //Day [1,31]
								uint8_t DIR_CrtDateM : 4; //Month [1,12]
								uint8_t DIR_CrtDateY : 7; //Count of years from 1980; [0,127] maps to [1980,2107].
							} __attribute__((packed));
						} __attribute__((packed));
						//Last accessed date
						union {
							uint16_t DIR_LstAccDate;
							struct {
								uint8_t DIR_LstAccDateD : 5; //Day [1,31]
								uint8_t DIR_LstAccDateM : 4; //Month [1,12]
								uint8_t DIR_LstAccDateY : 7; //Count of years from 1980; [0,127] maps to [1980,2107].
							} __attribute__((packed));
						} __attribute__((packed));

						//"DIR_CrtTimeMil" is referenced in the specification.  TODO: what is it and where does it go?
					} __attribute__((packed));
				} __attribute__((packed));

				//High word of this entry's first cluster number (always 0 for a FAT12 or FAT16 volume).
				uint16_t DIR_FstClusHI;

				//Time of last write.  Note: file creation is considered a write.
				uint16_t DIR_WrtTime;
				//Date of last write.  Note: file creation is considered a write.
				uint16_t DIR_WrtDate;

				//Low word of this entry's first cluster number.
				uint16_t DIR_FstClusLO;

				//32-bit DWORD holding this file's size in bytes.
				uint32_t DIR_FileSize;
			} __attribute__((packed));

			//Long directory entries
			struct {
				//The order of this entry in the sequence of long dir entries associated with the short dir entry (starting
				//	from one).  If |-ed with 0x40 (LAST_LONG_ENTRY), indicates that this long dir entry is the last.
				uint8_t LDIR_Ord;

				//Characters 1-5 of the long-name sub-component in this dir entry.
				uint16_t LDIR_Name1[5];

				//File attributes.  Must be ATTR_LONG_NAME := 0x0F.
				union {
					uint8_t LDIR_AttrValue;
					Attr_Type LDIR_Attr;
				} __attribute__((packed));

				//If zero, indicates a directory entry that is a sub-component of a long name.  Non-zero implies other dirent types.
				//	Note: other values reserved for future extensions.
				uint8_t LDIR_Type;

				//Checksum of name in the short dir entry.  Computed using:
				//	uint8_t ChkSum(uint8_t const* LDIR_Ord) {
				//		uint8_t result = 0;
				//		for (int i=11; i>0; --i) {
				//			//Note: the operation is an unsigned char rotate right
				//			result = ((result&1) ? 0x80 : 0x00) + (result>>1) + *(LDIR_Ord++);
				//		}
				//		return result;
				//	}
				uint8_t LDIR_Chksum;

				//Characters [6,11] of the long-name sub-component in this dir entry.
				uint16_t LDIR_Name2[6];

				//Must be zero.  This is an artifact of the FAT "first cluster" and must be zero for compatibility with existing
				//	disk utilities.  It is meaningless in the context of a long dir entry.
				uint16_t LDIR_FstClusLO;

				//Characters 12-13 of the long-name sub-component in this dir entry.
				uint16_t LDIR_Name3[2];
			} __attribute__((packed));
		} __attribute__((packed));

	public:
		void add_name_to_string(MOSST::String* string) const;

		void print(void) const;
} __attribute__((packed));
static_assert(sizeof(DirectoryEntry)==32,"DirectoryEntry is wrong size!");

//A block of file data.  A [bunch of ]cluster(s) or the root directory for FAT12/16.
class Chunk final {
	public:
		FileSystemFAT*const filesystem;

		RelativeLBA lba_start;
		uint32_t const num_sectors;

		uint8_t* data;

	public:
		Chunk(FileSystemFAT* filesystem, RelativeLBA lba_start,uint32_t num_sectors);
		~Chunk(void);
};

class ObjectFileFAT final : public ObjectFileBase {
	private:
		uint32_t const _cluster_number;

		uint32_t const _file_size; //in bytes

	public:
		ObjectFileFAT(FileSystemFAT* filesystem, char const* name, uint32_t cluster_number, uint32_t file_size);
		inline ~ObjectFileFAT(void) {}

		MOSST::Vector<uint8_t>* get_new_data(void) const override;
};
class ObjectDirectoryFAT final : public ObjectDirectoryBase {
	private:
		uint32_t const _cluster_number;

	public:
		ObjectDirectoryFAT(FileSystemFAT* filesystem, char const* name, uint32_t cluster_number);
		inline ~ObjectDirectoryFAT(void) {}

		void load_entries(void) override;

		ObjectBase* get_child(MOSST::String const& child_name) const override;
};

class FileSystemFAT final : public FileSystemBase {
	friend class ObjectDirectoryFAT;
	friend class ObjectFileFAT;
	public:
		//User can read all of these but should not change any of them directly.

		enum Type {
			FAT12 = 12,
			FAT16 = 16,
			FAT32 = 32
		} type;

		MOSST::String volume_label;

		MOSST::String oem_name;

		uint16_t bytes_per_sector;
		uint8_t sectors_per_cluster;
		uint32_t sectors_per_fat;

		//Number of reserved sectors.  Also equal to the relative LBA of the first sector of the first FAT entry.
		uint16_t number_reserved_sectors;

		//Relative LBA of the data region; the first sector of cluster 2
		RelativeLBA first_data_sector;
		//Relative LBA of the root directory
		RelativeLBA first_root_sector;

	private:
		FAT _fat;

	public:
		explicit FileSystemFAT(Partition* partition);
		inline virtual ~FileSystemFAT(void) {}

		ObjectFileBase* open(char const* path) override;

	private:
		//Returns LBA of the first sector of the given cluster.
		RelativeLBA _cluster_to_sector(uint32_t cluster_number) const;
		//Returns the cluster number associated with the given sector.
		uint32_t _sector_to_cluster(RelativeLBA sector) const;

		void _fill_new_cluster_chain(MOSST::LinkedList<Chunk*>* clusters, uint32_t first_cluster_number);

		void _print_recursive(ObjectDirectoryFAT* dir, int depth) const;
};


}}}
