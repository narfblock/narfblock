#include "narf/util/paths.h"

#if defined(__unix__) || defined(__APPLE__)

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

#endif // unix


#ifdef _WIN32

std::string narf::util::userConfigDir(const std::string& appName) {
	wchar_t buffer[MAX_PATH];
	HRESULT hr = SHGetFolderPathW(nullptr, CSIDL_APPDATA | CSIDL_FLAG_CREATE, nullptr, SHGFP_TYPE_CURRENT, buffer);
	if (!SUCCEEDED(hr)) {
		return ""; // error
	}
	std::string result;
	narf::toUTF8(buffer, result);
	return result + "\\" + appName;
}


#endif // _WIN32


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
