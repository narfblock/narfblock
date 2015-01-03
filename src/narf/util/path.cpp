#include "narf/console.h"
#include "narf/util/path.h"

#include <Poco/Path.h>
#include <Poco/File.h>

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

#endif // linux


#ifdef _WIN32

#include <windows.h>
#include <shlobj.h>
#include <Poco/Buffer.h>
#include <Poco/UnicodeConverter.h>

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

#endif // _WIN32


std::string narf::util::exeDir() {
	auto path = Poco::Path(narf::util::exeName());
	path.setFileName("");
	return path.toString();
}


std::string narf::util::dataDir() {
	static std::string cachedDataDir;

	if (cachedDataDir != "") {
		return cachedDataDir;
	}

	// walk up the path until data directory is found
	narf::console->println("Current Dir: " + Poco::Path::current());
	narf::console->println("Executable Dir: " + narf::util::exeDir());

	for (Poco::Path dir = Poco::Path(narf::util::exeDir()); ; dir = dir.parent()) {
		auto dataDir = Poco::Path(dir, "data");
		narf::console->println("Checking " + dataDir.toString());
		auto tmp = Poco::File(dataDir);
		if (tmp.exists()) {
			narf::console->println("Found data directory: " + dataDir.toString());
			cachedDataDir = dataDir.toString();
			return cachedDataDir;
		}
		if (dir.toString() == dir.parent().toString()) {
			break;
		}
	}

	return "";
}
