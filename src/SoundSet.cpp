#include "stdafx.h"

#include "SoundSet.h"
#include "SoundBuffer.h"

using namespace Dojo;

bool SoundSet::load()
{
	for( int i = 0; i < buffers.size(); ++i )
		buffers[i]->load();

	loaded = true;

	return true;
}

void SoundSet::unload()
{
	for( int i = 0; i < buffers.size(); ++i )
		buffers[i]->unload();

	loaded = false;
}