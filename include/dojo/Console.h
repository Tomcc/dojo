#pragma once

#include "dojo_common_header.h"

#include "Renderable.h"
#include "LogListener.h"

namespace Dojo
{
	class TextArea;
	class Log;

	///A Console is a renderable console widget that displays the contents of a Log in realtime
	class Console : 
		public Renderable,
		public LogListener
	{
	public:

		///Creates a new Console object with an AABB, a given font and a Log to be displayed
		Console( Object* parent, const Vector& topLeft, const Vector& bottomRight, const String& fontName, Log* source = gp_log );

		///adds a log to the listened logs - if getOldMessages is true, all of the messages in the log are added to the console
		void addLog( Log* l, bool getOldMessages = true );

		virtual void onLogUpdated( Log* l, const LogEntry& message );

		virtual void onAction( float dt );

		virtual void onDestruction();

	protected:

		std::list< String > mOutput;

		int mMaxLines, mPixelSize;
		bool mDirty;
		String mFontName;

		TextArea* mText;

		std::vector< Log* > pLogs;

		void _update();
	};
}


	