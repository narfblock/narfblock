#include "narf/config/property.h"

bool narf::config::Property::reset() {
	// TODO: Implement
	return false;
}

const narf::config::ConfigManager& narf::config::Property::getManager() {
	return manager;
}

std::string narf::config::Property::getName() {
	return name;
}
