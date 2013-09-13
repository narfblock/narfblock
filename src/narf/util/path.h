#ifndef NARF_UTIL_PATH_H
#define NARF_UTIL_PATH_H

#include <string>

namespace narf {
	namespace util {
		std::string exeName();
		std::string exeDir();

		std::string dataDir();
	}
}

#endif // NARF_UTIL_PATH_H
