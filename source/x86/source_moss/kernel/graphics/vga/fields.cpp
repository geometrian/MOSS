#include "fields.h"

#include "../../kernel.h"

#include "registers.h"


namespace MOSS { namespace Graphics { namespace VGA {


template <int width> void Fields::FieldBase<width>::load_regs(void) {
	for (int i=0;i<width;++i) bit_pointers[i]->reg->read();
}

template <int width> void Fields::FieldBase<width>::save_regs(void) {
	//kernel->write("Saving field\n"); print(); while(1);
	for (int i=0;i<width;++i) bit_pointers[i]->reg->write();
}

template <int width> void Fields::FieldBase<width>::print(void) const {
	kernel->write("Field (length %d",width);
	#ifdef MOSS_DEBUG
	kernel->write(" \"%s\"",name);
	#endif
	kernel->write("):\n");

	for (int i=0;i<width;++i) {
		kernel->write("  bit %d -> 0x%p (bit %d) within reg 0x%p\n",i,
			bit_pointers[i], bit_pointers[i]-&bit_pointers[i]->reg->bits[0],
			bit_pointers[i]->reg
		);
		bit_pointers[i]->reg->print();
	}
}

template class Fields::FieldBase< 1>;
template class Fields::FieldBase< 2>;
template class Fields::FieldBase< 3>;
template class Fields::FieldBase< 4>;
template class Fields::FieldBase< 5>;
template class Fields::FieldBase< 6>;
template class Fields::FieldBase< 7>;
template class Fields::FieldBase< 8>;
template class Fields::FieldBase< 9>;
template class Fields::FieldBase<10>;


}}}