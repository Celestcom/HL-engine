#include "PluginAPI.h"

extern nsvr_core* global_core;

inline void core_log(nsvr_severity level, const std::string& component, const std::string& message) {
	nsvr_log(global_core, level, component.c_str(), message.c_str());
}

inline void core_log(const std::string& message) {
	core_log(nsvr_severity_trace, "UnknownContext", message);
}

inline void core_log(const std::string& component, const std::string& message) {
	core_log(nsvr_severity_trace, component, message);
}
