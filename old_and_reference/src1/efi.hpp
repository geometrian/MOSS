#pragma once

#include "stdafx.hpp"



namespace EFI
{



//Basic Data Types
//	See UEFI Standard §2.3.1 "Data Types"
//	https://uefi.org/specs/UEFI/2.10/02_Overview.html#data-types

using BOOLEAN = bool;
static_assert( sizeof(bool) == sizeof(uint8) );

using UINTN = std::uintptr_t;
using  INTN = std:: intptr_t;

using UINT8  = uint8 ;
using  INT8  =  int8 ;
using UINT16 = uint16;
using  INT16 =  int16;
using UINT32 = uint32;
using  INT32 =  int32;
using UINT64 = uint64;
using  INT64 =  int64;
//	(not doing 128, but it's defined)

using CHAR8  = char   ; //ASCII, ISO-Latin-1
using CHAR16 = wchar_t; //UCS-2

using VOID = void;

//	GUID; based on time
//		See also UEFI Standard §A "GUID and Time Formats"
//		https://uefi.org/specs/UEFI/2.10/Apx_A_GUID_and_Time_Formats.html
struct alignas(64/8) GUID final
{
	uint32 time_low;
	uint16 time_mid;
	uint16 time_high_and_ver;
	uint8  data[ 2 + 6 ];
};
static_assert( sizeof(GUID) == 128/8 );

using STATUS = UINTN;
// https://uefi.org/specs/UEFI/2.10/Apx_D_Status_Codes.html

using HANDLE = VOID*;

using EVENT = VOID*;

using LBA = UINT64;

using TPL = UINTN;

struct MAC_ADDRESS final
{
	uint8 data[ 32 ];
};

struct IPv4_ADDRESS final
{
	uint8 data[ 4 ];
};
struct IPv6_ADDRESS final
{
	uint8 data[ 16 ];
};
union alignas(4) IP_ADDRESS final
{
	IPv4_ADDRESS ipv4;
	IPv6_ADDRESS ipv6;
};

enum class REVISION : UINT32
{
	EFI_1__0_2 = (1<<16) |   2,
	EFI_1_10_0 = (1<<16) |  10,
	EFI_2__0_0 = (2<<16) |   0,
	EFI_2__1_0 = (2<<16) |  10,
	EFI_2__2_0 = (2<<16) |  20,
	EFI_2__3_0 = (2<<16) |  30,
	EFI_2__3_1 = (2<<16) |  31,
	EFI_2__4_0 = (2<<16) |  40,
	EFI_2__5_0 = (2<<16) |  50,
	EFI_2__6_0 = (2<<16) |  60,
	EFI_2__7_0 = (2<<16) |  70,
	EFI_2__8_0 = (2<<16) |  80,
	EFI_2__9_0 = (2<<16) |  90,
	EFI_2_10_0 = (2<<16) | 100
};

//UEFI Table Header, used for various table types
//	See UEFI Standard §4.2 "EFI Table Header"
//	https://uefi.org/specs/UEFI/2.10/04_EFI_System_Table.html#efi-table-header
struct TABLE_HEADER final
{
	UINT64 signature;
	UINT32 revision;
	UINT32 header_size;
	UINT32 crc32;
	UINT32 reserved;
};



//Text Input
//	See UEFI Standard §12.3 "Simple Text Input Protocol"
//	https://uefi.org/specs/UEFI/2.10/12_Protocols_Console_Support.html#simple-text-input-protocol
struct INPUT_KEY final
{
	UINT16 scan_code;
	CHAR16 codepoint;
};
class SIMPLE_TEXT_INPUT_PROTOCOL;
using INPUT_RESET    = STATUS(*)( SIMPLE_TEXT_INPUT_PROTOCOL* self, BOOLEAN extra_verif );
using INPUT_READ_KEY = STATUS(*)( SIMPLE_TEXT_INPUT_PROTOCOL* self, INPUT_KEY* key );
class SIMPLE_TEXT_INPUT_PROTOCOL final
{
	public:
		static constexpr GUID guid =
		{
			0x387477c1, 0x69c7, 0x11d2, {0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b}
		};

	private:
		INPUT_RESET _reset;
		INPUT_READ_KEY _read;
		EVENT _wait_evt;

	public:
		//TODO: `[[nodiscard]]`
		STATUS reset( BOOLEAN extra_verif ) noexcept { return _reset( this, extra_verif ); }
		STATUS read( INPUT_KEY* key ) noexcept { return _read( this, key ); }
};

//Text Output
//	See UEFI Standard §12.4 "Simple Text Output Protocol"
//	https://uefi.org/specs/UEFI/2.10/12_Protocols_Console_Support.html#simple-text-output-protocol
struct SIMPLE_TEXT_OUTPUT_PROTOCOL;
using TEXT_RESET               = STATUS(*)( SIMPLE_TEXT_OUTPUT_PROTOCOL* self, BOOLEAN extra_verif );
using TEXT_STRING              = STATUS(*)( SIMPLE_TEXT_OUTPUT_PROTOCOL* self, CHAR16* str );
using TEXT_TEST_STRING         = STATUS(*)( SIMPLE_TEXT_OUTPUT_PROTOCOL* self, CHAR16* str );
using TEXT_SET_ATTRIBUTE       = STATUS(*)( SIMPLE_TEXT_OUTPUT_PROTOCOL* self, UINTN attrib );
using TEXT_CLEAR_SCREEN        = STATUS(*)( SIMPLE_TEXT_OUTPUT_PROTOCOL* self );
using TEXT_SET_CURSOR_POSITION = STATUS(*)( SIMPLE_TEXT_OUTPUT_PROTOCOL* self, UINTN col,UINTN row );
using TEXT_ENABLE_CURSOR       = STATUS(*)( SIMPLE_TEXT_OUTPUT_PROTOCOL* self, BOOLEAN visible );
class SIMPLE_TEXT_OUTPUT_PROTOCOL final
{
	public:
		static constexpr GUID guid =
		{
			0x387477c2, 0x69c7, 0x11d2, {0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b}
		};

	private:
		TEXT_RESET _reset;
		TEXT_STRING      _output_str;
		TEXT_TEST_STRING _test_str; //checks if string is printable
		#if 0
		TEXT_QUERY_MODE _query_mode;
		TEXT_SET_MODE   _set_mode;
		#else //Not implemented
		void* _query_mode;
		void* _set_mode;
		#endif
		TEXT_SET_ATTRIBUTE _set_attrib;
		TEXT_CLEAR_SCREEN _clear_screen;
		TEXT_SET_CURSOR_POSITION _set_pos;
		TEXT_ENABLE_CURSOR _enable_cursor;
		#if 0
		SIMPLE_TEXT_OUTPUT_MODE* Mode;
		#else //Not implemented
		void** Mode;
		#endif

	public:
		//TODO: `[[nodiscard]]`
		STATUS reset( BOOLEAN extra_verif ) noexcept { return _reset( this, extra_verif ); }

		STATUS print   ( CHAR16* str ) noexcept { return _output_str( this, str ); }
		STATUS check_ok( CHAR16* str ) noexcept { return _test_str  ( this, str ); }

		//Lower bits are FG blue, green, red, and extra brightness.  Upper four are the same for BG.
		//	Note this is *not* the same as ANSI escape code colors ( https://en.wikipedia.org/wiki/ANSI_escape_code#Colors )
		STATUS set_color( UINT8 bitfield___bg_Brgb___fg_Brgb ) noexcept
		{
			// Note: VirtualBox returns `EFI_UNSUPPORTED` (which is not allowed).
			return _set_attrib( this, bitfield___bg_Brgb___fg_Brgb );
		}
		STATUS set_cursor_pos( uint32 col_ind, uint32 row_ind ) noexcept
		{
			return _set_pos( this, col_ind,row_ind );
		}
		STATUS set_cursor_visible( bool visible ) noexcept { return _enable_cursor( this, visible ); }

		STATUS fill_with_bg() noexcept { return _clear_screen(this); }
};



//Delay
//	See UEFI Standard §7.5.2. "EFI_BOOT_SERVICES.Stall()"
//	https://uefi.org/specs/UEFI/2.10/07_Services_Boot_Services.html#efi-boot-services-stall
using STALL              = STATUS(*)( UINTN microseconds );
using SET_WATCHDOG_TIMER = STATUS(*)( UINTN timeout_s, UINT64 retcode, UINTN data_sz,CHAR16* data );

//EFI Boot Services Table
//	See UEFI Standard §4.4 "EFI Boot Services Table"
//	https://uefi.org/specs/UEFI/2.10/04_EFI_System_Table.html#efi-boot-services-table
class BOOT_SERVICES final
{
	public:
		static constexpr UINT64 signature = 0x56524553544f4f42;

	private:
		TABLE_HEADER _header;

	public:
		// Task priority
		#if 0
		RAISE_TPL   tpl_raise;
		RESTORE_TPL tpl_restore;
		#else //Not implemented
		void* tpl_raise;
		void* tpl_restore;
		#endif

		// Memory
		#if 0
		ALLOCATE_PAGES pages_alloc;
		FREE_PAGES     pages_free ;
		GET_MEMORY_MAP get_mem_map;
		ALLOCATE_POOL  pool_alloc;
		FREE_POOL      pool_free ;
		#else //Not implemented
		void* pages_alloc;
		void* pages_free ;
		void* get_mem_map;
		void* pool_alloc;
		void* pool_free ;
		#endif

		// Event & Timer
		#if 0
		CREATE_EVENT   evt_create;
		SET_TIMER      evt_set_timer;
		WAIT_FOR_EVENT evt_wait;
		SIGNAL_EVENT   evt_signal;
		CLOSE_EVENT    evt_close;
		CHECK_EVENT    evt_check;
		#else //Not implemented
		void* evt_create;
		void* evt_set_timer;
		void* evt_wait;
		void* evt_signal;
		void* evt_close;
		void* evt_check;
		#endif

		// Protocol Handler
		#if 0
		INSTALL_PROTOCOL_INTERFACE   proto_install  ;
		REINSTALL_PROTOCOL_INTERFACE proto_reinstall;
		UNINSTALL_PROTOCOL_INTERFACE proto_uninstall;
		HANDLE_PROTOCOL              proto_handle;
		VOID*                        reserved;
		REGISTER_PROTOCOL_NOTIFY     proto_register_notify;
		LOCATE_HANDLE                locate_handle;
		LOCATE_DEVICE_PATH           locate_device_path;
		INSTALL_CONFIGURATION_TABLE  install_config_table;
		#else //Not implemented
		void* proto_install  ;
		void* proto_reinstall;
		void* proto_uninstall;
		void* proto_handle;
		void* reserved;
		void* proto_register_notify;
		void* locate_handle;
		void* locate_device_path;
		void* install_config_table;
		#endif

		// Image
		#if 0
		IMAGE_UNLOAD       image_load ;
		IMAGE_START        image_start;
		EXIT               exit;
		IMAGE_UNLOAD       image_unload;
		EXIT_BOOT_SERVICES exit_boot_services;
		#else //Not implemented
		void* image_load ;
		void* image_start;
		void* exit;
		void* image_unload;
		void* exit_boot_services;
		#endif

		// Miscellaneous
		#if 0
		GET_NEXT_MONOTONIC_COUNT get_next_monotonic_count;
		#else //Not implemented
		void* get_next_monotonic_count;
		#endif
		STALL                    delay;
		SET_WATCHDOG_TIMER       set_watchdog_timer;

		// DriverSupport
		#if 0
		CONNECT_CONTROLLER    controller_connect   ;
		DISCONNECT_CONTROLLER controller_disconnect;
		#else //Not implemented
		void* controller_connect   ;
		void* controller_disconnect;
		#endif

		// Open and Close Protocol
		#if 0
		OPEN_PROTOCOL             proto_open ;
		CLOSE_PROTOCOL            proto_close;
		OPEN_PROTOCOL_INFORMATION proto_open_info;
		#else //Not implemented
		void* proto_open ;
		void* proto_close;
		void* proto_open_info;
		#endif

		// Library
		#if 0
		PROTOCOLS_PER_HANDLE                   protos_per_handle;
		LOCATE_HANDLE_BUFFER                   locate_handle_buffer;
		LOCATE_PROTOCOL                        locate_proto;
		UNINSTALL_MULTIPLE_PROTOCOL_INTERFACES   install_mult_proto_interfaces;
		UNINSTALL_MULTIPLE_PROTOCOL_INTERFACES uninstall_mult_proto_interfaces;
		#else //Not implemented
		void* protos_per_handle;
		void* locate_handle_buffer;
		void* locate_proto;
		void*   install_mult_proto_interfaces;
		void* uninstall_mult_proto_interfaces;
		#endif

		// 32-bit CRC
		#if 0
		CALCULATE_CRC32 calc_crc32;
		#else //Not implemented
		void* calc_crc32;
		#endif

		// Miscellaneous
		#if 0
		COPY_MEM        mem_cpy;
		SET_MEM         mem_set;
		CREATE_EVENT_EX evt_create_ex;
		#else //Not implemented
		void* mem_cpy;
		void* mem_set;
		void* evt_create_ex;
		#endif
};



//EFI System Table
//	See UEFI Standard §4.3 "EFI System Table"
//	https://uefi.org/specs/UEFI/2.10/04_EFI_System_Table.html#efi-system-table-1
class SYSTEM_TABLE final
{
	public:
		static constexpr UINT64 signature = 0x5453'5953'2049'4249ull;

	private:
		TABLE_HEADER _header;

	public:
		CHAR16* firmware_vendor  ;
		UINT32  firmware_revision;

		HANDLE                       text_in_hdl ;
		SIMPLE_TEXT_INPUT_PROTOCOL*  text_in     ;
		HANDLE                       text_out_hdl;
		SIMPLE_TEXT_OUTPUT_PROTOCOL* text_out    ;
		HANDLE                       text_err_hdl;
		SIMPLE_TEXT_OUTPUT_PROTOCOL* text_err    ;

		#if 0
		RUNTIME_SERVICES* runtime_services;
		#else //Not implemented
		void* runtime_services;
		#endif
		BOOT_SERVICES* boot_services;

		UINTN config_table_count;
		#if 0
		CONFIGURATION_TABLE* config_table;
		#else //Not implemented
		void* config_table;
		#endif
};



}
