#include "stdafx.h"

#include "SoundSet.h"
#include "SoundBuffer.h"

using namespace Dojo;

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