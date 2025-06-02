#include "__fprint.h"

#include "string.h"



int __moss_fprint_dec_u( FILE* stream, uint64_t val, int prec, bool septicks )
{
	__moss_assert( prec>=-1, "Invalid `prec`=%d; must be -1 or larger!", prec );
	if ( prec == -1 ) prec=1;
	else if ( prec==0 && val==0 ) return 0;
	__moss_assert_impl( prec >= 0 );

	int n=0, ret;

	// Output in reverse order, then output that reversed to get forward output.

	// Largest is 2⁶⁴-1 = "18'446'744'073'709'551'615"
	char tmp[ 26 + 1 ];
	memset( tmp, 0x00, 26+1 );

	char* tmp_ptr = tmp;
	int num_digs = 0;
	if ( val > 0 )
	{
		LOOP:

		*(tmp_ptr++) = '0' + val%10;
		val /= 10;
		++num_digs;

		if ( val > 0 )
		{
			if ( septicks && num_digs%3==0 ) *(tmp_ptr++)='\'';
			goto LOOP;
		}
	}
	--tmp_ptr;

	while ( prec > num_digs )
	{
		ret = fputc( '0', stream );
		if ( ret == EOF ) return ret;
		++n;

		--prec;

		if ( septicks && prec>0 && prec%3==0 )
		{
			ret = fputc( '\'', stream );
			if ( ret == EOF ) return ret;
			++n;
		}
	}

	while ( tmp_ptr >= tmp )
	{
		ret = fputc( *(tmp_ptr--), stream );
		if ( ret == EOF ) return ret;
		++n;
	}

	return n;
}
