#include "narf/console.h"
#include "narf/path.h"

#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>

#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#include <stdlib.h>
#endif

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#ifdef _WIN32

#include <windows.h>
#include <direct.h>
#include "narf/utf.h"

const std::string narf::util::DirSeparator("\\");
#else
const std::string narf::util::DirSeparator("/");
#endif


#ifdef linux

static std::string readLink(const std::string &path) {
	size_t len = 1024;
	size_t lenStep = 1024;
	size_t maxLen = 1024 * 4096;

	do {
		char *buf = new char[len + 1];
		if (!buf) {
			return std::string("");
		}

		auto bytes = readlink(path.c_str(), buf, len + 1);
		if (bytes < 0) {
			return std::string("");
		}

		if (static_cast<size_t>(bytes) > len) {
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

#if defined(__unix__) && !defined(linux)
std::string narf::util::exeName() {
	return "";
}
#endif // unix && !linux


#if defined(__unix__) || defined(__APPLE__)

bool narf::util::dirExists(const std::string& path) {
	struct stat st;
	if (stat(path.c_str(), &st) == -1) {
		return false;
	}
	return (st.st_mode & S_IFMT) == S_IFDIR;
}

#endif // unix

#ifdef _WIN32

std::string narf::util::exeName() {
	const DWORD len = 32768; // max NT-style name length + terminator
	wchar_t buffer[len];

	DWORD ret = GetModuleFileNameW(NULL, buffer, len);
	if (ret == 0 || ret >= len) {
		return ""; // error
	}

	std::string result;
	narf::toUTF8(buffer, result);
	return result;
}


bool narf::util::dirExists(const std::string& path) {
	std::wstring pathW;
	narf::toUTF16(path, pathW);
	DWORD attrs = GetFileAttributesW(pathW.c_str());
	return (attrs != INVALID_FILE_ATTRIBUTES) && (attrs & FILE_ATTRIBUTE_DIRECTORY);
}

#endif // _WIN32

#ifdef __APPLE__

std::string narf::util::exeName() {
	char buffer[32768];
	uint32_t len = sizeof(buffer);

	if (_NSGetExecutablePath(buffer, &len) == 0) {
		return std::string(buffer);
	}

	return ""; // error
}

#endif // __APPLE__


std::string narf::util::exeDir() {
	return dirName(exeName());
}


std::string narf::util::appendPath(const std::string& path, const std::string& append) {
	if (path.back() == DirSeparator[0]) {
		return path + append;
	} else {
		return path + DirSeparator + append;
	}
}

bool narf::util::createDir(const std::string& path) {
#ifdef _WIN32
	std::wstring pathW;
	narf::toUTF16(path, pathW);
	return _wmkdir(pathW.c_str()) == 0;
#else
	return mkdir(path.c_str(), 0755) == 0;
#endif
}

bool narf::util::createDirs(const std::string& path) {
	if (isDir(path)) {
		return true; // We're already a directory
	}
	if (!createDirs(dirName(path))) {
		return false; // Failed to create one of the parent directories
	}
	return createDir(path);
}

bool narf::util::exists(const std::string& path) {
#ifdef _WIN32
	std::wstring pathW;
	narf::toUTF16(path, pathW);
	struct _stat st;
	return _wstat(pathW.c_str(), &st) == 0;
#else
	struct stat st;
	return stat(path.c_str(), &st) == 0;
#endif
}

void narf::util::rename(const std::string& path, const std::string& newPath) {
	::rename(path.c_str(), newPath.c_str());
}

bool narf::util::isDir(const std::string& path) {
#ifdef _WIN32
	std::wstring pathW;
	narf::toUTF16(path, pathW);
	struct _stat st;
	if (_wstat(pathW.c_str(), &st) != 0) {
		return false;
	}
#else
	struct stat st;
	if (stat(path.c_str(), &st) != 0) {
		return false;
	}
#endif
	return S_ISDIR(st.st_mode);
}

// TODO: These will be broken on Windows when given the root directory
std::string narf::util::dirName(const std::string& path) {
	size_t len = path.length();
	while (len > 0 && path[len - 1] == DirSeparator[0]) {
		len--;
	}
	auto lastSlash = path.rfind(narf::util::DirSeparator, len - 1);
	if (lastSlash != std::string::npos) {
		return path.substr(0, lastSlash == 0 ? 1 : lastSlash);
	}
	return path;
}

std::string narf::util::baseName(const std::string& path) {
	size_t len = path.length();
	while (len > 0 && path[len - 1] == DirSeparator[0]) {
		len--;
	}
	size_t lastSlash = path.rfind(DirSeparator, len - 1);
	if (len > 1 && lastSlash != std::string::npos) {
		return path.substr(lastSlash + 1, len - lastSlash - 1);
	}
	return path;
}
