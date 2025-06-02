// UEFI From Scratch - ThatOSDev ( 2021 - 2022 )
// https://github.com/ThatOSDev/C_UEFI

#include "efi.hpp"
#include "efilibs.hpp"

#define HEX      16
#define DECIMAL  10

char const* GetMonth(unsigned short m)
{
	switch(m)
	{
		case 1:
		{
			return "January";
		}
		case 2:
		{
			return "February";
		}
		case 3:
		{
			return "March";
		}
		case 4:
		{
			return "April";
		}
		case 5:
		{
			return "May";
		}
		case 6:
		{
			return "June";
		}
		case 7:
		{
			return "July";
		}
		case 8:
		{
			return "August";
		}
		case 9:
		{
			return "September";
		}
		case 10:
		{
			return "October";
		}
		case 11:
		{
			return "November";
		}
		case 12:
		{
			return "December";
		}
	}
	return "N/A";
}

char const* GetDayExtension(unsigned short m)
{
	switch(m)
	{
		case 1:
		{
			return "st";
		}
		case 2:
		{
			return "nd";
		}
		case 3:
		{
			return "rd";
		}
		case 21:
		{
			return "st";
		}
		case 22:
		{
			return "nd";
		}
		case 23:
		{
			return "rd";
		}
		case 31:
		{
			return "st";
		}
	}
	return "th";
}

typedef long long INTN;

// From the GNU-EFI
INTN RtCompareGuid(EFI_GUID* Guid1, EFI_GUID* Guid2)
{
	INT32 *g1, *g2, r;
	g1 = (INT32 *) Guid1;
	g2 = (INT32 *) Guid2;
	r  = g1[0] - g2[0];
	r |= g1[1] - g2[1];
	r |= g1[2] - g2[2];
	r |= g1[3] - g2[3];
	return r;
}

INTN CompareGuid(EFI_GUID* Guid1, EFI_GUID* Guid2)
{
	return RtCompareGuid (Guid1, Guid2);
}

EFI_STATUS efi_main(EFI_HANDLE IH, EFI_SYSTEM_TABLE *ST)
{
	ImageHandle = IH;
	SystemTable = ST;
	
	InitializeSystem();
	
	readFile("ThatOS64\\loader.bin");

	char GOPINFO[12];
	
	Print("\r\n\r\n");
	SetColor(EFI_WHITE);
	Print("BaseAddress       : ");
	SetColor(EFI_YELLOW);
	Print("0x");
	itoa(*(unsigned long int*)&bi.BaseAddress, GOPINFO, HEX);
	Print(GOPINFO);
	Print("\r\n");
	
	SetColor(EFI_WHITE);
	Print("BufferSize        : ");
	SetColor(EFI_YELLOW);
	Print("0x");
	itoa(bi.BufferSize, GOPINFO, HEX);
	SetColor(EFI_YELLOW);
	Print(GOPINFO);
	Print("\r\n");

	SetColor(EFI_WHITE);
	Print("Width             : ");
	itoa(bi.ScreenWidth, GOPINFO, DECIMAL);
	SetColor(EFI_YELLOW);
	Print(GOPINFO);
	Print("\r\n");
	
	SetColor(EFI_WHITE);
	Print("Height            : ");
	itoa(bi.ScreenHeight, GOPINFO, DECIMAL);
	SetColor(EFI_YELLOW);
	Print(GOPINFO);
	Print("\r\n");

	SetColor(EFI_WHITE);
	Print("PixelsPerScanLine : ");
	itoa(bi.PixelsPerScanLine, GOPINFO, DECIMAL);
	SetColor(EFI_YELLOW);
	Print(GOPINFO);
	
	SetColor(EFI_CYAN);
	Print("\r\n\r\nDATE ");
	SetColor(EFI_WHITE);
	Print(": ");
	EFI_TIME Time;
	SystemTable->RuntimeServices->GetTime(&Time, NULL);
	
	char snum[4];
	SetColor(EFI_YELLOW);
	Print(GetMonth(Time.Month));
	SetColor(EFI_LIGHTRED);
	Print(" / ");
	SetColor(EFI_YELLOW);
	itoa(Time.Day, snum, DECIMAL);
	Print(snum);
	Print(GetDayExtension(Time.Day));
	SetColor(EFI_LIGHTRED);
	Print(" / ");
	SetColor(EFI_YELLOW);
	itoa(Time.Year, snum, DECIMAL);
	Print(snum);
	Print("\r\n\r\n");
	
	UINT8* loader = (UINT8*)OSBuffer_Handle;
	
	SetColor(EFI_WHITE);
	Print("LOADER FILE SIZE : ");
	itoa(*(unsigned long int*)&bi.LoaderFileSize, GOPINFO, DECIMAL);
	SetColor(EFI_YELLOW);
	Print(GOPINFO);
	SetColor(EFI_CYAN);
	Print(" Bytes\r\n\r\n");

	SetColor(EFI_WHITE);
	Print("Loading ThatOS64 ...");
	
	UINTN                  MemoryMapSize = 0;
	EFI_MEMORY_DESCRIPTOR  *MemoryMap;
	UINTN                  MapKey;
	UINTN                  DescriptorSize;
	UINT32                 DescriptorVersion;
	
	SystemTable->BootServices->GetMemoryMap(&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
	MemoryMapSize += 2 * DescriptorSize;
	SystemTable->BootServices->AllocatePool(2, MemoryMapSize, (void **)&MemoryMap);
	SystemTable->BootServices->GetMemoryMap(&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);

	// We look for the Root System Description Pointer ( RSDP )
	// https://wiki.osdev.org/RSDP
	// ACPI Specs PDF Page 140 Section 5.2.5.2
	EFI_CONFIGURATION_TABLE* configTable = SystemTable->ConfigurationTable;
	unsigned long long* tempRSDP = NULL;
	for(UINTN index = 0; index < SystemTable->NumberOfTableEntries; index++)
	{
		if(CompareGuid(&configTable[index].VendorGuid, &ACPI_20_TABLE_GUID))
		{
			if(strcmp((char*)"RSD PTR ", (char*)configTable->VendorTable) == 1)
			{
				tempRSDP = (unsigned long long*)configTable->VendorTable;
			}
		}
		configTable++;
	}

	auto *KernelBinFile = ((__attribute__((ms_abi)) void (*)(BLOCKINFO*) ) &loader[ENTRY_POINT]);
	
	bi.MMap = MemoryMap;
	bi.MMapSize = MemoryMapSize;
	bi.MMapDescriptorSize = DescriptorSize;
	bi.rsdp = tempRSDP;
	
	SystemTable->BootServices->ExitBootServices(ImageHandle, MapKey);
	
	gdt_init();
	
	KernelBinFile(&bi);
	
	while(1){__asm__ ("hlt");}
	
	// We should not make it to this point.
	return EFI_SUCCESS;
}
