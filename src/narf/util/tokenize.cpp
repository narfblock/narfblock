#include "narf/util/tokenize.h"
#include <stdlib.h>
#include <string>
#include <vector>
#include <sstream>

std::vector<std::string>& narf::util::tokenize(const std::string &input, char delimeter, std::vector<std::string>& tokens) {
	std::stringstream stream(input);
	std::string token;
	while (std::getline(stream, token, delimeter)) {
		tokens.push_back(token);
	}
	return tokens;
}

std::vector<std::string> narf::util::tokenize(const std::string &input, char delimeter) {
	std::vector<std::string> tokens;
	narf::util::tokenize(input, delimeter, tokens);
	return tokens;
}
