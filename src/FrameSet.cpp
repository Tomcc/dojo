#include "stdafx.h"

#include "FrameSet.h"

#include "ResourceGroup.h"
#include "Platform.h"

using namespace Dojo;

void FrameSet::setAtlas( Table* atlasTable, ResourceGroup* atlasTextureProvider )
{
	DEBUG_ASSERT( !isLoaded(), "setAtlas: this FrameSet is already loaded and can't be reset as an atlas" );

	String atlasName = atlasTable->getString( "texture" );
	FrameSet* atlasSet = atlasTextureProvider->getFrameSet( atlasName );	

	DEBUG_ASSERT_INFO( atlasSet, "The atlas Texture requested could not be found", "atlasName = " + atlasTable->getString( "texture" ) );

	Texture* atlas = atlasSet->getFrame(0);

	mPreferredAnimationTime = atlasTable->getNumber( "animationFrameTime" );
	
	Table* tiles = atlasTable->getTable( "tiles" );

	int x, y, sx, sy;
	for( int i = 0; i < tiles->getAutoMembers(); ++i )
	{
		Table* tile = tiles->getTable( i );

		x = tile->getInt( 0 );
		y = tile->getInt( 1 );
		sx = tile->getInt( 2 );
		sy = tile->getInt( 3 );

		Texture* tiletex = new Texture();

		tiletex->loadFromAtlas( atlas, x,y, sx,sy );

		addTexture( tiletex, true );
	}
}

bool FrameSet::onLoad()
{			
	DEBUG_ASSERT( !isLoaded(), "onLoad: this FrameSet is already loaded" );
	
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


