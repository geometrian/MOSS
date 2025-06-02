#include "color.h"



static_assert( sizeof(sRGB) == 3 );

static_assert( sizeof(A_sRGB) == 4 );

static_assert( sizeof(sRGB_A) == 4 );

static_assert( sizeof(lRGBA) == 4*sizeof(float) );

static_assert( sizeof(sBGR_A) == 4 );



/*
NUM_LVLS = 24
arr = []
for i in range( NUM_LVLS ):
	lum = float(i) / float(NUM_LVLS-1)

	if lum < 0.0031308:
		gam = 12.92 * lum
	else:
		gam = 1.055*pow( lum, 1.0/2.4 ) - 0.055
	gam = int(round( 255.0 * gam ))
	arr.append( gam )
s = str(arr)[1:-1]
print(f"uint8 const srgb_gamma_{NUM_LVLS}[ {NUM_LVLS} ] =\n{{\n\t{s}\n}};\n")
*/

uint8_t const srgb_gamma_6 [ 6] =
{
	0, 124, 170, 203, 231, 255
};
uint8_t const srgb_gamma_24[24] =
{
	  0,  59,  83, 101, 116, 128, 140, 150, 159, 168, 176, 184,
	191, 198, 205, 211, 217, 223, 229, 234, 240, 245, 250, 255
};

/*std::array<sRGB,8+8> const vga_colors =
{
	{   0,   0,   0 }, //  0 = black
	{ 170,   0,   0 }, //  1 = dark red
	{   0, 170,   0 }, //  2 = dark green
	{ 170,  85,   0 }, //  3 = brown

	{   0,   0, 170 }, //  4 = dark blue
	{ 170,   0, 170 }, //  5 = dark magenta
	{   0, 170, 170 }, //  6 = dark cyan
	{ 170, 170, 170 }, //  7 = light gray

	{  85,  85,  85 }, //  8 = dark gray
	{ 255,  85,  85 }, //  9 = bright red
	{  85, 255,  85 }, // 10 = bright green
	{ 255, 255,  85 }, // 11 = bright yellow

	{  85,  85, 255 }, // 12 = bright blue
	{ 255,  85, 255 }, // 13 = bright magenta
	{  85, 255, 255 }, // 14 = bright cyan
	{ 255, 255, 255 }  // 15 = white
};*/



MOSS_ND sRGB_A color_rgba_avg( sRGB_A const* colors, size_t count )
{
	lRGBA sum = {{ 0.0f, 0.0f, 0.0f, 0.0f }};
	for ( size_t k=0; k<count; ++k )
	{
		lRGBA lrgba = color_srgba_to_lrgba( colors[k] );
		sum = color_lrgba_add( sum, lrgba );
	}
	sum.vec = vec4f_scale( sum.vec, 1.0f/(float)count );
	return color_lrgba_to_srgba( sum );
}



MOSS_ND sRGB_A color_lrgba_to_srgba( lRGBA  lrgba  )
{
	uint8_t r = color_srgb_gamma_encode( lrgba.r );
	uint8_t g = color_srgb_gamma_encode( lrgba.g );
	uint8_t b = color_srgb_gamma_encode( lrgba.b );
	return sRGB_A_( r, g, b, (uint8_t)roundf(lrgba.a*255.0f) );
}
MOSS_ND lRGBA  color_srgba_to_lrgba( sRGB_A srgb_a )
{
	float r = color_srgb_gamma_decode( srgb_a.r );
	float g = color_srgb_gamma_decode( srgb_a.g );
	float b = color_srgb_gamma_decode( srgb_a.b );
	return lRGBA_( r, g, b, 255.0f*(float)srgb_a.a );
}

#if 0

[[nodiscard]] XYZ lrgb_to_xyz( lRGB const& lrgb ) noexcept
{
	//https://en.wikipedia.org/wiki/SRGB#From_sRGB_to_CIE_XYZ
	constexpr float matrix[3*3] =
	{
		0.4124f, 0.3576f, 0.1805f,
		0.2126f, 0.7152f, 0.0722f,
		0.0193f, 0.1192f, 0.9505f
	};
	float X = lrgb.r*matrix[0] + lrgb.g*matrix[1] + lrgb.b*matrix[2];
	float Y = lrgb.r*matrix[3] + lrgb.g*matrix[4] + lrgb.b*matrix[5];
	float Z = lrgb.r*matrix[6] + lrgb.g*matrix[7] + lrgb.b*matrix[8];
	return XYZ( X, Y, Z );
}

[[nodiscard]] Lab xyz_to_cielab( XYZ const& xyz ) noexcept
{
	//https://en.wikipedia.org/wiki/CIELAB_color_space#Converting_between_CIELAB_and_CIEXYZ_coordinates
	auto f = []( float t )
	{
		constexpr float del = (float)(6.0/29.0);
		if ( t > del*del*del ) return std::cbrt( t );
		return std::fma( (float)(1.0/(3.0*del*del)),t, (float)(4.0/29.0) );
	};
	float fX=f(xyz.X), fY=f(xyz.Y), fZ=f(xyz.Z);
	float L = std::fma( 116.0f,fY, -16.0f );
	float a = 500.0f*( fX - fY );
	float b = 200.0f*( fY - fZ );
	return Lab( L, a, b );
}




[[nodiscard]] float32 deltaE_1976_sq( sRGB const& col0, sRGB const& col1 ) noexcept
{
	//https://en.wikipedia.org/wiki/Color_difference#CIE76
	Lab lab0 = srgb_to_cielab( col0 );
	Lab lab1 = srgb_to_cielab( col1 );
	float32 dL = lab1.L - lab0.L;
	float32 da = lab1.a - lab0.a;
	float32 db = lab1.b - lab0.b;
	return dL*dL + da*da + db*db;
}



[[nodiscard]] uint8 srgb_to_uefi_bitfield( sRGB const& fg, sRGB const& bg ) noexcept
{
	/*
	Not clear what UEFI does, but if it emulates VGA, VGA seems to be standardized to levels of 0,
	170, or 255 (with 85 thrown in for bright colors and yellow).  UEFI supports one bit for each
	color and then, for the foreground only, a global brightness.  Thus, we have:

		0'000 => {   0,   0,   0 } (000)   1'000 => {  85,  85,  85 } (1'000)
		0'001 => {   0,   0, 170 } (100)   1'001 => {  85,  85, 255 } (1'100)
		0'010 => {   0, 170,   0 } (010)   1'010 => {  85, 255,  85 } (1'010)
		0'011 => {   0, 170, 170 } (110)   1'011 => {  85, 255, 255 } (1'110)
		0'100 => { 170,   0,   0 } (001)   1'100 => { 255,  85,  85 } (1'001)
		0'101 => { 170,   0, 170 } (101)   1'101 => { 255,  85, 255 } (1'101)
		0'110 => { 170,  85,   0 } (011)   1'110 => { 255, 255,  85 } (1'011)
		0'111 => { 170, 170, 170 } (111)   1'111 => { 255, 255, 255 } (1'111)
	*/

	struct Best final
	{
		float32 delE = std::numeric_limits<float32>::infinity();
		uint8 ind;
	};
	Best best_bg, best_fg;
	for ( uint8 i=0; i<=  0b111; ++i ) //3 bits for background
	{
		float32 delE = deltaE_1976_sq( bg, vga_colors[i] );
		if ( delE < best_bg.delE )
		{
			best_bg.delE = delE;
			best_bg.ind = i;
		}
	}
	for ( uint8 i=0; i<=0b1'111; ++i ) //4 bits for foreground
	{
		float32 delE = deltaE_1976_sq( fg, vga_colors[i] );
		if ( delE < best_fg.delE )
		{
			best_fg.delE = delE;
			best_fg.ind = i;
		}
	}

	constexpr uint8 bitfields[16] =
	{
		0b0'000, 0b0'100, 0b0'010, 0b0'110,
		0b0'001, 0b0'101, 0b0'011, 0b0'111,
		0b1'000, 0b1'100, 0b1'010, 0b1'110,
		0b1'001, 0b1'101, 0b1'011, 0b1'111
	};
	uint8 result = (bitfields[best_bg.ind]<<4) | bitfields[best_fg.ind];
	if      ( result == 0              ) result=0b100'1'001;
	else if ( result >  0b111'1'111    ) result=0b100'1'101;
	else if ( best_fg.ind==best_bg.ind ) result=0b100'1'110;
	return result;
	//return 0b011'1'101;
}

#endif
