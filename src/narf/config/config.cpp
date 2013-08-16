#include <Poco/Util/IniFileConfiguration.h>
#include <string>
#include <stdio.h>

#include "narf/config/config.h"
#include "narf/util/tokenize.h"

void narf::config::ConfigManager::load(std::string name, std::string filename) {
	auto tmp = new narf::config::ConfigIni(filename);
	configs[name] = tmp;
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
}
void narf::config::ConfigManager::enumerate(const std::string& key, Keys& range) const {
	auto tokens = narf::util::tokenize(key, '.');
	if (tokens.size() < 2 || configs.count(tokens[0]) != 1) {
		return;
	}
	const std::string subkey = narf::util::join(std::vector<std::string>(tokens.begin() + 1, tokens.end()), ".");
	configs[tokens[0]]->enumerate(subkey, range);
}