#include "fat.h"

#include "../../../kernel.h"


namespace MOSS { namespace Disk { namespace FileSystem {


uint16_t FAT::_get_next_cluster_number_fat12(LazySector*restrict fatsectors[2],uint32_t fatentry_offset, uint16_t cluster_number) const {
	//Fetch the next cluster number:
	//	Access the FAT entry as an "uint16_t" just as we do for FAT16, but if the cluster number is even,
	//		we only want the lower twelve bits of the sixteen bits we fetch and if the cluster number is odd,
	//		we only want the upper twelve bits of the sixteen bits we fetch.
	uint16_t next_cluster_number;

	if (fatentry_offset == filesystem->bytes_per_sector-1u) {
		uint16_t  low = *reinterpret_cast<uint8_t*>(fatsectors[0]->data+fatentry_offset);
		uint16_t high = *reinterpret_cast<uint8_t*>(fatsectors[1]->data);
		next_cluster_number = (high<<8) | low;
	} else {
		next_cluster_number = *reinterpret_cast<uint16_t*>(fatsectors[0]->data+fatentry_offset);
	}

	if (cluster_number & 0x0001) {
		//"&" is in case of signed right shifts
		next_cluster_number = (next_cluster_number>>4) & 0xF000; //cluster number is odd
	} else {
		next_cluster_number =  next_cluster_number     & 0x0FFF; //cluster number is even
	}

	return next_cluster_number;
}
void     FAT::_set_next_cluster_number_fat12(LazySector*restrict fatsectors[2],uint32_t fatentry_offset, uint16_t cluster_number, uint16_t next_cluster_number) {
	//Set the next cluster number

	uint8_t*restrict  low;
	uint8_t*restrict high;
	if (fatentry_offset == filesystem->bytes_per_sector-1u) {
		low  = fatsectors[0]->data + fatentry_offset;
		high = fatsectors[1]->data;
	} else {
		low  = fatsectors[0]->data + fatentry_offset;
		high = fatsectors[0]->data + fatentry_offset + 1u;
	}

	if (cluster_number & 0x0001) {
		next_cluster_number <<= 4;
		*low  &= 0x0F; //cluster number is odd
	} else {
		next_cluster_number &= 0x0FFF;
		*high &= 0xF0; //cluster number is even
	}
	*low  |=  next_cluster_number     & 0x00FF;
	*high |= (next_cluster_number>>8) & 0x00FF;
}
uint16_t FAT::_get_next_cluster_number_fat16(LazySector*restrict fatsectors[2],uint32_t fatentry_offset) const {
	assert_term(
		fatentry_offset<=512-sizeof(uint16_t) && fatentry_offset%sizeof(uint16_t)==0,
		"FAT16 FAT entry offset %u exceeds sector bound or is unaligned!",fatentry_offset
	);
	return *reinterpret_cast<uint16_t*>(fatsectors[0]->data + fatentry_offset);
}
void     FAT::_set_next_cluster_number_fat16(LazySector*restrict fatsectors[2],uint32_t fatentry_offset, uint16_t next_cluster_number) {
	assert_term(
		fatentry_offset<=512-sizeof(uint16_t) && fatentry_offset%sizeof(uint16_t)==0,
		"FAT16 FAT entry offset %u exceeds sector bound or is unaligned!",fatentry_offset
	);
	*reinterpret_cast<uint16_t*>(fatsectors[0]->data + fatentry_offset) = next_cluster_number;
}
uint32_t FAT::_get_next_cluster_number_fat32(LazySector*restrict fatsectors[2],uint32_t fatentry_offset) const {
	assert_term(
		fatentry_offset<=512-sizeof(uint32_t) && fatentry_offset%sizeof(uint32_t)==0,
		"FAT32 FAT entry offset %u exceeds sector bound or is unaligned!",fatentry_offset
	);
	return *reinterpret_cast<uint32_t*>(fatsectors[0]->data + fatentry_offset) & 0x0FFFFFFFu;
}
void     FAT::_set_next_cluster_number_fat32(LazySector*restrict fatsectors[2],uint32_t fatentry_offset, uint32_t next_cluster_number) {
	assert_term(
		fatentry_offset<=512-sizeof(uint32_t) && fatentry_offset%sizeof(uint32_t)==0,
		"FAT32 FAT entry offset %u exceeds sector bound or is unaligned!",fatentry_offset
	);
	//A FAT32 FAT entry is actually only a 28-bit entry.  The high 4 bits of a FAT32 FAT entry are reserved.
	//	The only time that the high four bits of FAT32 FAT entries should ever be changed is when the volume is
	//	formatted, at which time the whole 32-bit FAT entry should be zeroed, including the high four bits.
	next_cluster_number &= 0x0FFFFFFFu;
	*reinterpret_cast<uint32_t*>(fatsectors[0]->data + fatentry_offset) &= 0xF0000000u;
	*reinterpret_cast<uint32_t*>(fatsectors[0]->data + fatentry_offset) |= next_cluster_number;
}

void FAT::_get_required_sectors_for_access(RelativeLBA fatentry_lba,uint32_t fatentry_offset, LazySector*restrict fatsectors[2]) const {
	assert_term(filesystem->bytes_per_sector==512,"Sector size of %d is not implemented (only 512 is supported)!\n",static_cast<int>(filesystem->bytes_per_sector));

	fatsectors[0] = (*filesystem->partition)[fatentry_lba];

	if (filesystem->type==FileSystemFAT::TYPE::FAT12) {
		//This cluster access spans a sector boundary in the FAT
		if (fatentry_offset == filesystem->bytes_per_sector-1u) {
			assert_term(
				fatentry_lba - filesystem->number_reserved_sectors + 1 < filesystem->sectors_per_fat,
				"FAT12 cluster access spans sector boundary, but this sector is the last sector in the FAT."
			);
			fatsectors[1] = (*filesystem->partition)[fatentry_lba+1];
		}
	}
	//Note that because "filesystem->bytes_per_sector" is always divisible by "2" and "4", you never have to
	//	worry about a FAT16 or FAT32 FAT entry spanning over a sector boundary.
}

bool FAT::get_next_cluster_number(uint32_t cluster_number, uint32_t* next_cluster_number) const {
	RelativeLBA fatentry_lba; uint32_t fatentry_offset;
	get_addr_cluster_fatentry(cluster_number, &fatentry_lba,&fatentry_offset, 0);

	LazySector*restrict fatsectors[2];
	_get_required_sectors_for_access(fatentry_lba,fatentry_offset, fatsectors);

	//Microsoft operating system FAT drivers use the EOC (end of chain) value 0x0FFF for FAT12, 0xFFFF
	//	for FAT16, and 0x0FFFFFFF for FAT32 when they set the contents of a cluster to the EOC mark.
	//	There are various disk utilities that use a different value, however.
	switch (filesystem->type) {
		case FileSystemFAT::TYPE::FAT12: *next_cluster_number=_get_next_cluster_number_fat12(fatsectors,fatentry_offset,cluster_number); return *next_cluster_number<    0x0FF8;
		case FileSystemFAT::TYPE::FAT16: *next_cluster_number=_get_next_cluster_number_fat16(fatsectors,fatentry_offset               ); return *next_cluster_number<    0xFFF8;
		case FileSystemFAT::TYPE::FAT32: *next_cluster_number=_get_next_cluster_number_fat32(fatsectors,fatentry_offset               ); return *next_cluster_number<0x0FFFFFF8;
		default: assert_term(false,"Implementation error!"); return false;
	}
}
void FAT::set_next_cluster_number(uint32_t cluster_number, uint32_t  next_cluster_number)       {
	RelativeLBA fatentry_lba; uint32_t fatentry_offset;
	get_addr_cluster_fatentry(cluster_number, &fatentry_lba,&fatentry_offset, 0);

	LazySector*restrict fatsectors[2];
	_get_required_sectors_for_access(fatentry_lba,fatentry_offset, fatsectors);

	switch (filesystem->type) {
		case FileSystemFAT::TYPE::FAT12: return _set_next_cluster_number_fat12(fatsectors,fatentry_offset,cluster_number,static_cast<uint16_t>(next_cluster_number));
		case FileSystemFAT::TYPE::FAT16: return _set_next_cluster_number_fat16(fatsectors,fatentry_offset,                                     next_cluster_number );
		case FileSystemFAT::TYPE::FAT32: return _set_next_cluster_number_fat32(fatsectors,fatentry_offset,                                     next_cluster_number );
		default: assert_term(false,"Implementation error!");
	}
}

//Returns the LBA of the first sector of the FAT entry for the given cluster number
void FAT::get_addr_cluster_fatentry(uint32_t cluster_number, RelativeLBA*restrict fatentry_lba,uint32_t*restrict fatentry_offset, int fat_index/*=0*/) const {
	//Given any valid cluster number N, where in the FAT(s) is the entry for that cluster number?
	uint32_t fat_offset;
	if        (filesystem->type==FileSystemFAT::TYPE::FAT12) {
		//1.5 bytes per FAT entry.  Multiply by 1.5 without using floating point; the divide by 2 rounds down.
		fat_offset = cluster_number + (cluster_number / 2);
	} else if (filesystem->type==FileSystemFAT::TYPE::FAT16) {
		fat_offset = cluster_number * 2;
	} else { assert_term(filesystem->type==FileSystemFAT::TYPE::FAT32,"Implementation error!");
		fat_offset = cluster_number * 4;
	}

	*fatentry_lba = filesystem->number_reserved_sectors + fat_offset/filesystem->bytes_per_sector;
	*fatentry_lba += fat_index*filesystem->sectors_per_fat;

	*fatentry_offset = fat_offset % filesystem->bytes_per_sector;
}


/*void DirectoryEntry::print(void) const {
	if        (DIR_Name[0]==0x00) {
		kernel->write("Directory Record:    [End]\n");
	} else if (DIR_Name[0]==0xE5) {
		kernel->write("Directory Record: [Unused]\n");
	} else {
		if ((DIR_AttrValue&0x0F) == 0x0F) { //Long file names
			kernel->write("Directory Record:   [Long]\n");
		} else { //Normal
			kernel->write("Directory Record: [Normal] \"%s\"",get_filename());
			if (DIR_Attr.is_read_only) kernel->write(" readonly");
			if (DIR_Attr.is_hidden) kernel->write(" hidden");
			if (DIR_Attr.is_system) kernel->write(" system");
			if (DIR_Attr.is_volume_ID) kernel->write(" volume_ID");
			if (DIR_Attr.is_directory) kernel->write(" subdirectory");
			if (DIR_Attr.has_changed_since_backup) kernel->write(" changed-since-backup");
			kernel->write("\n");
		}
	}
}*/

void DirectoryEntry::add_name_to_string(MOSST::String* string) const {
	/*//TODO: long filenames!
	static uint8_t temp[12];

	for (int i=0;i<11;++i) temp[i]=DIR_Name[i];
	if (temp[0]==0x05) temp[0]=0xE5; 
	temp[11]='\0';
	for (int i=10;i>=0;--i) if (temp[i]==' ') temp[i]='\0';

	return reinterpret_cast<char const*>(temp);*/

	if (DIR_AttrValue&0x0F) { //short
		if (DIR_Name[0]==0x05) string->insert_back(0xE5); //Work around for the Japanese.
		else                   string->insert_back(DIR_NameMain[0]);

		for (int i=1;i<8;++i) string->insert_back(DIR_NameMain[i]);
		for (int i=7;i>=0;--i) { if (DIR_Name[i]==' ') { string->remove_back(); } else { break; } }

		if (DIR_NameExt[0]==' '&&DIR_NameMain[1]==' '&&DIR_NameMain[2]==' ') {
		} else {
			string->insert_back('.');
			for (int i=0;i<3;++i) string->insert_back(DIR_NameExt[i]);
			if (DIR_NameExt[2]==' ') {
				string->remove_back();
				if (DIR_NameExt[1]==' ') {
					string->remove_back();
				}
			}
		}
	} else { //long
		for (int i=0;i<5;++i) string->insert_back(static_cast<char>(LDIR_Name1[i]));
		for (int i=0;i<6;++i) string->insert_back(static_cast<char>(LDIR_Name2[i]));
		for (int i=0;i<2;++i) string->insert_back(static_cast<char>(LDIR_Name3[i]));
	}
}

Chunk::Chunk(FileSystemFAT* filesystem, RelativeLBA lba_start,uint32_t num_sectors) :
	filesystem(filesystem), lba_start(lba_start), num_sectors(num_sectors)
{
	data = new uint8_t[num_sectors*filesystem->bytes_per_sector];

	for (uint32_t i=0u;i<num_sectors;++i) {
		LazySector* sector = (*filesystem->partition)[lba_start+i];
		for (uint32_t j=0u;j<filesystem->bytes_per_sector;++j) {
			data[i*filesystem->bytes_per_sector + j] = sector->data[j];
		}
	}
}
Chunk::~Chunk(void) {
	//TODO: should write back any changes!
	delete [] data;
}


FileSystemFAT::FileSystemFAT(Partition* partition) : FileSystemBase(partition), _fat(this) {
	LazySector* volume_ID = (*partition)[0];

	BS_BPB& bs_bpb = reinterpret_cast<BS_BPB&>(volume_ID->data);

	//Determine FAT type.  The specification has an entire section on this, since apparently everyone gets it wrong.
	#if 1
	//	Number of sectors occupied by root directory.
	//		Note: on a FAT32 volume, ".BPB_RootEntCnt" is always zero; so on a FAT32 volume, this is always zero.  Therefore, on
	//			FAT32 volumes, the "Root Directory Region" does not exist; the root directory is an ordinary directory.
	uint32_t RootDirSectors = (static_cast<uint32_t>(bs_bpb.BPB_RootEntCnt)*32u + static_cast<uint32_t>(bs_bpb.BPB_BytsPerSec)-1u) / static_cast<uint32_t>(bs_bpb.BPB_BytsPerSec);
	//	Next, determine the count of sectors in the data region of the volume:
	uint32_t TotSec;
	if (bs_bpb.BPB_FATSz16 == 0) {
		//If this happens, then we need the field ".BPB_FATSz32" inside the FAT32 structure.  This implies we already know that
		//	the FAT volume is of type FAT32.
		//To check that this isn't an error in the spec., ensure that if "bs_bpb.BPB_FATSz16" is zero, then a FAT12/16 drive can't
		//	pretend to be a FAT32 drive (such that "CountofClusters" is >= 65525).  That is, assume a FAT12/16 drive and try to find:
		//		(TotSec - (bs_bpb.BPB_RsvdSecCnt + (bs_bpb.BPB_NumFATs * FATSz) + RootDirSectors)) / bs_bpb.BPB_SecPerClus >= 65525
		//		TotSec >= 65525 + bs_bpb.BPB_RsvdSecCnt + (bs_bpb.BPB_NumFATs * FATSz) + RootDirSectors
		//		TotSec >= 65526 + FATSz + RootDirSectors
		//	TODO: finish!  I'm going to assume it's okay . . .
		sectors_per_fat = bs_bpb.fat32.BPB_FATSz32;
	} else {
		sectors_per_fat = bs_bpb.BPB_FATSz16;
	}
	if (bs_bpb.BPB_TotSec16 == 0) {
		TotSec = bs_bpb.BPB_TotSec32;
	} else {
		TotSec = bs_bpb.BPB_TotSec16;
	}
	assert_term(
		TotSec <= partition->entry->total_sectors,
		"Total sectors the FAT filesystem purports to have (%d) exceeds the actual number of sectors in the partition (%d)!",
		static_cast<int>(TotSec), static_cast<int>(partition->entry->total_sectors)
	);
	uint32_t DataSec = TotSec - (bs_bpb.BPB_RsvdSecCnt + (bs_bpb.BPB_NumFATs * sectors_per_fat) + RootDirSectors);
	//	Now we determine the count of clusters.  Note that this computation rounds down.
	//		The maximum valid cluster number for the volume is "CountofClusters + 1".
	//		The "count of clusters including the two reserved clusters" is "CountofClusters + 2".
	uint32_t CountofClusters = DataSec / bs_bpb.BPB_SecPerClus;
	//	Now we can determine the FAT type.  This looks bogus, but it's not.
	if        (CountofClusters <  4085) {
		type = FAT12;
	} else if (CountofClusters < 65525) {
		type = FAT16;
	} else {
		type = FAT32;
	}
	#endif

	//Load and check other parameters
	#if 1
	for (int i=0;i<8;++i) oem_name.insert_back(static_cast<char>(bs_bpb.BS_OEMName[i]));
	while (oem_name[oem_name.size-1]==' ') oem_name.remove_back();

	bytes_per_sector = bs_bpb.BPB_BytsPerSec;
	assert_term(
		bytes_per_sector==512 || bytes_per_sector==1024 || bytes_per_sector==2048 || bytes_per_sector==4096,
		"Invalid number of bytes per sector \"%d\"!  Valid counts are \"512\", \"1024\", \"2048\", and \"4096\".",
		static_cast<int>(bytes_per_sector)
	);

	sectors_per_cluster = bs_bpb.BPB_SecPerClus;
	assert_term(
		sectors_per_cluster== 1 || sectors_per_cluster== 2 || sectors_per_cluster== 4 || sectors_per_cluster==  8 ||
		sectors_per_cluster==16 || sectors_per_cluster==32 || sectors_per_cluster==64 || sectors_per_cluster==128,
		"Invalid number of sectors per cluster \"%d\" (unaccepted value)!",
		static_cast<int>(sectors_per_cluster)
	);
	assert_term( //TODO: warning, maybe?
		bytes_per_sector*sectors_per_cluster<=32768,
		"Invalid number of sectors per cluster \"%d\" (bytes/cluster %d exceeds 32768)!",
		static_cast<int>(sectors_per_cluster), bytes_per_sector*sectors_per_cluster
	);

	number_reserved_sectors = bs_bpb.BPB_RsvdSecCnt;
	assert_term(number_reserved_sectors>0,"Number of reserved sectors must be positive!");

	uint8_t num_FATs = bs_bpb.BPB_NumFATs;
	assert_term(num_FATs>0,"Number of FATs must be positive!"); //I assume this is a requirement.

	#ifdef MOSS_DEBUG
	if (type==FAT32) {
		assert_term(bs_bpb.BPB_RootEntCnt==0,"FAT32 root entry count is positive (%d)!",static_cast<int>(bs_bpb.BPB_RootEntCnt));

		assert_term(bs_bpb.BPB_TotSec16==0,"FAT32 16-bit total sector count is positive (%d)!",static_cast<int>(bs_bpb.BPB_TotSec16));
		assert_term(bs_bpb.BPB_TotSec32>0,"FAT32 32-bit total sector count is zero!");

		assert_term(bs_bpb.BPB_FATSz16==0,"FAT32 16-bit FAT sector size is positive (%d)!",static_cast<int>(bs_bpb.BPB_FATSz16));
		assert_term(bs_bpb.fat32.BPB_FATSz32>0,"FAT32 32-bit FAT sector size is zero!",static_cast<int>(bs_bpb.fat32.BPB_FATSz32));
	} else {
		assert_term(
			(bs_bpb.BPB_RootEntCnt*32)%bs_bpb.BPB_BytsPerSec==0,
			"FAT12/16 root entry count * 32 is not a multiple of bytes/sector (%d*32!=%d)!",
			static_cast<int>(bs_bpb.BPB_RootEntCnt), static_cast<int>(bs_bpb.BPB_BytsPerSec)
		);

		if (bs_bpb.BPB_TotSec16==0) {
			assert_term(bs_bpb.BPB_TotSec32>0,"FAT12/16 both 16- and 32-bit total sector count are zero!");
		}

		assert_term(bs_bpb.BPB_FATSz16==0,"FAT12/16 16-bit FAT sector size is zero!");
	}

	switch (bs_bpb.BPB_Media) {
		case 0xF0: //often, removable
		case 0xF8: //standard for non-removable
		case 0xF9: case 0xFA: case 0xFB: case 0xFC: case 0xFD: case 0xFE: case 0xFF:
			break;
		default:
			assert_warn(false,"Invalid value for media type \"%d\".",static_cast<int>(bs_bpb.BPB_Media));
	}

	//"bs_bpb.BPB_FATSz16" is checked above.
	#endif

	assert_term(bs_bpb.BPB_SecPerTrk>0,"Sectors per track is zero!"); //I assume this is a requirement.
	assert_term(bs_bpb.BPB_NumHeads>0,"Number of heads is zero!"); //I assume this is a requirement.

	//BPB_HiddSec

	//Note: "bs_bpb.BPB_TotSec32" checked above, alongside "bs_bpb.BPB_TotSec16".

	#if 1
	if (type==FAT32) {
		//Note: "bs_bpb.fat32.BPB_FATSz32" checked alongside "bs_bpb.fat32.BPB_FATSz32".

		assert_term(!bs_bpb.fat32.BPB_ExtFlags.not_mirrored,"Not mirroring FATs not implemented!"); //TODO: this?

		assert_term(bs_bpb.fat32.BPB_FSVer==0,"Detected version %d.%d; not implemented!",static_cast<int>(bs_bpb.fat32.BPB_FSVerMajor),static_cast<int>(bs_bpb.fat32.BPB_FSVerMinor));

		//BPB_RootClus

		//BPB_FSInfo

		//BPB_BkBootSec

		//BPB_Reserved

		//BS_DrvNum

		//BS_Reserved1

		assert_term(bs_bpb.fat32.BS_BootSig==0x29,"FAT32 invalid extended boot signature %d!",static_cast<int>(bs_bpb.fat32.BS_BootSig));

		//BS_VolID

		for (int i=0;i<11;++i) volume_label.insert_back(bs_bpb.fat32.BS_VolLab[i]);
		while (volume_label[volume_label.size-1]==' ') volume_label.remove_back();

		assert_warn(MOSSC::memcmp("FAT32   ",bs_bpb.fat32.BS_FilSysType, 8)==0,"File system type string was set incorrectly: \"%8s\"!",bs_bpb.fat32.BS_FilSysType);

		//_remainder
	} else {
		//BS_DrvNum

		//BS_Reserved1

		assert_term(bs_bpb.fat1216.BS_BootSig==0x29,"FAT12/16 invalid extended boot signature %d!",static_cast<int>(bs_bpb.fat1216.BS_BootSig));

		//BS_VolID

		//BS_VolLab
		for (int i=0;i<11;++i) volume_label.insert_back(bs_bpb.fat1216.BS_VolLab[i]);
		while (volume_label[volume_label.size-1]==' ') volume_label.remove_back();

		#ifdef MOSS_DEBUG
		bool found       =MOSSC::memcmp("FAT12   ",bs_bpb.fat1216.BS_FilSysType, 8)==0;
		if (!found) found=MOSSC::memcmp("FAT16   ",bs_bpb.fat1216.BS_FilSysType, 8)==0;
		if (!found) found=MOSSC::memcmp("FAT     ",bs_bpb.fat1216.BS_FilSysType, 8)==0;
		assert_warn(!found,"File system type string was set incorrectly: \"%8s\"!",bs_bpb.fat1216.BS_FilSysType);
		#endif

		//_remainder
	}
	#endif

	assert_term(bs_bpb.magic_0xAA55==0xAA55,"Invalid boot sector [510,511] byte signature 0x%X!",static_cast<int>(bs_bpb.magic_0xAA55));
	#endif

	//Calculations
	first_data_sector = number_reserved_sectors + bs_bpb.BPB_NumFATs*sectors_per_fat + RootDirSectors;

	if (type==FAT32) {
		first_root_sector = _cluster_to_sector(bs_bpb.fat32.BPB_RootClus);
	} else {
		first_root_sector = number_reserved_sectors + bs_bpb.BPB_NumFATs*bs_bpb.BPB_FATSz16;
	}

	//Load root directory
	if (type==FAT32) {
		//An ordinary directory
		//Chunk root_chunk(this, first_root_sector,);
	} else {
		//Root directory region
		//Chunk root_chunk(this, first_root_sector,RootDirSectors);
	}

	//I don't know how the Root Directory Region is supposed to work.
	assert_term(type==FAT32,"Only FAT32 implemented!");

	_fill_directory(root,_sector_to_cluster(first_root_sector));
}

void FileSystemFAT::_fill_directory(ObjectDirectory* directory, uint32_t cluster_number) {
	MOSST::LinkedList<Chunk*> clusters;

	_fill_new_cluster_chain(&clusters,cluster_number);
	assert_term(clusters.size>0,"No clusters found for root directory!");

	for (auto iter=clusters.cbegin(); iter!=clusters.cend(); ++iter) {
		DirectoryEntry const* entries = reinterpret_cast<DirectoryEntry const*>((*iter)->data);
		for (int i=0;i<bytes_per_sector*sectors_per_cluster;i+=sizeof(DirectoryEntry)) {
			DirectoryEntry const* entry = entries + i;

			if (entry->DIR_Name[0]==0x00) goto END;

			if (entry->DIR_Name[0]==0xE5) continue; //unused

			MOSST::String name;
			entry->add_name_to_string(&name);

			if (entry->DIR_AttrValue & 0x0F) { //long file names
				int j = 1;
				LOOP:
					DirectoryEntry const* entry_long = entries - j;
					if (entry_long->LDIR_Ord & 0x40) { //last entry
						entry_long->add_name_to_string(&name);
					} else {
						++j;
						goto LOOP;
					}
			}

			directory->paths_children.insert_back(name);
		}
	}
	END:;

	//Chunk* all = new Chunk(this, (*clusters.cbegin())->lba_start,clusters.size()*sectors_per_cluster);

	while (clusters.size>0) delete clusters.remove_back();

	for (int i=0;i<directory->paths_children.size;++i) {
		kernel->write("\"%s\"\n",directory->paths_children[i].c_str());
	}


	/*uint8_t temp[512];

	//AbsoluteLBA lba = cluster_begin_lba + (cluster_number-2ull)*sectors_per_cluster;
	partition->read_sectors(temp, lba,1);

	for (int i=0;i<512/32;++i) {
		reinterpret_cast<DirectoryEntry*>(temp)[i].print();
	}

	kernel->write("Stop %d\n",static_cast<int>(first_root_sector)); while (1);*/

	/*for (int i=0;i<512;++i) {
		kernel->write("%X ",static_cast<int>(temp[i]));
		if (i>0 && i%32==0) kernel->write("\n");
	}*/

	//MOSST::String path;
	//root->paths_children.insert_back(path);
}

//TODO: inline?
RelativeLBA FileSystemFAT::_cluster_to_sector(uint32_t cluster_number) const {
	//TODO: ".sectors_per_cluster" is a power of two.  Could rewite with log and bitshift.
	return (cluster_number - 2u)*sectors_per_cluster + first_data_sector;
}
uint32_t FileSystemFAT::_sector_to_cluster(RelativeLBA sector) const {
	assert_term(sector>=first_data_sector,"Sector %d is smaller than that of the first data sector %d!",static_cast<int>(sector),static_cast<int>(first_data_sector));
	return (sector - first_data_sector)/sectors_per_cluster + 2u;
}

void FileSystemFAT::_fill_new_cluster_chain(MOSST::LinkedList<Chunk*>* clusters, uint32_t first_cluster_number) {
	//Following cluster chains:
	//	See: http://wiki.osdev.org/FAT#Following_Cluster_Chains
	//	1: Extract the value from the FAT for the current cluster.  Go to 2.
	//	2: Is this cluster marked as the last cluster in the chain?  If yes, go to 4.  If no, go to 3.
	//	3: Read the cluster represented by the extracted value and return for more directory parsing.
	//	4: The end of the cluster chain has been found.
	#define cluster_number first_cluster_number
	LOOP:
		if (_fat.get_next_cluster_number(cluster_number,&cluster_number)) {
			kernel->write("Got %d\n",static_cast<int>(cluster_number));
			Chunk* cluster = new Chunk(this, _cluster_to_sector(cluster_number),sectors_per_cluster);
			clusters->insert_back(cluster);
			goto LOOP;
		}
		kernel->write("Missed %d\n",static_cast<int>(cluster_number));
	#undef cluster_number
}


}}}
