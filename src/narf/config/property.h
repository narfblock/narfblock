#ifndef NARF_CONFIG_PROPERTY_H
#define NARF_CONFIG_PROPERTY_H

#include <stdlib.h>
#include <string>

namespace narf {
	namespace config {

		class ConfigManager;

		class Property {
			public:
				Property(ConfigManager& manager, std::string name) : name(name), manager(manager) {};
				template <typename T>
				T as();
				ConfigManager& getManager();
				std::string getName();
				bool reset();
			private:
				std::string name;
				ConfigManager& manager;
		};

	}
}

#endif
