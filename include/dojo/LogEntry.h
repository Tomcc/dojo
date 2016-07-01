#pragma once

#include "dojo_common_header.h"

namespace Dojo {

	///An individual entry of the log, complete with timestamp and importance
	class LogEntry {
	public:
		enum Level {
			EL_INFO,
			EL_WARNING,
			EL_ERROR,
			ER_FATAL
		};

		time_t timestamp;
		utf::string text;
		Level level;

		LogEntry(utf::string_view msg, Level lvl) :
			text(msg.copy()),
			level(lvl) {
			timestamp = time(nullptr);
		}
	};

}
