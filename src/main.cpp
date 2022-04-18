#include "core.hpp"
#include <cstdlib>
#include <ctime>

int main()
{
	srand(time(NULL));

	Core core = Core();
	core.Loop();

	return 0;
}