// UEFI From Scratch - ThatOSDev ( 2021 - 2022 )
// https://github.com/ThatOSDev/C_UEFI

#ifndef EFILIBS_H
#define EFILIBS_H



#include <stddef.h>

#define HEX 16
#define DECIMAL 10

EFI_HANDLE ImageHandle;
EFI_SYSTEM_TABLE* SystemTable;
EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Volume;
EFI_FILE_PROTOCOL* RootFS;



CHAR16* to_utf16( char const* str )
{
	static CHAR16 g_tmp_utf16[512];
	CHAR16* dst = g_tmp_utf16;
	while ( *str != '\0' )
	{
		*(dst++) = *(str++);
	}
	*dst = '\0';
	return dst;
}


void SetTextPosition(UINT32 Col, UINT32 Row)
{
	SystemTable->ConOut->SetCursorPosition(SystemTable->ConOut, Col, Row);
}
void SetColor(UINTN Attribute)
{
	SystemTable->ConOut->SetAttribute(SystemTable->ConOut, Attribute);
}
void Print( char const* str )
{
	SystemTable->ConOut->OutputString(SystemTable->ConOut,to_utf16(str));
}



void itoa(unsigned long int n, char* buffer, int basenumber)
{
	unsigned long int hold;
	int i, j;
	hold = n;
	i = 0;

	do{
		hold = n % basenumber;
		buffer[i++] = (hold < 10) ? (hold + '0') : (hold + 'a' - 10);
	} while(n /= basenumber);
	buffer[i--] = 0;
	
	for(j = 0; j < i; j++, i--)
	{
		hold = buffer[j];
		buffer[j] = buffer[i];
		buffer[i] = hold;
	}
}

int strcmp(const char* a, const char* b)
{
	int length = strlen(a);
	for(int i = 0; i < length; i++)
	{
		if(a[i] != b[i]){return 0;}
	}
	return 1;
}

typedef struct BLOCKINFO
{
	unsigned long long     BaseAddress;
	unsigned long long     BufferSize;
	unsigned int           ScreenWidth;
	unsigned int           ScreenHeight;
	unsigned int           PixelsPerScanLine;
	unsigned long long     LoaderFileSize;
	EFI_MEMORY_DESCRIPTOR* MMap;
	unsigned long long     MMapSize;
	unsigned long long     MMapDescriptorSize;
	unsigned long long*    rsdp;
} __attribute__((__packed__)) BLOCKINFO;

BLOCKINFO bi;

void InitializeFILESYSTEM()
{
	EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
	EFI_DEVICE_PATH_PROTOCOL *DevicePath;
	
	if((SystemTable->BootServices->HandleProtocol(ImageHandle, &EFI_LOADED_IMAGE_PROTOCOL_GUID, (void**)&LoadedImage)) == EFI_SUCCESS)
	{
		if((SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &EFI_DEVICE_PATH_PROTOCOL_GUID, (void**)&DevicePath)) == EFI_SUCCESS)
		{
			if((SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID, (void**)&Volume)) == EFI_SUCCESS)
			{
				if((Volume->OpenVolume(Volume, &RootFS)) != EFI_SUCCESS)
				{
					Print("Loading Root File System FAILED!\r\n");
				}
			} else {
				Print("Volume Handle FAILED!\r\n");
			}
		} else {
			Print("DevicePath FAILED!\r\n");
		}
	} else {
		Print("LoadedImage FAILED!\r\n");
	}
}

EFI_FILE_PROTOCOL* getFile(char const* FileName)
{
	// This opens a file from the EFI FAT32 file system volume.
	// It loads from root, so you must supply full path if the file is not in the root.
	// Example : "somefolder//myfile"  <--- Notice the double forward slash.
	EFI_FILE_PROTOCOL* FileHandle = NULL;
	if((RootFS->Open(RootFS, &FileHandle, to_utf16(FileName), EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0)) == EFI_NOT_FOUND)
	{
		SetColor(EFI_CYAN);
		Print("WARNING : Unable to find File.\r\n");
	}
	
	return FileHandle;
}

EFI_FILE_PROTOCOL* getDir(char const* DirName)
{
	EFI_FILE_PROTOCOL* FileHandle = NULL;
	if((RootFS->Open(RootFS, &FileHandle, to_utf16(DirName), EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, EFI_FILE_DIRECTORY)) == EFI_NOT_FOUND)
	{
		SetColor(EFI_CYAN);
		Print("WARNING : Unable to find Directory.\r\n");
	}
	
	return FileHandle;
}

void closeFile(EFI_FILE_PROTOCOL* FileHandle)
{
	if((FileHandle->Close(FileHandle)) != EFI_SUCCESS)
	{
		SetColor(EFI_BROWN);
		Print("Closing File Handle FAILED\r\n");
	}
}

void closeDir(EFI_FILE_PROTOCOL* FileHandle)
{
	if((FileHandle->Close(FileHandle)) != EFI_SUCCESS)
	{
		SetColor(EFI_BROWN);
		Print("Closing Directory Handle FAILED\r\n");
	}
}

EFI_FILE_PROTOCOL* createFile(char const* FileName)
{
	EFI_FILE_PROTOCOL* FileHandle = NULL;
	if((RootFS->Open(RootFS, &FileHandle, to_utf16(FileName), EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0)) == EFI_NOT_FOUND)
	{
		SetColor(EFI_CYAN);
		Print("WARNING : Unable to create file. Please check your path.\r\n");
	}
	
	return FileHandle;
}

unsigned int ENTRY_POINT;
void*        OSBuffer_Handle;

void readFile(char const* FileName)
{
	// We get the file size, allocate memory for it,
	// read the file into the buffer, then we close the file.
	EFI_FILE_PROTOCOL* FileHandle = getFile(FileName);
	if(FileHandle != NULL)
	{
		UINT64* FileSize = 0;
		FileHandle->SetPosition(FileHandle, 0xFFFFFFFFFFFFFFFFULL);
		FileHandle->GetPosition(FileHandle, FileSize);
		FileHandle->SetPosition(FileHandle, 0);

		if((SystemTable->BootServices->AllocatePool(EfiLoaderData, *FileSize, (void**)&OSBuffer_Handle)) != EFI_SUCCESS)
		{
			SetColor(EFI_BROWN);
			Print("Allocating Pool FAILED\r\n");
		}
		
		FileHandle->SetPosition(FileHandle, 0);

		if((FileHandle->Read(FileHandle, FileSize, OSBuffer_Handle)) != EFI_SUCCESS)
		{
			SetColor(EFI_BROWN);
			Print("Reading File FAILED\r\n");
		}
		
		SetColor(EFI_LIGHTCYAN);    
		Print("          Dynamic File Signature\r\n");
		SetColor(EFI_BROWN);    
		UINT8* test1 = (UINT8*)OSBuffer_Handle;
		
		// 86 64    <---- BIN
		// 45 4C 46 <---- ELF
		
		UINT8 p1,p2,p3,p4;
		p1 = *test1;
		test1+=1;
		p2 = *test1;
		test1+=1;
		p3 = *test1;
		test1+=1;
		p4 = *test1;

		if(p1 == 100 && p2 == 134)
		{
			Print("          BINARY - 8664 Signature\r\n\r\n\r\n\r\n");
			SetColor(EFI_WHITE);
			test1+=37;
			p1 = *test1;
			test1+=1;
			p2 = *test1;
			test1+=1;
			p3 = *test1;
			test1+=1;
			p4 = *test1;

				char s[2];
				itoa(p1, s, 16);
				Print(s);
				Print("  ");
				
				itoa(p2, s, 16);
				Print(s);
				Print("  ");
				
				itoa(p3, s, 16);
				Print(s);
				Print("  ");
				
				itoa(p4, s, 16);
				Print(s);
				SetColor(EFI_BROWN); 
				Print("  \r\nENTRY POINT : ");
				SetColor(EFI_GREEN); 
				
				ENTRY_POINT = (p4 << 24) | (p3 << 16) | (p2 << 8) | p1 ;
				
				char s2[5];
				itoa(ENTRY_POINT, s2, 10);
				Print(s2);
				Print("  ");
		}
		else if(p2 == 69 && p3 == 76 && p4 == 70)
		{
			Print("ELF - 45 4C 46 Signature\r\n");
			Print("Add your own code + the ELF Header file to make this work.\r\n");
		} else {
			ENTRY_POINT = 0;
			Print("WARNING : RAW FILE HAS BEEN LOADED - No Signature DETECTED\r\n");
		}
		
		closeFile(FileHandle);
		bi.LoaderFileSize = *FileSize;
	}
}

void removeDir( char const* dirName )
{
	EFI_FILE_PROTOCOL* FileHandle = getDir(dirName);
	if(FileHandle != NULL)
	{
		if((FileHandle->Delete(FileHandle)) != EFI_SUCCESS)
		{
			SetColor(EFI_BROWN);
			Print("Deleting Directory FAILED\r\n");
		}
	}
}

void makeDir( char const* dirName )
{
	EFI_FILE_PROTOCOL* FileHandle = NULL;
	
	if((RootFS->Open(RootFS, &FileHandle, to_utf16(dirName), EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, EFI_FILE_DIRECTORY)) != EFI_SUCCESS)
	{
		SetColor(EFI_BROWN);
		Print("Creating Directory FAILED\r\n");
	}
	closeDir(FileHandle);
}

void deleteFile( char const* FileName )
{
	EFI_FILE_PROTOCOL* FileHandle = getFile(FileName);
	if(FileHandle != NULL)
	{
		if((FileHandle->Delete(FileHandle)) != EFI_SUCCESS)
		SetColor(EFI_BROWN);
		Print("Deleting File FAILED\r\n");
	}
}

void WriteToFile(char* buf, char const* FileName)
{
	UINT64 fileSize = strlen(buf);
	EFI_FILE_PROTOCOL* writefilehandle = createFile(FileName);
	if(writefilehandle != NULL)
	{
		if((writefilehandle->Write(writefilehandle, &fileSize, buf)) != EFI_SUCCESS)
		{
			SetColor(EFI_BROWN);
			Print("Writing to File FAILED\r\n");
		}
		closeFile(writefilehandle);
	}
}

void InitializeGOP()
{
	// We initialize the Graphics Output Protocol.
	// This is used instead of the VGA interface.
	SystemTable->BootServices->LocateProtocol(&EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID, 0, (void**)&gop);
	
	UINT32 NewNativeMode = 9000000;
	UINTN  GOPSizeOfInfo      = gop->Mode->SizeOfInfo;
	UINT32 MaxResolutionModes = gop->Mode->MaxMode;
	/*
	EFI_STATUS Status;
	for (UINT32 i = 0; i < MaxResolutionModes; i++)
	{
		EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
		Status = gop->QueryMode(gop, i, &GOPSizeOfInfo, &info);
		if(Status == EFI_SUCCESS)
		{
			if((info->HorizontalResolution == 1920) && (info->VerticalResolution == 1080))
			{
				NewNativeMode = i;
				SetColor(EFI_LIGHTGREEN);
				Print("Setting 1920 x 1080 MODE ... \r\n");
			}
		}
	}
	if(NewNativeMode == 9000000)
	{
		*/
		SetColor(EFI_LIGHTRED);
		//Print("WARNING : Unable to find 1920 x 1080 MODE !\r\nUsing Default GOP.\r\n");
		Print("          During the testing phase we will use the default GOP.\r\n");
		bi.BaseAddress        = gop->Mode->FrameBufferBase;
		bi.BufferSize         = gop->Mode->FrameBufferSize;
		bi.ScreenWidth        = gop->Mode->Info->HorizontalResolution;
		bi.ScreenHeight       = gop->Mode->Info->VerticalResolution;
		bi.PixelsPerScanLine  = gop->Mode->Info->PixelsPerScanLine;
	/*
	} else {
		Status = gop->SetMode(gop, NewNativeMode);
		if(Status == EFI_SUCCESS)
		{
			bi.BaseAddress        = gop->Mode->FrameBufferBase;
			bi.BufferSize         = gop->Mode->FrameBufferSize;
			bi.ScreenWidth        = gop->Mode->Info->HorizontalResolution;
			bi.ScreenHeight       = gop->Mode->Info->VerticalResolution;
			bi.PixelsPerScanLine  = gop->Mode->Info->PixelsPerScanLine;
			SetColor(EFI_LIGHTGREEN);
			Print("1920 x 1080 MODE set.\r\n");
		} else {
			SetColor(EFI_LIGHTRED);
			Print("WARNING : Unable to find 1920 x 1080 MODE !\r\nUsing Default GOP.\r\n");
			bi.BaseAddress        = gop->Mode->FrameBufferBase;
			bi.BufferSize         = gop->Mode->FrameBufferSize;
			bi.ScreenWidth        = gop->Mode->Info->HorizontalResolution;
			bi.ScreenHeight       = gop->Mode->Info->VerticalResolution;
			bi.PixelsPerScanLine  = gop->Mode->Info->PixelsPerScanLine;
		}
	}
	*/
}

void InitializeSystem()
{
	ResetScreen();
	InitializeGOP();
	InitializeFILESYSTEM();
}

#endif // EFILIBS_H