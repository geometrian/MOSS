#pragma once


const void* memchr(const void* ptr, int value, size_t num);
      void* memchr(      void* ptr, int value, size_t num);

const char* strchr(const char* str, int character);
      char* strchr(      char* str, int character);

size_t strcspn(const char* str1, const char* str2);

const char* strpbrk(const char* str1, const char* str2);
      char* strpbrk(      char* str1, const char* str2);

const char* strrchr(const char* str, int character);
      char* strrchr(      char* str, int character);

size_t strspn(const char* str1, const char* str2);

const char* strstr(const char* str1, const char* str2);
      char* strstr(      char* str1, const char* str2);

//TODO: this
char* strtok(char* str, const char* delimiters);