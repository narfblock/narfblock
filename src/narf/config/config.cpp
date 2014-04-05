#include <Poco/Util/IniFileConfiguration.h>
#include <string>
#include <stdio.h>

#include "narf/config/config.h"
#include "narf/util/tokenize.h"
#include "narf/notifications/ConfigNotification.h"

void narf::config::ConfigManager::load(std::string name, std::string filename) {
	narf::config::ConfigIni* cfg;
	try {
		cfg = new narf::config::ConfigIni(filename);
	} catch (...) {
		// file not found or could not be opened
		// use an empty config (get defaults)
		cfg = new narf::config::ConfigIni();
	}
	configs[name] = cfg;
}

bool narf::config::ConfigManager::getRaw(const std::string& key, std::string& value) const {
	auto tokens = narf::util::tokenize(key, '.');
	if (tokens.size() < 2 || configs.count(tokens[0]) != 1) {
		return false;
	}
	const std::string subkey = narf::util::join(std::vector<std::string>(tokens.begin() + 1, tokens.end()), ".");
	return configs[tokens[0]]->getRaw(subkey, value);
}

void narf::config::ConfigManager::setRaw(const std::string& key, const std::string& value) {
	auto tokens = narf::util::tokenize(key, '.');
	if (tokens.size() < 2 || configs.count(tokens[0]) != 1) {
		return;
	}
	const std::string subkey = narf::util::join(std::vector<std::string>(tokens.begin() + 1, tokens.end()), ".");
	configs[tokens[0]]->setRaw(subkey, value);
	notificationCenter.postNotification(new narf::config::ConfigUpdateNotification(key));
}
void narf::config::ConfigManager::enumerate(const std::string& key, Keys& range) const {
	auto tokens = narf::util::tokenize(key, '.');
	if (tokens.size() < 2 || configs.count(tokens[0]) != 1) {
		return;
	}
	const std::string subkey = narf::util::join(std::vector<std::string>(tokens.begin() + 1, tokens.end()), ".");
	configs[tokens[0]]->enumerate(subkey, range);
}


void narf::config::ConfigManager::initBool(const std::string& key, bool defaultValue) {
	setBool(key, getBool(key, defaultValue));
}


void narf::config::ConfigManager::initInt(const std::string& key, int defaultValue) {
	setInt(key, getInt(key, defaultValue));
}


void narf::config::ConfigManager::initDouble(const std::string& key, double defaultValue) {
	setDouble(key, getDouble(key, defaultValue));
}

void narf::config::ConfigManager::initString(const std::string& key, const std::string& defaultValue) {
	setString(key, getString(key, defaultValue));
}
