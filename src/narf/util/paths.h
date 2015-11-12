#ifndef NARF_UTIL_PATHS_H
#define NARF_UTIL_PATHS_H

#include "narf/path.h"
#include "narf/console.h"
#include <string>

namespace narf {
	namespace util {
		std::string dataDir();
		std::string userConfigDir(const std::string& appName);
	}
}

#endif // NARF_UTIL_PATHS_H
