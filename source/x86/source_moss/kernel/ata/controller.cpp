#include "controller.h"

#include "../kernel.h"

#include "bus.h"
#include "device.h"


namespace MOSS { namespace ATA {


Controller::Controller(void) {
	for (int i=0;i<4;++i) {
		buses[i] = new Bus(this, i);
		if (!buses[i]->valid) {
			delete buses[i];
			buses[i] = nullptr;
		}
	}
}
Controller::~Controller(void) {
	for (int i=0;i<4;++i) {
		if (buses[i]!=nullptr) delete buses[i];
	}
}

void Controller::read_sectors(uint8_t* data_buffer, uint64_t lba,int num_sectors) const {
	buses[0]->command_readsectors(0, data_buffer, lba,num_sectors);
}

void Controller::print(int indent) const {
	for (int i=0;i<indent;++i) kernel->write("  "); kernel->write("Controller's buses:\n");
	for (int i=0;i<4;++i) {
		Bus* bus = buses[i];
		if (bus==nullptr) {
			for (int i=0;i<indent+1;++i) kernel->write("  "); kernel->write("%d: Bus nonexistent (or contains no devices)\n",i);
		} else {
			bus->print(indent+1);
		}
	}
}


}}