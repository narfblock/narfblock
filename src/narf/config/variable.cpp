#include "narf/config/variable.h"

bool narf::config::Variable::reset() {
	// TODO: Implement
	return false;
}

narf::config::ConfigManager& narf::config::Variable::getManager() {
	return manager;
}

std::string narf::config::Variable::getName() {
	return name;
}
