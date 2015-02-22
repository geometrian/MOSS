#pragma once

typedef struct {
	int quot;
	int  rem;
} div_t;
typedef struct {
	long int quot;
	long int  rem;
} ldiv_t;
typedef struct {
	long long quot;
	long long  rem;
} lldiv_t;

int abs(int n);
long int labs(long int n);
long long int llabs(long long int n);

div_t div(int numer, int denom);
ldiv_t ldiv(long int numer, long int denom);
lldiv_t lldiv(long long int numer, long long int denom);