#ifndef NARF_UTIL_PATH_H
#define NARF_UTIL_PATH_H

#include <string>

namespace narf {
	namespace util {
		extern const std::string DirSeparator;

		std::string exeName();
		std::string exeDir();

		std::string dataDir();

		std::string userConfigDir(const std::string& appName);

		std::string appendPath(const std::string& path, const std::string& append);
		std::string dirName(const std::string& path);
		bool dirExists(const std::string& path);
	}
}

#endif // NARF_UTIL_PATH_H
