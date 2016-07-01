#include "Log.h"
#include "Platform.h"
#include "BackgroundWorker.h"
#include "LogListener.h"

using namespace Dojo;

void Dojo::Log::_append(utf::string_view message, LogEntry::Level level) {
	mOutput.emplace_back(LogEntry(message.copy(), level));

	if (mOutput.size() == mMaxLines) {
		mOutput.erase(mOutput.begin());
	}

	_fireOnLogUpdated(getLastMessage());
}

///appends another message to the log, with an optional severity level
void Dojo::Log::append(utf::string_view message, LogEntry::Level level /*= LogEntry::EL_WARNING*/) {
	std::lock_guard<std::mutex> lock(mAppendMutex);

	_append(message, level);
}

void Log::_fireOnLogUpdated(const LogEntry& e) {
	for (auto&& listener : pListeners) {
		listener->onLogUpdated(self, getLastMessage());
	}
}
