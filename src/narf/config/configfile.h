#ifndef NARF_CONFIG_CONFIGFILE_H
#define NARF_CONFIG_CONFIGFILE_H

#include <stdlib.h>

namespace narf {
	namespace config {

		class ConfigFile {
			public:
				ConfigFile(std::string filename);
				bool save();

				Variable get(std::string path);
				Variable get(std::vector<std::string> path);

				bool unset(Variable var)
				bool set(Variable var);

				bool exists(std::string path);
				bool exists(std::vector<std::string> path);

			private:
				std::vector<std::string> lines;
				std::vector<Variable> variables;
				std::string filename;
		};

	}
}

#endif
