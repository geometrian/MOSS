#pragma once

#include "../includes.hpp"


namespace MOSS { namespace Serial {


//https://wiki.osdev.org/Serial_Ports

void init(void);

void write(uint8_t value);

uint8_t read(void);


}}