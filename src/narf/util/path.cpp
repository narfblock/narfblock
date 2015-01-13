#include "narf/console.h"
#include "narf/util/path.h"


#ifdef linux

#include <sys/stat.h>
#include <unistd.h>

const std::string narf::util::DirSeparator("/");

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

std::string narf::util::userConfigDir(const std::string& appName) {
	// look up home dir and append .config/<appName>
	char* home;
	home = getenv("HOME");
	// TODO: sanitize/validate $HOME?
	if (!home) {
		// TODO: fall back to getpwuid() (use getpwuid_r?)
	}

	if (home) {
		return std::string(home) + "/.config/" + appName;
	}
	return "";
}


bool narf::util::dirExists(const std::string& path) {
	struct stat st;
	if (stat(path.c_str(), &st) == -1) {
		return false;
	}
	return (st.st_mode & S_IFMT) == S_IFDIR;
}

#endif // linux


#ifdef _WIN32

#include <windows.h>
#include <shlobj.h>
#include <Poco/Buffer.h>
#include <Poco/UnicodeConverter.h>

const std::string narf::util::DirSeparator("\\");

std::string narf::util::exeName() {
	size_t len = 32768; // max NT-style name length + terminator
	Poco::Buffer<wchar_t> buffer(len);

	DWORD ret = GetModuleFileNameW(NULL, buffer.begin(), len);
	if (ret == 0 || ret >= len) {
		return ""; // error
	}

	std::string result;
	Poco::UnicodeConverter::toUTF8(buffer.begin(), result);
	return result;
}


std::string narf::util::userConfigDir(const std::string& appName) {
	wchar_t buffer[MAX_PATH];
	HRESULT hr = SHGetFolderPathW(nullptr, CSIDL_APPDATA | CSIDL_FLAG_CREATE, nullptr, SHGFP_TYPE_CURRENT, buffer);
	if (!SUCCEEDED(hr)) {
		return ""; // error
	}
	std::string result;
	Poco::UnicodeConverter::toUTF8(buffer, result);
	return result + "\\" + appName;
}


bool narf::util::dirExists(const std::string& path) {
	std::wstring pathW;
	Poco::UnicodeConverter::toUTF16(path, pathW);
	DWORD attrs = GetFileAttributesW(pathW.c_str());
	return (attrs != INVALID_FILE_ATTRIBUTES) && (attrs & FILE_ATTRIBUTE_DIRECTORY);
}

#endif // _WIN32

std::string narf::util::dirName(const std::string& path) {
	size_t len = path.length();
	while (len > 0 && path[len - 1] == DirSeparator[0]) {
		len--;
	}
	auto lastSlash = path.rfind(narf::util::DirSeparator, len - 1);
	// TODO
	if (lastSlash != std::string::npos) {
		return path.substr(0, lastSlash);
	}
	return path;
}


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


std::string narf::util::dataDir() {
	static std::string cachedDataDir;

	if (cachedDataDir != "") {
		return cachedDataDir;
	}

	// walk up the path until data directory is found
	narf::console->println("Executable Dir: " + narf::util::exeDir());

	for (auto dir = narf::util::exeDir(); ; dir = dirName(dir)) {
		auto dataDir = appendPath(dir, "data");
		narf::console->println("Checking " + dataDir);
		if (dirExists(dataDir)) {
			narf::console->println("Found data directory: " + dataDir);
			cachedDataDir = dataDir;
			return cachedDataDir;
		}
		if (dir.compare(dirName(dir)) == 0) {
			break;
		}
	}

	return "";
}
