#include "../libmoss-stdcpp/cstdio"

#include "../libmoss-stdcpp/algorithm"

#include "../libmoss-stdcpp/vector"



void print( std::vector<int> const& vec ) noexcept
{
	std::printf("[");
	for ( int i : vec ) std::printf(" %d",i);
	std::printf(" ]\n");
}

int main( int /*arc*/, char* /*argv*/[] )
{
	std::vector<int> vec = { 2, 1, 2, 3, 2, 2, 5, 1, 2, 7, 2, 2 }; // [2,1,2,3,2,2,5,1,2,7,2,2]
	print(vec);

	auto compacted_end = std::remove( vec.begin(),vec.end(), 2 );  // [1,3,5,1,7,2,#,#,2,#,2,2]
	print(vec);

	vec.erase( compacted_end, vec.end() );                         // [1,3,5,1,7]
	print(vec);

	return 0;
}
