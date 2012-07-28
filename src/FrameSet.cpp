#include "stdafx.h"

#include "FrameSet.h"

#include "ResourceGroup.h"
#include "Platform.h"

using namespace Dojo;

void FrameSet::setAtlas( Table* atlasTable, ResourceGroup* atlasTextureProvider )
{
	DEBUG_ASSERT( !isLoaded() );

	FrameSet* atlasSet = atlasTextureProvider->getFrameSet( atlasTable->getString( "texture" ) );	

	DEBUG_ASSERT( atlasSet );

	Texture* atlas = atlasSet->getFrame(0);

	Table* tiles = atlasTable->getTable( "tiles" );

	uint x, y, sx, sy;
	for( int i = 0; i < tiles->getAutoMembers(); ++i )
	{
		Table* tile = tiles->getTable( i );

		x = tile->getInt( 0 );
		y = tile->getInt( 1 );
		sx = tile->getInt( 2 );
		sy = tile->getInt( 3 );

		Texture* tiletex = new Texture( NULL, String::EMPTY );

		tiletex->loadFromAtlas( atlas, x,y, sx,sy );

		addTexture( tiletex, true );
	}
}

bool FrameSet::onLoad()
{			
	DEBUG_ASSERT( !isLoaded() );
	
	loaded = true;
	for( int i = 0; i < frames.size(); ++i )
	{
		Texture* t = frames[i];
		if( !t->isLoaded() )
		{
			t->onLoad();

			loaded &= t->isLoaded();

			// count bytesize
			if( t->isLoaded() )
				size += t->getByteSize();
		}
	}
		
	return loaded;	
}


