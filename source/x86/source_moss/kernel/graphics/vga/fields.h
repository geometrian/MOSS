#pragma once

#include "../../../includes.h"

#include "registers.h"


namespace MOSS { namespace Graphics { namespace VGA {


class Fields final {
	public:
		//Type
		#if 1
		template <int width> class FieldBase {
			public:
				Registers::RegisterBase::Bit* bit_pointers[width]; //Note not restrict; sizeof(bool) isn't a bit.

				#ifdef MOSS_DEBUG
				char const*const name;
				#endif

			private:
				template <int i, typename TypeReg                          > inline void _helper(TypeReg* reg,int index                       ) {
					bit_pointers[i] = reg->bits + index;
				}
				template <int i, typename TypeReg,typename... TypesIndsRegs> inline void _helper(TypeReg* reg,int index, TypesIndsRegs... regs) {
					bit_pointers[i] = reg->bits + index;
					_helper<i+1>(regs...);
				}
			public:
				template <typename... TypesIndsRegs> inline FieldBase(DEBUG_ONLY(char const* name COMMA) TypesIndsRegs... regs) DEBUG_ONLY(: name(name)) {
					static_assert(sizeof...(TypesIndsRegs)==2*width,"Incorrect number of register backings!");
					_helper<0>(regs...);
				}
				inline virtual ~FieldBase(void) {}

				//Load all registers this field requires.  TODO: prevent redundant loads.
				void load_regs(void);
				//Save all registers this field touches.  TODO: prevent redundant stores.
				void save_regs(void);

				template <typename type> type get_as(void) const {
					type result = 0;
					for (int i=0;i<width;++i) if (bit_pointers[i]->_value) result|=1<<i;
					return result;
				}

				template <typename type> inline void operator=(type other) { for (int i=0;i<width;++i) bit_pointers[i]->_value=static_cast<bool>(other&(1<<i)); }
				template <typename type> inline type operator|(type other) const { return get_as<type>()|other; }
				template <typename type> inline type operator&(type other) const { return get_as<type>()&other; }
				template <typename type> inline void operator|=(type other) { for (int i=0;i<width;++i) bit_pointers[i]->_value|=static_cast<bool>(other&(1<<i)); }
				template <typename type> inline void operator&=(type other) { for (int i=0;i<width;++i) bit_pointers[i]->_value&=static_cast<bool>(other&(1<<i)); }

				void print(void) const;
		};
		#endif

		//Subclasses
		#if 1
		#define MOSS_FIELD_DEF_OPERATORS(WIDTH,TYPE)\
			inline void operator=(TYPE other) { FieldBase<WIDTH>::operator=(other); }\
			inline TYPE operator|(TYPE other) const { return FieldBase<WIDTH>::operator|(other); }\
			inline TYPE operator&(TYPE other) const { return FieldBase<WIDTH>::operator&(other); }\
			inline void operator|=(TYPE other) { FieldBase<WIDTH>::operator|=(other); }\
			inline void operator&=(TYPE other) { FieldBase<WIDTH>::operator&=(other); }
		#define MOSS_FIELD1_DEC(NAME, R0,I0)\
			class Field##NAME final : public FieldBase<1> { public:\
				inline explicit Field##NAME(Registers* regs) : FieldBase<1>(DEBUG_ONLY(#NAME COMMA) &regs->R0##_reg,I0) {} inline ~Field##NAME(void) {}\
				MOSS_FIELD_DEF_OPERATORS(1,bool)\
			} NAME;
		#define MOSS_FIELD2_DEC(NAME, R0,I0,R1,I1)\
			class Field##NAME final : public FieldBase<2> { public:\
				inline explicit Field##NAME(Registers* regs) : FieldBase<2>(DEBUG_ONLY(#NAME COMMA) &regs->R0##_reg,I0,&regs->R1##_reg,I1) {} inline ~Field##NAME(void) {}\
				MOSS_FIELD_DEF_OPERATORS(2,uint8_t)\
			} NAME;
		#define MOSS_FIELD3_DEC(NAME, R0,I0,R1,I1,R2,I2)\
			class Field##NAME final : public FieldBase<3> { public:\
				inline explicit Field##NAME(Registers* regs) : FieldBase<3>(DEBUG_ONLY(#NAME COMMA) &regs->R0##_reg,I0,&regs->R1##_reg,I1,&regs->R2##_reg,I2) {} inline ~Field##NAME(void) {}\
				MOSS_FIELD_DEF_OPERATORS(3,uint8_t)\
			} NAME;
		#define MOSS_FIELD4_DEC(NAME, R0,I0,R1,I1,R2,I2,R3,I3)\
			class Field##NAME final : public FieldBase<4> { public:\
				inline explicit Field##NAME(Registers* regs) : FieldBase<4>(DEBUG_ONLY(#NAME COMMA) &regs->R0##_reg,I0,&regs->R1##_reg,I1,&regs->R2##_reg,I2,&regs->R3##_reg,I3) {} inline ~Field##NAME(void) {}\
				MOSS_FIELD_DEF_OPERATORS(4,uint8_t)\
			} NAME;
		#define MOSS_FIELD5_DEC(NAME, R0,I0,R1,I1,R2,I2,R3,I3,R4,I4)\
			class Field##NAME final : public FieldBase<5> { public:\
				inline explicit Field##NAME(Registers* regs) : FieldBase<5>(DEBUG_ONLY(#NAME COMMA) &regs->R0##_reg,I0,&regs->R1##_reg,I1,&regs->R2##_reg,I2,&regs->R3##_reg,I3,&regs->R4##_reg,I4) {} inline ~Field##NAME(void) {}\
				MOSS_FIELD_DEF_OPERATORS(5,uint8_t)\
			} NAME;
		#define MOSS_FIELD6_DEC(NAME, R0,I0,R1,I1,R2,I2,R3,I3,R4,I4,R5,I5)\
			class Field##NAME final : public FieldBase<6> { public:\
				inline explicit Field##NAME(Registers* regs) : FieldBase<6>(DEBUG_ONLY(#NAME COMMA) &regs->R0##_reg,I0,&regs->R1##_reg,I1,&regs->R2##_reg,I2,&regs->R3##_reg,I3,&regs->R4##_reg,I4,&regs->R5##_reg,I5) {} inline ~Field##NAME(void) {}\
				MOSS_FIELD_DEF_OPERATORS(6,uint8_t)\
			} NAME;
		#define MOSS_FIELD7_DEC(NAME, R0,I0,R1,I1,R2,I2,R3,I3,R4,I4,R5,I5,R6,I6)\
			class Field##NAME final : public FieldBase<7> { public:\
				inline explicit Field##NAME(Registers* regs) : FieldBase<7>(DEBUG_ONLY(#NAME COMMA) &regs->R0##_reg,I0,&regs->R1##_reg,I1,&regs->R2##_reg,I2,&regs->R3##_reg,I3,&regs->R4##_reg,I4,&regs->R5##_reg,I5,&regs->R6##_reg,I6) {} inline ~Field##NAME(void) {}\
				MOSS_FIELD_DEF_OPERATORS(7,uint8_t)\
			} NAME;
		#define MOSS_FIELD8_DEC(NAME, R0,I0,R1,I1,R2,I2,R3,I3,R4,I4,R5,I5,R6,I6,R7,I7)\
			class Field##NAME final : public FieldBase<8> { public:\
				inline explicit Field##NAME(Registers* regs) : FieldBase<8>(DEBUG_ONLY(#NAME COMMA) &regs->R0##_reg,I0,&regs->R1##_reg,I1,&regs->R2##_reg,I2,&regs->R3##_reg,I3,&regs->R4##_reg,I4,&regs->R5##_reg,I5,&regs->R6##_reg,I6,&regs->R7##_reg,I7) {} inline ~Field##NAME(void) {}\
				MOSS_FIELD_DEF_OPERATORS(8,uint8_t)\
			} NAME;
		#define MOSS_FIELD9_DEC(NAME, R0,I0,R1,I1,R2,I2,R3,I3,R4,I4,R5,I5,R6,I6,R7,I7,R8,I8)\
			class Field##NAME final : public FieldBase<9> { public:\
				inline explicit Field##NAME(Registers* regs) : FieldBase<9>(DEBUG_ONLY(#NAME COMMA) &regs->R0##_reg,I0,&regs->R1##_reg,I1,&regs->R2##_reg,I2,&regs->R3##_reg,I3,&regs->R4##_reg,I4,&regs->R5##_reg,I5,&regs->R6##_reg,I6,&regs->R7##_reg,I7,&regs->R8##_reg,I8) {} inline ~Field##NAME(void) {}\
				MOSS_FIELD_DEF_OPERATORS(9,uint16_t)\
			} NAME;
		#define MOSS_FIELD10_DEC(NAME, R0,I0,R1,I1,R2,I2,R3,I3,R4,I4,R5,I5,R6,I6,R7,I7,R8,I8,R9,I9)\
			class Field##NAME final : public FieldBase<10> { public:\
				inline explicit Field##NAME(Registers* regs) : FieldBase<10>(DEBUG_ONLY(#NAME COMMA) &regs->R0##_reg,I0,&regs->R1##_reg,I1,&regs->R2##_reg,I2,&regs->R3##_reg,I3,&regs->R4##_reg,I4,&regs->R5##_reg,I5,&regs->R6##_reg,I6,&regs->R7##_reg,I7,&regs->R8##_reg,I8,&regs->R9##_reg,I9) {} inline ~Field##NAME(void) {}\
				MOSS_FIELD_DEF_OPERATORS(10,uint16_t)\
			} NAME;
		#define MOSS_FIELDS(MACRO1,MACRO2,MACRO3,MACRO4,MACRO5,MACRO6,MACRO7,MACRO8,MACRO9,MACRO10,MACRO_INBETWEEN)\
			/*Horizontal*/\
			MACRO8(HorizontalTotal,\
				crtc_horizontal_total,0, crtc_horizontal_total,1, crtc_horizontal_total,2, crtc_horizontal_total,3,\
				crtc_horizontal_total,4, crtc_horizontal_total,5, crtc_horizontal_total,6, crtc_horizontal_total,7\
			) MACRO_INBETWEEN\
			MACRO8(HorizontalDisplayEnd,\
				crtc_end_horizontal_display,0, crtc_end_horizontal_display,1, crtc_end_horizontal_display,2, crtc_end_horizontal_display,3,\
				crtc_end_horizontal_display,4, crtc_end_horizontal_display,5, crtc_end_horizontal_display,6, crtc_end_horizontal_display,7\
			) MACRO_INBETWEEN\
			MACRO8(HorizontalBlankingStart,\
				crtc_start_horizontal_blanking,0, crtc_start_horizontal_blanking,1, crtc_start_horizontal_blanking,2, crtc_start_horizontal_blanking,3,\
				crtc_start_horizontal_blanking,4, crtc_start_horizontal_blanking,5, crtc_start_horizontal_blanking,6, crtc_start_horizontal_blanking,7\
			) MACRO_INBETWEEN\
			/*MACRO2(DisplayEnableSkew,\
				crtc_end_horizontal_blanking,5, crtc_end_horizontal_blanking,6\
			) MACRO_INBETWEEN*/\
			MACRO8(Offset,\
				crtc_offset,0, crtc_offset,1, crtc_offset,2, crtc_offset,3,\
				crtc_offset,4, crtc_offset,5, crtc_offset,6, crtc_offset,7\
			) MACRO_INBETWEEN\
			MACRO6(HorizontalBlankingEnd,\
				crtc_end_horizontal_blanking,0, crtc_end_horizontal_blanking,1, crtc_end_horizontal_blanking,2, crtc_end_horizontal_blanking,3,\
				crtc_end_horizontal_blanking,4, crtc_end_horizontal_retrace,7\
			) MACRO_INBETWEEN\
			MACRO8(HorizontalRetraceStart,\
				crtc_start_horizontal_retrace,0, crtc_start_horizontal_retrace,1, crtc_start_horizontal_retrace,2, crtc_start_horizontal_retrace,3,\
				crtc_start_horizontal_retrace,4, crtc_start_horizontal_retrace,5, crtc_start_horizontal_retrace,6, crtc_start_horizontal_retrace,7\
			) MACRO_INBETWEEN\
			MACRO5(HorizontalRetraceEnd,\
				crtc_end_horizontal_retrace,0, crtc_end_horizontal_retrace,1, crtc_end_horizontal_retrace,2, crtc_end_horizontal_retrace,3,\
				crtc_end_horizontal_retrace,4\
			) MACRO_INBETWEEN\
			/*Vertical*/\
			MACRO10(VerticalTotal,\
				crtc_start_horizontal_retrace,0, crtc_start_horizontal_retrace,1, crtc_start_horizontal_retrace,2, crtc_start_horizontal_retrace,3,\
				crtc_start_horizontal_retrace,4, crtc_start_horizontal_retrace,5, crtc_start_horizontal_retrace,6, crtc_start_horizontal_retrace,7,\
				crtc_overflow,0, crtc_overflow,5\
			) MACRO_INBETWEEN\
			MACRO10(VerticalRetraceStart,\
				crtc_vertical_retrace_start,0, crtc_vertical_retrace_start,1, crtc_vertical_retrace_start,2, crtc_vertical_retrace_start,3,\
				crtc_vertical_retrace_start,4, crtc_vertical_retrace_start,5, crtc_vertical_retrace_start,6, crtc_vertical_retrace_start,7,\
				crtc_overflow,2, crtc_overflow,7\
			) MACRO_INBETWEEN\
			MACRO4(VerticalRetraceEnd,\
				crtc_vertical_retrace_end,0, crtc_vertical_retrace_end,1, crtc_vertical_retrace_end,2, crtc_vertical_retrace_end,3\
			) MACRO_INBETWEEN\
			MACRO10(VerticalDisplayEnd,\
				crtc_vertical_display_end,0, crtc_vertical_display_end,1, crtc_vertical_display_end,2, crtc_vertical_display_end,3,\
				crtc_vertical_display_end,4, crtc_vertical_display_end,5, crtc_vertical_display_end,6, crtc_vertical_display_end,7,\
				crtc_overflow,1, crtc_overflow,6\
			) MACRO_INBETWEEN\
			MACRO10(VerticalBlankingStart,\
				crtc_start_vertical_blanking,0, crtc_start_vertical_blanking,1, crtc_start_vertical_blanking,2, crtc_start_vertical_blanking,3,\
				crtc_start_vertical_blanking,4, crtc_start_vertical_blanking,5, crtc_start_vertical_blanking,6, crtc_start_vertical_blanking,7,\
				crtc_overflow,3, crtc_maximum_scan_line,5\
			) MACRO_INBETWEEN\
			MACRO7(VerticalBlankingEnd,\
				crtc_end_vertical_blanking,0, crtc_end_vertical_blanking,1, crtc_end_vertical_blanking,2, crtc_end_vertical_blanking,3,\
				crtc_end_vertical_blanking,4, crtc_end_vertical_blanking,5, crtc_end_vertical_blanking,6\
			) MACRO_INBETWEEN\
			/*Misc.*/\
			MACRO1(ProtectCRTC,\
				crtc_vertical_retrace_end,7\
			) MACRO_INBETWEEN\
			MACRO10(LineCompare,\
				crtc_line_compare,0, crtc_line_compare,1, crtc_line_compare,2, crtc_line_compare,3,\
				crtc_line_compare,4, crtc_line_compare,5, crtc_line_compare,6, crtc_line_compare,7,\
				crtc_overflow,4, crtc_maximum_scan_line,6\
			) MACRO_INBETWEEN\
			MACRO2(ClockSelect,\
				miscellaneous_output,2, miscellaneous_output,3\
			) MACRO_INBETWEEN\
			MACRO1(DotMode,\
				sequencer_clocking_mode,0\
			) MACRO_INBETWEEN\
			MACRO2(ReadMapSelect,\
				gc_read_map_select,0, gc_read_map_select,1\
			) MACRO_INBETWEEN\
			MACRO4(WritePlaneEnable,\
				sequencer_map_mask,0, sequencer_map_mask,1, sequencer_map_mask,2, sequencer_map_mask,3\
			) MACRO_INBETWEEN\
			MACRO5(MaximumScanLine, /*of a character*/\
				crtc_maximum_scan_line,0, crtc_maximum_scan_line,1, crtc_maximum_scan_line,2, crtc_maximum_scan_line,3, crtc_maximum_scan_line,4\
			) MACRO_INBETWEEN\
			MACRO3(OEHostMem,\
				/*This is actually three different fields, arranged in order:*/\
				/*	Odd/Even Host Memory Write Addressing Disable*/\
				/*	Odd/Even Host Memory Read Addressing Disable*/\
				/*	Chain Odd/Even Enable*/\
				/*Set to 0b110 to disable odd/even and set to 0b001 to enable.*/\
				sequencer_memory_mode,2, gc_mode,5, gc_misc,1\
			)
		MOSS_FIELDS(MOSS_FIELD1_DEC,MOSS_FIELD2_DEC,MOSS_FIELD3_DEC,MOSS_FIELD4_DEC,MOSS_FIELD5_DEC,MOSS_FIELD6_DEC,MOSS_FIELD7_DEC,MOSS_FIELD8_DEC,MOSS_FIELD9_DEC,MOSS_FIELD10_DEC,NOTHING)
		#endif

	public:
		explicit Fields(Registers* regs) :
			#define MOSS_FIELD_CONSTRUCT(NAME,...) NAME(regs)
			MOSS_FIELDS(MOSS_FIELD_CONSTRUCT,MOSS_FIELD_CONSTRUCT,MOSS_FIELD_CONSTRUCT,MOSS_FIELD_CONSTRUCT,MOSS_FIELD_CONSTRUCT,MOSS_FIELD_CONSTRUCT,MOSS_FIELD_CONSTRUCT,MOSS_FIELD_CONSTRUCT,MOSS_FIELD_CONSTRUCT,MOSS_FIELD_CONSTRUCT,COMMA)
		{}
		inline ~Fields(void) = default;
};


}}}