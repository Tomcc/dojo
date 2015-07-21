#include "LogListener.h"
#include "LogEntry.h"

using namespace Dojo;


void StdoutLog::onLogUpdated(Log* l, const LogEntry& message) {
	std::cout << message.text << std::endl;
}
