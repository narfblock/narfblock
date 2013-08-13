#ifndef NARF_CONFIG_VARIABLE_H
#define NARF_CONFIG_VARIABLE_H

#include <stdlib.h>
#include <string>

namespace narf {
	namespace config {

		class ConfigManager;

		class Variable {
			public:
				Variable(ConfigManager& manager, std::string name) : name(name), manager(manager) {};
				template <typename T>
				T as();
				ConfigManager& getManager();
				std::string getName();
				bool reset();
			private:
				narf::any value;
				narf::any default_value;
				std::string name;
				ConfigManager& manager;
		};

	}
}

#endif
