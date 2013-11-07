#include "Poco/Notification.h"
#include <string>

namespace narf {
	namespace config {

		class ConfigBaseNotification : public Poco::Notification {};

		class ConfigUpdateNotification : public ConfigBaseNotification {
			public:
				ConfigUpdateNotification(std::string key) : key(key) {};
				std::string key;
		};

	}
}
