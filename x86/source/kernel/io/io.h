#pragma once


namespace MOSS { namespace IO {


//http://wiki.osdev.org/Inline_Assembly/Examples#I.2FO_access

template <typename type> type recv(unsigned short port);
template <typename type> void send(unsigned short port, type value);

void wait(void);


}}