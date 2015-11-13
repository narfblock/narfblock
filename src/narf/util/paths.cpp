#include "narf/util/paths.h"

std::string narf::util::dataDir() {
	static std::string cachedDataDir;

	if (cachedDataDir != "") {
		return cachedDataDir;
	}

	// walk up the path until data directory is found
	for (auto dir = narf::util::exeDir(); ; dir = dirName(dir)) {
		auto dataDir = appendPath(dir, "data");
		if (dirExists(dataDir)) {
			cachedDataDir = dataDir;
			return cachedDataDir;
		}
		if (dir.compare(dirName(dir)) == 0) {
			break;
		}
	}

	return "";
}
