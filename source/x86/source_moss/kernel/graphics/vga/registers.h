#pragma once

#include "../../../includes.h"


namespace MOSS { namespace Graphics { namespace VGA {


//The VGA has many registers.
//	The external registers have their own IO ports.
//	The other registers do not.  I don't know of an official name for them, so I'll call them internal registers.
//		These must be indexed into groups: an address register for the group, and a data register for the group
//		(an offset within the register group):
//			Internal registers type 1: Sequencer, Graphics, and CRT Controller Registers
//				The Sequencer has 5 indexed registers
//				The Graphics Controller has 9 indexed registers
//				The CRTC has 19 indexed registers (although some say 25?)
//			Internal registers type 2: Attribute Registers (some claim there are 21)
				#define MOSS_VGA_ADDR_AC       0x03C0
				#define MOSS_VGA_DATAR_AC      0x03C1
				#define MOSS_VGA_DATAW_AC      0x03C0
//			Internal registers type 3: Color Registers
				#define MOSS_VGA_STATE_DAC     0x03C7 //DAC State Register
				#define MOSS_VGA_ADDRR_PEL     0x03C7 //PEL Address Read Mode Register
				#define MOSS_VGA_ADDRW_PEL     0x03C8 //PEL Address Write Mode Register
				#define MOSS_VGA_DATA_PEL      0x03C9 //PEL Data Register
class Registers final {
	public:
		//Register Base Types
		#if 1
		class RegisterBase {
			public:
				class Bit final { public:
					RegisterBase* reg;
					bool value;
				} bits[8];

				#ifdef MOSS_DEBUG
				char const*const name;
				#endif

			protected:
				DEBUG_ONLY(explicit) RegisterBase(DEB_REL_CODE(char const* name,void));
			public:
				inline virtual ~RegisterBase(void) {}

				virtual void  read(void) = 0;
				virtual void write(void) = 0;

				void print(void) const;
		};

		class RegisterExternalBase : public RegisterBase {
			private:
				uint16_t const _port_read, _port_write;

			protected:
				inline RegisterExternalBase(DEBUG_ONLY(char const* name COMMA) uint16_t port_read, uint16_t port_write) :
					RegisterBase(DEBUG_ONLY(name)), _port_read(port_read),_port_write(port_write)
				{}
			public:
				inline virtual ~RegisterExternalBase(void) {}

				void  read(void) override;
				void write(void) override;
		};
		class RegisterInternalBase : public RegisterBase {
			protected:
				uint16_t const _port_addr, _port_data;
				uint8_t const _index;

			protected:
				inline RegisterInternalBase(DEBUG_ONLY(char const* name COMMA) uint16_t port_addr, uint16_t port_data, uint8_t index) :
					RegisterBase(DEBUG_ONLY(name)), _port_addr(port_addr),_port_data(port_data),_index(index)
				{}
			public:
				inline virtual ~RegisterInternalBase(void) {}
		};

		class RegisterInternalType1Base : public RegisterInternalBase {
			protected:
				inline RegisterInternalType1Base(DEBUG_ONLY(char const* name COMMA) uint16_t port_addr, uint16_t port_data, uint8_t index) :
					RegisterInternalBase(DEBUG_ONLY(name COMMA) port_addr,port_data,index)
				{}
			public:
				inline virtual ~RegisterInternalType1Base(void) {}

				void  read(void) override;
				void write(void) override;
		};
		class RegisterInternalType2Base : public RegisterInternalBase {
			protected:
				inline RegisterInternalType2Base(DEBUG_ONLY(char const* name COMMA) uint16_t port_addr, uint16_t port_data, uint8_t index) :
					RegisterInternalBase(DEBUG_ONLY(name COMMA) port_addr,port_data,index)
				{}
			public:
				inline virtual ~RegisterInternalType2Base(void) {}

				void  read(void) override;
				void write(void) override;
		};
		#endif

		//External registers
		#if 1
		//	Miscellaneous Output Register
		class RegisterMiscellaneousOutput final : public RegisterExternalBase { public:
			inline RegisterMiscellaneousOutput(void) : RegisterExternalBase(DEBUG_ONLY("MiscellaneousOutput" COMMA) 0x03CC,0x03C2) {}
			inline virtual ~RegisterMiscellaneousOutput(void) {}
		} miscellaneous_output_reg;
		//	Feature Control Register
		//	Input Status #0 Register
		class RegisterInputStatus0 final : public RegisterExternalBase { public: //Note: read-only
			inline RegisterInputStatus0(void) : RegisterExternalBase(DEBUG_ONLY("InputStatus0" COMMA) 0x03C2,0x0000) {}
			inline virtual ~RegisterInputStatus0(void) {}
		} input_status0_reg;
		//	Input Status #1 Register
		class RegisterInputStatus1 final : public RegisterExternalBase { public: //Note: read-only
			inline RegisterInputStatus1(void) : RegisterExternalBase(DEBUG_ONLY("InputStatus1" COMMA) 0x03DA,0x0000) {} //0x03BA for mono
			inline virtual ~RegisterInputStatus1(void) {}
		} input_status1_reg;
		#endif

		//Internal registers
		#if 1
		//	Sequencer
		//		Reset Register
		//		Clocking Mode Register
		class RegisterSequencerClockingMode final : public RegisterInternalType1Base { public:
			inline RegisterSequencerClockingMode(void) : RegisterInternalType1Base(DEBUG_ONLY("ClockingMode" COMMA) 0x03C4,0x03C5,0x01) {}
			inline virtual ~RegisterSequencerClockingMode(void) {}
		} sequencer_clocking_mode_reg;
		//		Map Mask Register
		class RegisterSequencerMapMask final : public RegisterInternalType1Base { public:
			inline RegisterSequencerMapMask(void) : RegisterInternalType1Base(DEBUG_ONLY("MapMask" COMMA) 0x03C4,0x03C5,0x02) {}
			inline virtual ~RegisterSequencerMapMask(void) {}
		} sequencer_map_mask_reg;
		//		Character Map Select Register
		//		Sequencer Memory Mode Register
		class RegisterSequencerMemoryMode final : public RegisterInternalType1Base { public:
			inline RegisterSequencerMemoryMode(void) : RegisterInternalType1Base(DEBUG_ONLY("MemoryMode" COMMA) 0x03C4,0x03C5,0x04) {}
			inline virtual ~RegisterSequencerMemoryMode(void) {}
		} sequencer_memory_mode_reg;
		//	Graphics Controller
		//		Set/Reset Register
		//		Enable Set/Reset Register
		//		Color Compare Register
		//		Data Rotate Register
		//		Read Map Select Register
		class RegisterGCReadMapSelect final : public RegisterInternalType1Base { public:
			inline RegisterGCReadMapSelect(void) : RegisterInternalType1Base(DEBUG_ONLY("ReadMapSelect" COMMA) 0x03CE,0x03CF,0x04) {}
			inline virtual ~RegisterGCReadMapSelect(void) {}
		} gc_read_map_select_reg;
		//		Graphics Mode Register
		class RegisterGCMode final : public RegisterInternalType1Base { public:
			inline RegisterGCMode(void) : RegisterInternalType1Base(DEBUG_ONLY("Mode" COMMA) 0x03CE,0x03CF,0x04) {}
			inline virtual ~RegisterGCMode(void) {}
		} gc_mode_reg;
		//		Miscellaneous Graphics Register
		class RegisterGCMiscellaneous final : public RegisterInternalType1Base { public:
			inline RegisterGCMiscellaneous(void) : RegisterInternalType1Base(DEBUG_ONLY("Mode" COMMA) 0x03CE,0x03CF,0x04) {}
			inline virtual ~RegisterGCMiscellaneous(void) {}
		} gc_misc_reg;
		//		Color Don't Care Register
		//		Bit Mask Register
		//	Cathode Ray Tube Controller
		#define MOSS_CRTC_REG_DEC(NAME,name,OFFSET)\
			class RegisterCRTC##NAME final : public RegisterInternalType1Base { public:\
				inline RegisterCRTC##NAME(void) : RegisterInternalType1Base(DEBUG_ONLY(#NAME COMMA) 0x03D4,0x03D5,OFFSET) {} /*0x03B4,0x03B5 for mono*/\
				inline virtual ~RegisterCRTC##NAME(void) {}\
			} crtc_##name##_reg;
		#define MOSS_CRTC_REG_MACRO(MACRO)\
			/*Horizontal Total Register*/\
			MACRO(HorizontalTotal,horizontal_total,0x00)\
			/*End Horizontal Display Register*/\
			MACRO(EndHorizontalDisplay,end_horizontal_display,0x01)\
			/*Start Horizontal Blanking Register*/\
			MACRO(StartHorizontalBlanking,start_horizontal_blanking,0x02)\
			/*End Horizontal Blanking Register*/\
			MACRO(EndHorizontalBlanking,end_horizontal_blanking,0x03)\
			/*Start Horizontal Retrace Register*/\
			MACRO(StartHorizontalRetrace,start_horizontal_retrace,0x04)\
			/*End Horizontal Retrace Register*/\
			MACRO(EndHorizontalRetrace,end_horizontal_retrace,0x05)\
			/*Vertical Total Register*/\
			MACRO(VerticalTotal,vertical_total,0x06)\
			/*Overflow Register*/\
			MACRO(Overflow,overflow,0x07)\
			/*Preset Row Scan Register*/\
			/*Maximum Scan Line Register*/\
			MACRO(MaximumScanLine,maximum_scan_line,0x09)\
			/*Cursor Start Register*/\
			/*Cursor End Register*/\
			/*Start Address High Register*/\
			/*Start Address Low Register*/\
			/*Cursor Location High Register*/\
			/*Cursor Location Low Register*/\
			/*Vertical Retrace Start Register*/\
			MACRO(VerticalRetraceStart,vertical_retrace_start,0x10)\
			/*Vertical Retrace End Register*/\
			MACRO(VerticalRetraceEnd,vertical_retrace_end,0x11)\
			/*Vertical Display End Register*/\
			MACRO(VerticalDisplayEnd,vertical_display_end,0x12)\
			/*Offset Register*/\
			MACRO(Offset,offset,0x13)\
			/*Underline Location Register*/\
			/*Start Vertical Blanking Register*/\
			MACRO(StartVerticalBlanking,start_vertical_blanking,0x15)\
			/*End Vertical Blanking*/\
			MACRO(EndVerticalBlanking,end_vertical_blanking,0x16)\
			/*CRTC Mode Control Register*/\
			/*Line Compare Register*/\
			MACRO(LineCompare,line_compare,0x18)
		MOSS_CRTC_REG_MACRO(MOSS_CRTC_REG_DEC)
		#endif

	public:
		void  read_all_registers(void);
		void write_all_registers(void);

		void  read_all_registers_external(void);
		void write_all_registers_external(void);

		void  read_all_registers_internal_sequencer(void);
		void write_all_registers_internal_sequencer(void);

		void  read_all_registers_internal_gc(void);
		void write_all_registers_internal_gc(void);

		void  read_all_registers_internal_crtc(void);
		void write_all_registers_internal_crtc(void);
};


}}}