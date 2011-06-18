#include "stdafx.h"

#include "dojo/SoundBuffer.h"
#include "dojo/SoundManager.h"

#include "dojo/Utils.h"


using namespace Dojo;

#define OGG_ENDIAN 0

///////////////////////////////////////

SoundBuffer::SoundBuffer( ResourceGroup* creator, const String& path ) :
Buffer( creator, path ),
size(0),
buffer( AL_NONE )
{
	DEBUG_ASSERT( creator );
}

SoundBuffer::~SoundBuffer()
{
	unload();
}

bool SoundBuffer::load()
{
	if( isLoaded() )	return false;

	alGenBuffers(1,&buffer);
	
	DEBUG_ASSERT( buffer );

	ALenum error = alGetError();
	
	size = Platform::getSingleton()->loadAudioFileContent( buffer, filePath );
		
	//error check
	if( error == AL_NO_ERROR )
		error = alGetError();

	if( error != AL_NO_ERROR )
	{
		alDeleteBuffers(1, &buffer);
		buffer = AL_NONE;
	}

	return error != AL_NO_ERROR;
}

void SoundBuffer::unload()
{
	if( isLoaded() )
	{
		alDeleteBuffers(1, &buffer);
		buffer = AL_NONE;
	}
}