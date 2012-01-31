#include "stdafx.h"

#include "FrameSet.h"

#include "ResourceGroup.h"
#include "Platform.h"

using namespace Dojo;

bool FrameSet::load()
{			
	if( isLoaded() )
		return true;
	
	for( int i = 0; i < frames.size(); ++i )
	{
		Texture* t = frames[i];
		if( t->load() )
		{
			// count bytesize
			size += t->getByteSize();
		}
	}		
		
	loaded = true;		
		
	return true;	
}


bool FrameSet::loadAtlas( Table* data, ResourceGroup* atlasTextureProvider )
{
	DEBUG_ASSERT( data );
	DEBUG_ASSERT( atlasTextureProvider );

	if( isLoaded() )
		return true;
	
	//get atlas texture
	FrameSet* atlasSet = atlasTextureProvider->getFrameSet( data->getString( "texture" ) );	

	DEBUG_ASSERT( atlasSet );

	Texture* atlas = atlasSet->getFrame(0);

	Table* tiles = data->getTable( "tiles" );

	uint x, y, sx, sy;
	for( int i = 0; i < tiles->getAutoMembers(); ++i )
	{
		Table* tile = tiles->getTable( i );

		x = tile->getInt( 0 );
		y = tile->getInt( 1 );
		sx = tile->getInt( 2 );
		sy = tile->getInt( 3 );

		Texture* tiletex = new Texture( NULL, String::EMPTY );

		if( tiletex->loadFromAtlas( atlas, x,y, sx,sy ) )			
			addTexture( tiletex, true );
	}
		
	//loaded at least one?
	loaded = frames.size() > 0;
	
	return true;
}

