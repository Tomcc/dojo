#include "stdafx.h"

#include "Console.h"
#include "GameState.h"
#include "Viewport.h"
#include "TextArea.h"
#include "Log.h"

using namespace Dojo;

Console::Console( Object* parent, const Vector& topLeft, const Vector& bottomRight, const String& fontName, Log* source ) :
	Renderable( parent, Vector::ZERO, "texturedQuad" ),
	mDirty( true )
{
	//set the console itself as backdrop
	scale = topLeft - bottomRight;
	position = (topLeft + bottomRight) * 0.5f;
	color = Color( 0,0,0,0.5 );

	//create the textArea centered in topleft
	mText = new TextArea( parent, fontName, Vector( -scale.x*0.5f + 0.3f, scale.y * 0.5f - 0.2f) );
	mText->inheritScale = false;
//	mText->setMaxLineLength( scale.x );

	//find how many lines of this textArea can be contained in this console
	Vector fontSize; 
	parent->getGameState()->getViewport()->makeScreenSize( fontSize,
		mText->getFont()->getFontWidth(), 
		mText->getLineHeight() ); 
	mMaxLines = (int)((scale.y-0.2f) / fontSize.y);
	
	addLog( source );
}

void Console::onAction( float dt )
{
	DEBUG_TODO; //this hack isn't supported anymore
// 	if( mText->getParent() != this && hasLayer() ) //attach the text in front of this object when the parent has been attached
// 		addChild( *mText, getLayer()+1 );
// 
// 	if( mDirty )
// 	{
// 		mDirty = false;
// 		String text;
// 		for( auto& string : mOutput )
// 			text += string + '\n';
// 
// 		mText->clearText();
// 		mText->addText( text );
// 	}
// 
// 	mText->setVisible( isVisible() );

	Renderable::onAction( dt );
}

void Console::addLog(Log* l, bool getOldMessages /*= true */) {
	DEBUG_ASSERT(l, "Passed a NULL Log to a Console");

	pLogs.push_back(l);
	l->addListener(this);

	if (getOldMessages)
	{
		for (auto& entry : *l)
			onLogUpdated(l, entry);
	}
}

void Console::onLogUpdated(Log* l, const LogEntry& message) {
	mDirty = true;

	mOutput.push_back(message.text);

	if (mOutput.size() > (size_t)mMaxLines)
		mOutput.erase(mOutput.begin());
}

void Console::onDestruction() {
	for (auto log : pLogs)
		log->removeListener(this);
}
