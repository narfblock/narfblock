#ifndef NARF_UTIL_PATH_H
#define NARF_UTIL_PATH_H

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#include <string>
#include <cstring>

namespace narf {
	namespace util {
		extern const std::string DirSeparator;

		std::string exeName();
		std::string exeDir();

		std::string appendPath(const std::string& path, const std::string& append);
		bool dirExists(const std::string& path);
		std::string dirName(const std::string& path);

		bool createDir(const std::string& path);
		bool createDirs(const std::string& path);
		bool exists(const std::string& path);
		void rename(const std::string& path, const std::string& newPath);
		bool isDir(const std::string& path);
		std::string baseName(const std::string& path);
	}
}

#endif // NARF_UTIL_PATH_H