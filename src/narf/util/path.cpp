#include "narf/util/path.h"

#include <Poco/Path.h>

#ifdef linux

#include <sys/stat.h>
#include <unistd.h>

static std::string readLink(const std::string &path) {
	ssize_t len = 1024;
	ssize_t lenStep = 1024;
	ssize_t maxLen = 1024 * 4096;

	do {
		char *buf = new char[len + 1];
		if (!buf) {
			return std::string("");
		}

		auto bytes = readlink(path.c_str(), buf, len + 1);
		if (bytes < 0) {
			return std::string("");
		}

		if (bytes > len) {
			// try again with larger buffer
			len += lenStep;
			delete [] buf;
		} else {
			buf[bytes] = '\0';
			auto s = std::string(buf);
			delete [] buf;
			return s;
		}
	} while (len <= maxLen);

	return std::string("");
}


std::string narf::util::exeName() {
	return readLink("/proc/self/exe");
}

#endif // linux


#ifdef _WIN32

#include <windows.h>

std::string narf::util::exeName() {
#ifdef UNICODE
	// TODO: wstring nonsense
#else
	return std::string(_pgmptr);
#endif
}

#endif // _WIN32


std::string narf::util::exeDir() {
	auto path = Poco::Path(narf::util::exeName());
	path.setFileName("");
	return path.toString();
}

