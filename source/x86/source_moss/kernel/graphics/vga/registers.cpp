#include "registers.h"

#include "../../io/io.h"
#include "../../kernel.h"


namespace MOSS { namespace Graphics { namespace VGA {


Registers::RegisterBase::RegisterBase(DEB_REL_CODE(char const* name,void)) DEBUG_ONLY(: name(name)) {
	for (int i=0;i<8;++i) bits[i].reg=this;//reinterpret_cast<RegisterBase*>(0x12345678);
}

void Registers::RegisterBase::print(void) const {
	kernel->write("Register [");
	for (int i=0;i<4;++i) kernel->write("%d ",bits[i].value);
	for (int i=4;i<8;++i) kernel->write(" %d",bits[i].value);
	kernel->write("] (0x%p",this);
	#ifdef MOSS_DEBUG
	kernel->write(" \"%s Register\"",name);
	#endif
	kernel->write(")\n");
	/*for (int i=0;i<8;++i) {
		kernel->write("  bit %d (0x%p) parent reg 0x%p\n",i,bits+i,bits[i].reg);
	}*/
}


void Registers::RegisterExternalBase:: read(void) /*override*/ {
	uint8_t value = IO::recv<uint8_t>(_port_read);
	for (int i=0;i<8;++i) bits[i].value=static_cast<bool>(value&(1<<i));
}
void Registers::RegisterExternalBase::write(void) /*override*/ {
	#ifdef MOSS_DEBUG
	if (_port_write==0x0000) {
		print();
		assert_term(false,"Implementation error!  Cannot write read-only register!");
	}
	#endif
	uint8_t value=0; for (int i=0;i<8;++i) if (bits[i].value) value|=(1<<i);
	IO::send<uint8_t>(_port_write, value);
}

//1. Input the value of the Address Register and save it for step 6
//2. Output the index of the desired Data Register to the Address Register.
//3. Read the value of the Data Register and save it for later restoration upon termination, if needed.
//4. If writing, modify the value read in step 3, making sure to mask off bits not being modified.
//5. If writing, write the new value from step 4 to the Data register.
//6. Write the value of Address register saved in step 1 to the Address Register.
void Registers::RegisterInternalType1Base:: read(void) /*override*/ {
	uint8_t old_addr_reg_value = IO::recv<uint8_t>(_port_addr);
	IO::send<uint8_t>(_port_addr, _index);

	uint8_t value = IO::recv<uint8_t>(_port_data);
	for (int i=0;i<8;++i) bits[i].value=static_cast<bool>(value&(1<<i));

	IO::send<uint8_t>(_port_addr,old_addr_reg_value);
}
void Registers::RegisterInternalType1Base::write(void) /*override*/ {
	uint8_t old_addr_reg_value = IO::recv<uint8_t>(_port_addr);
	IO::send<uint8_t>(_port_addr, _index);

	//uint8_t old_data_reg_value = IO::recv<uint8_t>(_port_data);
	uint8_t value=0; for (int i=0;i<8;++i) if (bits[i].value) value|=(1<<i);
	IO::send<uint8_t>(_port_data,value);

	IO::send<uint8_t>(_port_addr,old_addr_reg_value);
}

//1. Input a value from the Input Status #1 Register (normally port 0x03DA) and discard it.
//2. Read the value of the Address/Data Register and save it for step 7.
//3. Output the index of the desired Data Register to the Address/Data Register
//4. Read the value of the Data Register and save it for later restoration upon termination, if needed.
//5. If writing, modify the value read in step 4, making sure to mask off bits not being modified.
//6. If writing, write the new value from step 5 to the Address/Data register.
//7. Write the value of Address register saved in step 1 to the Address/Data Register.
//8. If you wish to leave the register waiting for an index, input a value from the Input Status #1 Register (normally port 3DAh) and discard it.
void Registers::RegisterInternalType2Base:: read(void) /*override*/ {
	IO::recv<uint8_t>(0x03DA); //TODO: better value?

	uint8_t old_addr_reg_value = IO::recv<uint8_t>(MOSS_VGA_ADDR_AC);
	IO::send<uint8_t>(MOSS_VGA_ADDR_AC, _index);

	uint8_t value = IO::recv<uint8_t>(MOSS_VGA_DATAR_AC);
	for (int i=0;i<8;++i) bits[i].value=static_cast<bool>(value&(1<<i));

	IO::send<uint8_t>(MOSS_VGA_ADDR_AC,old_addr_reg_value);
}
void Registers::RegisterInternalType2Base::write(void) /*override*/ {
	IO::recv<uint8_t>(0x03DA); //TODO: better value?

	uint8_t old_addr_reg_value = IO::recv<uint8_t>(MOSS_VGA_ADDR_AC);
	IO::send<uint8_t>(MOSS_VGA_ADDR_AC, _index);

	//uint8_t old_data_reg_value = IO::recv<uint8_t>(MOSS_VGA_DATAR_AC);
	uint8_t value=0; for (int i=0;i<8;++i) if (bits[i].value) value|=(1<<i);
	IO::send<uint8_t>(MOSS_VGA_DATAW_AC,value);

	IO::send<uint8_t>(MOSS_VGA_ADDR_AC,old_addr_reg_value);
}


void Registers:: read_all_registers(void) {
	read_all_registers_external();
	read_all_registers_internal_sequencer();
	read_all_registers_internal_gc();
	read_all_registers_internal_crtc();
}
void Registers::write_all_registers(void) {
	write_all_registers_external();
	write_all_registers_internal_sequencer();
	write_all_registers_internal_gc();
	write_all_registers_internal_crtc();
}

void Registers:: read_all_registers_external(void) {
	miscellaneous_output_reg.read();
	input_status0_reg.read();
	input_status1_reg.read();
}
void Registers::write_all_registers_external(void) {
	miscellaneous_output_reg.write();
	//Note: the input status registers are read-only
}

void Registers:: read_all_registers_internal_sequencer(void) {
	sequencer_clocking_mode_reg.read();
	sequencer_map_mask_reg.read();
	sequencer_memory_mode_reg.read();
}
void Registers::write_all_registers_internal_sequencer(void) {
	sequencer_clocking_mode_reg.write();
	sequencer_map_mask_reg.write();
	sequencer_memory_mode_reg.write();
}

void Registers:: read_all_registers_internal_gc(void) {
	gc_read_map_select_reg.read();
	gc_mode_reg.read();
	gc_misc_reg.read();
}
void Registers::write_all_registers_internal_gc(void) {
	gc_read_map_select_reg.write();
	gc_mode_reg.write();
	gc_misc_reg.write();
}

void Registers:: read_all_registers_internal_crtc(void) {
	#define MOSS_CRTC_REG_READ(NAME,name,OFFSET) crtc_##name##_reg.read();
	MOSS_CRTC_REG_MACRO(MOSS_CRTC_REG_READ)
	#undef MOSS_CRTC_REG_READ
}
void Registers::write_all_registers_internal_crtc(void) {
	#define MOSS_CRTC_REG_READ(NAME,name,OFFSET) crtc_##name##_reg.write();
	MOSS_CRTC_REG_MACRO(MOSS_CRTC_REG_READ)
	#undef MOSS_CRTC_REG_READ
}


}}}