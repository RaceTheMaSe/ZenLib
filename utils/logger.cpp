#include "logger.h"

#ifdef USE_LOG
// Definition of the static log callback
std::function<void(ZenLib::Log::EMessageType type, const char* line)> ZenLib::Log::s_LogCallback;
#endif
