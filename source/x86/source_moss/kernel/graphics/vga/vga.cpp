#include "vga.h"

#include "../../io/io.h"


namespace MOSS { namespace Graphics { namespace VGA {


uint8_t  read_internal_register_type1(uint16_t addr_reg,uint16_t data_reg, uint8_t data_reg_index) {
	uint8_t old_addr_reg_value = IO::recv<uint8_t>(addr_reg);
	IO::send<uint8_t>(addr_reg, data_reg_index);

	uint8_t data_reg_value = IO::recv<uint8_t>(data_reg);

	IO::send<uint8_t>(addr_reg,old_addr_reg_value);

	return data_reg_value;
}
void    write_internal_register_type1(uint16_t addr_reg,uint16_t data_reg, uint8_t data_reg_index,uint8_t data_reg_value) {
	uint8_t old_addr_reg_value = IO::recv<uint8_t>(addr_reg);
	IO::send<uint8_t>(addr_reg, data_reg_index);

	uint8_t old_data_reg_value = IO::recv<uint8_t>(data_reg);
	//Modify the value, making sure to mask off bits not being modified.
	old_data_reg_value = data_reg_value;
	IO::send<uint8_t>(data_reg,old_data_reg_value);

	IO::send<uint8_t>(addr_reg,old_addr_reg_value);
}

uint8_t  read_internal_register_type2(uint8_t data_reg_index) {
	IO::recv<uint8_t>(0x03DA); //TODO: better value?

	uint8_t old_addr_reg_value = IO::recv<uint8_t>(MOSS_VGA_ADDR_AC);
	IO::send<uint8_t>(MOSS_VGA_ADDR_AC, data_reg_index);

	uint8_t data_reg_value = IO::recv<uint8_t>(MOSS_VGA_DATAR_AC);

	IO::send<uint8_t>(MOSS_VGA_ADDR_AC,old_addr_reg_value);

	return data_reg_value;
}
void    write_internal_register_type2(uint8_t data_reg_index,uint8_t data_reg_value) {
	IO::recv<uint8_t>(0x03DA); //TODO: better value?

	uint8_t old_addr_reg_value = IO::recv<uint8_t>(MOSS_VGA_ADDR_AC);
	IO::send<uint8_t>(MOSS_VGA_ADDR_AC, data_reg_index);

	uint8_t old_data_reg_value = IO::recv<uint8_t>(MOSS_VGA_DATAR_AC);
	//Modify the value, making sure to mask off bits not being modified.
	old_data_reg_value = data_reg_value;
	IO::send<uint8_t>(MOSS_VGA_DATAW_AC,old_data_reg_value);

	IO::send<uint8_t>(MOSS_VGA_ADDR_AC,old_addr_reg_value);
}


}}}