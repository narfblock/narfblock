#ifndef NARF_CONFIG_PROPERTY_H
#define NARF_CONFIG_PROPERTY_H

#include <stdlib.h>
#include "narf/config/configmanager.h"

namespace narf {
	namespace config {

		class ConfigManager;

		class Property {
			public:
				Property(const ConfigManager& manager, std::string name) : name(name), manager(manager) {};
				template <typename T>
				T as();
				const ConfigManager& getManager();
				std::string getName();
				bool reset();
			private:
				std::string name;
				const ConfigManager& manager;
		};
	}
}

#endif
