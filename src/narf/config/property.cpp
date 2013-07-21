#include "narf/config/property.h"

//template <typename T>
//bool narf::config::Property::set(T value) {
	//manager->findNode(name) = value;
//}

template <typename T>
T narf::config::Property::as() {
	return manager.get<T>(name);
}

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
