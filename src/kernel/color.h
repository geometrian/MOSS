#pragma once

#include "stdafx.h"



typedef struct { uint8_t r,g,b; } sRGB;
#define sRGB_( R, G, B ) ((sRGB){ R, G, B })

typedef union
{
	struct { uint8_t a, r,g,b; };
	uint32_t packed;
	uint8_t data[4];
}
A_sRGB;
#define A_sRGB_( A, R,G,B ) ((A_sRGB){{ A, R,G,B }})

typedef union
{
	struct { uint8_t r,g,b, a; };
	uint32_t packed;
	uint8_t data[4];
}
sRGB_A;
#define sRGB_A_( R,G,B, A ) ((sRGB_A){{ R,G,B, A }})

typedef union
{
	struct { float r,g,b, a; };
	vec4f vec;
	float data[4];
}
lRGBA;
#define lRGBA_( R,G,B, A ) ((lRGBA){{ R,G,B, A }})

typedef union
{
	struct { uint8_t b,g,r, a; };
	uint32_t packed;
	uint8_t data[4];
}
sBGR_A;

MOSS_ND_INLINE A_sRGB color_srgba_to_asrgb( sRGB_A srgba )
{
	// R, G, B, A   ->   A, R, G, B
	#if 0 //Impedes optimization
		_A_sRGB ret;
		asm(
			"rorx   %0,%1, 24\n"
			: "=r"(ret.packed)
			: "rm"(srgba.packed)
			:
		);
		return ret;
	#else
		return (A_sRGB){ .packed=(srgba.packed>>24)|(srgba.packed<<8) };
	#endif
}
MOSS_ND_INLINE sBGR_A color_srgba_to_sbgra( sRGB_A srgba )
{
	// R, G, B, A   ->   A, R, G, B  ->   B, G, R, A
	#if 0 //Impedes optimization
		_sBGR_A ret;
		asm(
			"rorx    %0,%1, 24\n"
			"bswap   %0\n"
			: "=r"(ret.packed)
			: "rm"(srgba.packed)
			:
		);
		return ret;
	#else
		return (sBGR_A){ .packed=__builtin_bswap32((srgba.packed>>24)|(srgba.packed<<8)) };
	#endif
}

MOSS_ND sRGB_A color_rgba_avg( sRGB_A const* colors, size_t count );

MOSS_ND_INLINE lRGBA color_lrgba_add( lRGBA lhs, lRGBA rhs )
{
	return (lRGBA){ .vec=vec4f_add( lhs.vec, rhs.vec ) };
}



/*union sRGB
{
	struct { uint8 r, g, b; };
	uint8 rgb[3];

	sRGB() noexcept = default;
	constexpr sRGB( uint8 r, uint8 g, uint8 b ) noexcept : r(r), g(g), b(b) {}
	sRGB( std::initializer_list<uint8> ilist ) noexcept
	{
		uint8* ptr = rgb;
		for ( auto iter=ilist.begin(); iter!=ilist.end(); ++iter ) *(ptr++)=*iter;
	}

	[[nodiscard]] bool operator==( sRGB const& other ) const noexcept
	{
		return r==other.r && g==other.g && b==other.b;
	}
};

union lRGB
{
	struct { float32 r, g, b; };
	float32 rgb[3];

	lRGB() noexcept = default;
	constexpr lRGB( float32 r, float32 g, float32 b ) noexcept : r(r), g(g), b(b) {}
	lRGB( std::initializer_list<float32> ilist ) noexcept
	{
		float32* ptr = rgb;
		for ( auto iter=ilist.begin(); iter!=ilist.end(); ++iter ) *(ptr++)=*iter;
	}
};

union XYZ
{
	struct { float X, Y, Z; };
	float data[3];

	XYZ() noexcept = default;
	constexpr XYZ( float32 X, float32 Y, float32 Z ) noexcept : X(X), Y(Y), Z(Z) {}
	XYZ( std::initializer_list<float32> ilist ) noexcept
	{
		float32* ptr = data;
		for ( auto iter=ilist.begin(); iter!=ilist.end(); ++iter ) *(ptr++)=*iter;
	}
};

//https://en.wikipedia.org/wiki/CIELAB_color_space
union Lab final
{
	struct { float32 L, a, b; };
	float32 data[3];

	Lab() noexcept = default;
	constexpr Lab( float32 L, float32 a, float32 b ) noexcept : L(L), a(a), b(b) {}
	Lab( std::initializer_list<float32> ilist ) noexcept
	{
		float32* ptr = data;
		for ( auto iter=ilist.begin(); iter!=ilist.end(); ++iter ) *(ptr++)=*iter;
	}
};*/



extern uint8_t const srgb_gamma_6 [ 6];
extern uint8_t const srgb_gamma_24[24];

//extern sRGB const vga_colors[8+8];



MOSS_ND_INLINE uint8_t color_srgb_gamma_encode( float val ) // gamma + quantization
{
	val = val<0.0031308f ? 12.92f*val : 1.055f*powf(val,1.0f/2.4f)-0.055f;
	return (uint8_t)roundf( 255.0f * val);
}
MOSS_ND_INLINE float color_srgb_gamma_decode( uint8_t val ) // unpack and inverse gamma
{
	float tmp = (float)val / 255.0f;
	return tmp<0.04045f ? tmp/12.92f : powf((tmp+0.055f)/1.055f,2.4f);
}

MOSS_ND sRGB_A color_lrgba_to_srgba( lRGBA  lrgba  );
MOSS_ND lRGBA  color_srgba_to_lrgba( sRGB_A srgb_a );

#if 0
XYZ lrgb_to_xyz( lRGB const& lrgb );
/*MOSS_ND_INLINE lRGB xyz_to_lrgb( XYZ const& xyz ) noexcept
{
	//https://en.wikipedia.org/wiki/SRGB#From_CIE_XYZ_to_sRGB
}*/

Lab        xyz_to_cielab ( XYZ  const& xyz  );
MOSS_ND_INLINE Lab lrgb_to_cielab( lRGB const& lrgb )
{
	return xyz_to_cielab( lrgb_to_xyz(lrgb) );
}
MOSS_ND_INLINE Lab srgb_to_cielab( sRGB const& srgb )
{
	return lrgb_to_cielab( srgb_to_lrgb(srgb) );
}



//Calculates (CIE76 ΔE*)²
MOSS_ND        float deltaE_1976_sq( sRGB const& col0, sRGB const& col1 ) noexcept;
//TODO: this
MOSS_ND_INLINE float deltaE_1976   ( sRGB const& col0, sRGB const& col1 ) noexcept
{
	return sqrtf(deltaE_1976_sq( col0, col1 ));
}



[[nodiscard]] uint8_t srgb_to_uefi_bitfield( sRGB const& fg, sRGB const& bg ) noexcept;
#endif
