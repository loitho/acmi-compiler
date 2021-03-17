#include "AcmiTape.h"

#include <cstdio>
#include <chrono>

using namespace std::chrono;

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage:\n./acmi-compiler.exe source-File.flt destination-File.vhs\n");
		return -1;
	}

	ACMITape newtape;
	const auto t = steady_clock::now();

	printf("----- Starting conversion -----\n\n");
	if (newtape.Import(argv[1], argv[2]))
	{
		const duration<float> elapsed = steady_clock::now() - t;
		printf("\n----- File converted with success in %f seconds. -----\n", elapsed.count());
		return 0;
	}
	else
	{
		printf("\n[ERROR] - ERROR DURING THE CONVERSION SEE ABOVE ERROR\n");
		return -1;
	}
}

