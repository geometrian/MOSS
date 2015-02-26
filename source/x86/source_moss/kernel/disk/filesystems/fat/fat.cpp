#include "fat.h"

#include "../../../kernel.h"

#include "../../disk.h"


namespace MOSS { namespace Disk { namespace FileSystem {


void DirectoryRecord::print(void) const {
	if        (short_filename[0]==0x00) {
		kernel->write("Directory Record:    [End]\n");
	} else if (short_filename[0]==0xE5) {
		kernel->write("Directory Record: [Unused]\n");
	} else {
		if ((attrib_value&0x0F) == 0x0F) { //Long file names
			kernel->write("Directory Record:   [Long]\n");
		} else { //Normal
			kernel->write("Directory Record: [Normal] \"%s\"",get_filename());
			if (attrib.is_read_only) kernel->write(" readonly");
			if (attrib.is_hidden) kernel->write(" hidden");
			if (attrib.is_system) kernel->write(" system");
			if (attrib.is_volume_ID) kernel->write(" volume_ID");
			if (attrib.is_subdirectory) kernel->write(" subdirectory");
			if (attrib.has_changed_since_backup) kernel->write(" changed-since-backup");
			kernel->write("\n");
		}
	}
}

char const* DirectoryRecord::get_filename(void) const {
	//TODO: long filenames!
	static char temp[12];

	for (int i=0;i<11;++i) temp[i]=static_cast<char>(short_filename[i]); temp[11]='\0';
	for (int i=10;i>=0;--i) if (temp[i]==' ') temp[i]='\0';

	return temp;
}


FileSystemFAT::FileSystemFAT(Partition* partition) : FileSystemBase(partition) {
	LazySector* volume_ID = (*partition)[0];

	//(Microsoft: "BPB_BytsPerSec")
	uint16_t bytes_per_sector = *reinterpret_cast<uint16_t const*>(volume_ID->data+0x0B);
	assert_term(bytes_per_sector==512,"Expected bytes per sector to be 512 (got %d)!\n",static_cast<int>(bytes_per_sector));

	sectors_per_cluster = volume_ID->data[0x0D];

	//Note: Usually 0x20.  (Microsoft: "BPB_RsvdSecCnt")
	uint16_t num_reserved_sectors = *reinterpret_cast<uint16_t const*>(volume_ID->data+0x0E);

	//Note: always 2.  (Microsoft: "BPB_NumFATs")
	uint8_t num_fats = volume_ID->data[0x10];
	assert_term(num_fats==2,"Expected number of FATs to be 2 (got %d)!\n",static_cast<int>(num_fats));

	//Note: depends on disk size.  (Microsoft: "BPB_FATSz32")
	uint32_t sectors_per_fat = *reinterpret_cast<uint32_t const*>(volume_ID->data+0x24);

	root_directory_first_cluster = *reinterpret_cast<uint32_t const*>(volume_ID->data+0x2C);

	//Note: always 0x55 0xAA.
	uint16_t signature = *reinterpret_cast<uint16_t const*>(volume_ID->data+0x1FE);
	assert_term(signature==0xAA55,"Invalid signature (got %X)!\n",static_cast<int>(signature));

	fat_begin_lba = partition->entry->relative_sector + num_reserved_sectors;
	cluster_begin_lba = partition->entry->relative_sector + num_reserved_sectors + num_fats*sectors_per_fat;

	_fill_directory(root,root_directory_first_cluster);
}

void FileSystemFAT::_fill_directory(ObjectDirectory* directory, uint64_t cluster_number) {
	uint8_t temp[512];

	uint64_t abs_lba = cluster_begin_lba + (cluster_number-2ull)*sectors_per_cluster;
	partition->drive->read_sectors(temp, abs_lba,1);

	for (int i=0;i<512/32;++i) {
		reinterpret_cast<DirectoryRecord*>(temp)[i].print();
	}

	/*for (int i=0;i<512;++i) {
		kernel->write("%X ",static_cast<int>(temp[i]));
		if (i>0 && i%32==0) kernel->write("\n");
	}*/

	//MOSST::String path;
	//root->paths_children.insert_back(path);
}


}}}
