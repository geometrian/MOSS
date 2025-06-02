#pragma once

#include "../includes.hpp"


#define MOSS_DECL_FP1(FUNCTION_NAME)\
	      float FUNCTION_NAME(      float x);\
	     double FUNCTION_NAME(     double x);\
	long double FUNCTION_NAME(long double x);
