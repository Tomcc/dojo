#include "stdafx.h"

#include "dojo/FrameSet.h"

#include "dojo/ResourceGroup.h"
#include "dojo/Platform.h"

using namespace Dojo;

bool FrameSet::load()
{			
	if( isLoaded() )
		return true;
	
	for( uint i = 0; i < frames.size(); ++i )
	{
		if( frames.at(i)->load() )
	
			// count bytesize
			size += frames.at(i)->getByteSize();
	}		
		
	loaded = true;		
		
	return true;	
}


bool FrameSet::loadAtlas( const String& infoFile, Texture* atlas )
{
	if( isLoaded() )
		return true;
	
	//can obtain the atlas?
	if( !creator && !atlas )
		return NULL;
	
	char* data; 
	uint size = Platform::getSingleton()->loadFileContent( data, infoFile );
	
	if( !data )
		return NULL;
	
	char* buf = data;
	char* end = data + size;
		
	//retrieve the correctly named texture
	if( !atlas )
	{		
		FrameSet* atlasSet = creator->getFrameSet( Utils::toString( buf , end ) );
		//check atlas existence
		if( !atlasSet || atlasSet->getFrameNumber() == 0 )
			return false;
		
		atlas = atlasSet->getFrame(0);
	}	
	uint x, y, sx, sy;
	Texture* tex;
	while( buf < end )
	{
		//for each 4 ints extracted, create a new tile texture
		x = Utils::toInt( buf, end );
		y = Utils::toInt( buf, end );
		
		sx = Utils::toInt( buf, end );
		sy = Utils::toInt( buf, end );
		
		tex = new Texture( creator, "none" );
		
		if( tex->loadFromAtlas( atlas, x,y, sx,sy ) )			
			addTexture( tex, true );
		
	}
	
	//loaded at least one?
	loaded = frames.size() > 0;
	
	free( data );
	
	return true;
}

