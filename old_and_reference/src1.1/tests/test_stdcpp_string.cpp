#include "../libmoss-stdcpp/cstdio"

#include "../libmoss-stdcpp/string"



void print( std::string const& str ) noexcept
{
	std::printf( "\"%s\"\n", str.c_str() );
}

int main( int /*arc*/, char* /*argv*/[] )
{
	__moss_memory_dump();
	{
		std::string str = "hello world";
		print( str );

		str += "?!";
		print( str );

		str.pop_back();
		print( str );

		std::printf( "<front,back,4> = <%c,%c,%c>\n",
			str.front(), str.back(), str[4]
		);

		str += str;
		print( str );

		str += str;
		print( str );

		str += str;
		print( str );
	}
	__moss_memory_dump();

	(void)std::getchar();

	return 0;
}
