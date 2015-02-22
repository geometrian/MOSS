#pragma once

#include "../../includes.h"


namespace MOSS { namespace IO {


//http://wiki.osdev.org/Inline_Assembly/Examples#I.2FO_access

template <typename type> type recv(uint16_t port            );
template <typename type> void send(uint16_t port, type value);

void wait(void);


}}