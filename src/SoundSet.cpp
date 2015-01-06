#include "stdafx.h"

#include "SoundSet.h"
#include "SoundBuffer.h"

using namespace Dojo;

SoundSet::SoundSet(ResourceGroup* creator, const String& setName) :
Resource(creator),
name(setName),
buffers(1, 1) {

}

SoundBuffer* SoundSet::getBuffer(int i /*= -1 */) {
	DEBUG_ASSERT(buffers.size(), "This SoundSet is empty");
	DEBUG_ASSERT_INFO((int)buffers.size() > i, "Trying to get an OOB sound index", "index = " + String(i));

	if (i < 0)
	{
		if (buffers.size() > 1)
			i = (int)Math::rangeRandom(0, (float)buffers.size());
		else
			i = 0;
	}

	return buffers.at(i);
}

void SoundSet::addBuffer(SoundBuffer* b) {
	DEBUG_ASSERT(b, "Adding a NULL SoundBuffer");

	buffers.add(b);
}

bool SoundSet::onLoad()
{
	for( int i = 0; i < buffers.size(); ++i )
	{
		if( !buffers[i]->isLoaded() )
			buffers[i]->onLoad();
	}

	loaded = true;

	return true;
}

void SoundSet::onUnload( bool soft )
{
	for( int i = 0; i < buffers.size(); ++i )
	{
		if( buffers[i]->isLoaded() )
			buffers[i]->onUnload( soft );
	}

	loaded = false;
}