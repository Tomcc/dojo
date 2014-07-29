#pragma once

#include "dojo_common_header.h"

#include "Renderable.h"
#include "TextArea.h"
#include "Log.h"

namespace Dojo
{
	///A Console is a renderable console widget that displays the contents of a Log in realtime
	class Console : 
		public Renderable,
		public Log::Listener
	{
	public:

		///Creates a new Console object with an AABB, a given font and a Log to be displayed
		Console( Object* parent, const Vector& topLeft, const Vector& bottomRight, const String& fontName, Log* source = Dojo::gp_log );

		///adds a log to the listened logs - if getOldMessages is true, all of the messages in the log are added to the console
		void addLog( Log* l, bool getOldMessages = true )
		{
			DEBUG_ASSERT( l, "Passed a NULL Log to a Console" );
			
			pLogs.push_back( l );
			l->addListener( this );

			if( getOldMessages )
			{
				for( auto& entry : *l )
					onLogUpdated( l, entry );
			}
		}

		virtual void onLogUpdated( Log* l, const Log::Entry& message )
		{
			mDirty = true;
			
			mOutput.push_back( message.text );

			if( mOutput.size() > (size_t)mMaxLines )
				mOutput.erase( mOutput.begin() );
		}

		virtual void onAction( float dt );

		virtual void onDestruction()
		{
			for( auto log : pLogs )
				log->removeListener( this );
		}

	protected:

		std::list< String > mOutput;

		int mMaxLines, mPixelSize;
		bool mDirty;
		String mFontName;

		Dojo::TextArea* mText;

		std::vector< Log* > pLogs;

		void _update();
	};
}


	