#include "../libmoss-stdcpp/new"
#include "../libmoss-stdcpp/cstdio"
#include "../libmoss-stdcpp/cstdlib"



void test_simple()
{
	std::__moss_memory_dump();

	int* data0 = new int[4];
	std::__moss_memory_dump();

	delete[] data0;
	std::__moss_memory_dump();
}

void test_delete_middle()
{
	int* data0 = new int[4];
	int* data1 = new int[4];
	int* data2 = new int[4];
	std::printf("Allocated three blocks:\n");
	std::__moss_memory_dump();

	std::printf("\nDeleting middle one:\n");
	delete[] data1;
	std::__moss_memory_dump();

	std::printf("\nDeleting first one:\n");
	delete[] data0;
	std::__moss_memory_dump();

	std::printf("\nDeleting end one:\n");
	delete[] data2;
	std::__moss_memory_dump();
}

int main( int /*arc*/, char* /*argv*/[] )
{
	//test_simple();
	test_delete_middle();

	(void)std::getchar();

	return 0;
}
