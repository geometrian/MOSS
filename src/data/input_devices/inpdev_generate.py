import os

from inpdev_defs import InpDev_Btn, inpdev_ctrl_cmds
import inpdev_ps2sc



script_dir = os.path.dirname( os.path.abspath(__file__) )
os.chdir(script_dir)



file = open( "../../kernel/device/inpdev_defs.autogen.h", "w" )
file.write(
"""#pragma once

#include "../stdafx.h"



/* ######################################################
 * Auto-generated file; change the Python script instead!
 * ###################################################### */



// Input Device Controller Buttons

#define INPUTDEV_BTNS_APPLY(MACRO)\\
""")
for k,btn in enumerate(InpDev_Btn.btns):
	name = "\""+btn.name.replace("\\","\\\\")+"\""
	file.write(f"\tMACRO( {btn.macro_token:13}, {btn.value:3}, {name:16} )")
	if k < len(InpDev_Btn.btns)-1:
		file.write("\\")
	if btn.value in (17,49,81,116,149,175,212,228):
		file.write("\n\t\\")
	file.write("\n")

file.write(
"""
enum
{
	#define MOSS_DEF( TOKEN, IND, NAME )\\
		INPDEV_BTN_##TOKEN = IND,
	INPUTDEV_BTNS_APPLY( MOSS_DEF )
	#undef MOSS_DEF
};
typedef uint8_t INPDEV_BTN;

MOSS_ND char const* inpdev_btn_to_str( INPDEV_BTN inpdev_btn );



// Input Device Controller bytecode

enum
{
""")
for cmd in inpdev_ctrl_cmds:
	file.write(f"\tINPDEV_CMD_{cmd},\n")
file.write(
"""};
typedef uint8_t INPDEV_CMD;
""")
file.close()



file = open( "../../kernel/device/inpdev_defs.autogen.c", "w" )
file.write(
"""#include "inpdev_defs.autogen.h"



MOSS_ND char const* inpdev_btn_to_str( INPDEV_BTN inpdev_btn )
{
	// TODO: better?
	switch ( inpdev_btn )
	{
""")
for btn in InpDev_Btn.btns:
	name = "\""+btn.name.replace("\\","\\\\")+"\""
	file.write(f"\t\tcase {btn.value:3}: return {name:16};\n")
file.write("""\t\tdefault:
		return "Invalid";
	}
}
""")



file = open( "../../kernel/device/inpdev_kb_defs.autogen.h", "w" )
file.write(
"""#pragma once

//#include "../stdafx.h"

#include "inpdev_defs.autogen.h"



/* ######################################################
 * Auto-generated file; change the Python script instead!
 * ###################################################### */



// Scancode Translation Tables

#define INV INPDEV_BTN_Invalid

""")

def fixup_write_table( arr, name ):
	skip_back = 0
	while arr[-1] == inpdev_ps2sc.INV:
		skip_back += 1
		arr.pop()
	if skip_back > 0:
		less_str1 = f" - {skip_back}"
		less_str2 = f"\t// (elided {skip_back} invalid elements)\n"
	else:
		skip_back = 0
		less_str1=less_str2 = ""

	skip_front = 0
	for elem in arr:
		if elem != inpdev_ps2sc.INV: break
		skip_front += 1
	if skip_front > 4:
		arr = arr[skip_front:]
		less_str3 = f" - {skip_front}"
		less_str4 = f"\t// (elided {skip_front} invalid elements)\n"
		less_str5 = (skip_front%16)*"    " + ((skip_front%16)//4)*"   "
	else:
		skip_front = 0
		less_str3=less_str4=less_str5 = ""

	s = f"static INPDEV_BTN const {name}[ 256{less_str1}{less_str3} ] =\n{{\n{less_str4}\t{less_str5}"
	for k, elem in enumerate(arr):
		if elem == inpdev_ps2sc.INV:
			s += "INV"
		else:
			s += f"{elem:3d}"
		s += ","
		if k+1 < len(arr):
			k2 = k + skip_front%16
			if   k2%16 == 15: s+="\n\t"
			elif k2% 4 ==  3: s+="   "
	s = s[:-1]+f"\n{less_str2}}};\n"

	file.write(s)

fixup_write_table( inpdev_ps2sc.table0a, "_sc_set1_table_make1byte"  )
fixup_write_table( inpdev_ps2sc.table0b, "_sc_set1_table_break1byte" )
fixup_write_table( inpdev_ps2sc.table0c, "_sc_set1_table_make2byte"  )
fixup_write_table( inpdev_ps2sc.table0d, "_sc_set1_table_break2byte" )
file.write("\n")

fixup_write_table( inpdev_ps2sc.table1a, "_sc_set2_table_1byte" )
fixup_write_table( inpdev_ps2sc.table1b, "_sc_set2_table_2byte" )
file.write("\n")

fixup_write_table( inpdev_ps2sc.table2, "_sc_set3_table" )
file.write("\n")

file.write("//#undef INV\n")

file.close()
