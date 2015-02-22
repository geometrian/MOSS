#pragma once

int strlen(const char* str) {
	int length = 0;

	LOOP:
		if (*str=='\0') return length;
		++str;
		goto LOOP;
}