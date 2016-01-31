#pragma once

#include "dojo_common_header.h"

namespace Dojo {
	class LogEntry;

	///A Log::Listener receives events fired by Logs
	class LogListener {
	public:
		///the LogUpdate event is fired each time a new string is appended to a Log
		virtual void onLogUpdated(Log& l, const LogEntry& message) {
		}
	};

	///A StdoutListener just echoes the log on the standard output
	class StdoutLog : public LogListener {
	public:
		virtual void onLogUpdated(Log& l, const LogEntry& message);
	};

}
