#include "narf/config/configfil.h"

#include <string>
#include <vector>
#include <fstream>

narf::config::ConfigFile::ConfigFile(std::string filename) {
	loadFile(filename);
}

narf::config::ConfigFile::ConfigFile(std::string filename, std::vector<Variable> initial_variables) : filename(filename) {
	for (auto var : initial_variables) {
		variables[var.name] = var;
	}
	load();
}

void narf::config::ConfigFile::loadFile() {
	std::ifstream file(filename);
	std::string line;
	if (!file.good()) {
		// Ohnos
		return
	}
	while (std::getline(file, line)) {
		// open filename
		// iterate over lines
		lines.push_back(line);
		auto variable = parseLine(line)
		current_section = ""
		if (!variable.empty()) {
			variables.push_back(variable);
		}
	}
}

bool narf::config::ConfigFile::save() {
	// Iterate over lines, checking if line matches stored variable
	// If it doesn't, replace line
	// Save line to file
	// If line is last line of section, create lines for unsaved variables (that don't equal their default)
	// if end of file, iterate over unsaved variables and make sections + lines for them
}

Variable get(std::string name) {
	if (variables.count(name) > 0) {
	for (auto var : variables) {
		if (var.name == name) {
			return var;
		}
	}
	return Variable(configName + "." + name) // Return empty variable
}

bool narf::config::ConfigFile::unset(std::string name) { // Reset variable to default and mark as not outputted
	if (variables.count(name) > 0) {
		variables[name].reset();
		variables[name].show = false;
		return true;
	}
	return false;
}

bool narf::config::ConfigFile::set(Variable var);

bool narf::config::ConfigFile::exists(std::string name);

	private:
		std::vector<std::string> lines;
		std::map<std::string, Variable> variables;
		std::string filename;
};
