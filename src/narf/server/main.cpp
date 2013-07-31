#include <stdio.h>

#include "narf/version.h"

int main(int argc, char **argv)
{
	printf("Hello, world - I'm a server.\n");
	printf("Version: %d.%d%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE);

	return 0;
}
