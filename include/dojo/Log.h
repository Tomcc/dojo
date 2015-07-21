#pragma once

#include "dojo_common_header.h"

#include "LogEntry.h"

namespace Dojo {
	class LogListener;

	///The Log class manages dojo's debug output and can redirect it to file, or it can be read from a console
	class Log {
	public:

		typedef std::vector<LogEntry> LogQueue;

		explicit Log(uint32_t maxLines = 1024) :
			mMaxLines(maxLines) {
			DEBUG_ASSERT( mMaxLines > 0, "Cannot create a Log with 0 or less lines" );
		}

		~Log() {
		}

		///appends another message to the log, with an optional severity level
		void append(const std::string& message, LogEntry::Level level = LogEntry::EL_WARNING);

		///adds a listener that will receive events from this Log
		void addListener(LogListener& l) {
			pListeners.emplace(&l);
		}

		void removeListener(LogListener& l) {
			pListeners.erase(&l);
		}

		///returns the last appended entry
		const LogEntry& getLastMessage() {
			return mOutput.back();
		}

		LogQueue::iterator begin() {
			return mOutput.begin();
		}

		LogQueue::iterator end() {
			return mOutput.end();
		}

	protected:

		SmallSet<LogListener*> pListeners;
		LogQueue mOutput;
		uint32_t mMaxLines;

		void _append(const std::string& message, LogEntry::Level level);

		void _fireOnLogUpdated(const LogEntry& e);
	};
}
