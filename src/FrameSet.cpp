#include "stdafx.h"

#include "FrameSet.h"

#include "ResourceGroup.h"
#include "Platform.h"
#include "Texture.h"
#include "Table.h"

using namespace Dojo;

FrameSet::FrameSet(ResourceGroup* creator, const String& prefixName) :
Resource(creator),
name(prefixName),
mPreferredAnimationTime(0) {

}

FrameSet::~FrameSet() {
	//destroy child textures
	for (int i = 0; i < frames.size(); ++i)
		SAFE_DELETE(frames[i]);
}

void FrameSet::setPreferredAnimationTime(float t) {
	DEBUG_ASSERT(t > 0, "setPreferredAnimationTime: t must be more than 0");

	mPreferredAnimationTime = t;
}

void FrameSet::setAtlas( const Table& atlasTable, ResourceGroup& atlasTextureProvider )
{
	DEBUG_ASSERT( !isLoaded(), "setAtlas: this FrameSet is already loaded and can't be reset as an atlas" );

	String atlasName = atlasTable.getString( "texture" );
	FrameSet* atlasSet = atlasTextureProvider.getFrameSet( atlasName );	

	DEBUG_ASSERT_INFO( atlasSet, "The atlas Texture requested could not be found", "atlasName = " + atlasTable.getString( "texture" ) );

	Texture* atlas = atlasSet->getFrame(0);

	mPreferredAnimationTime = atlasTable.getNumber( "animationFrameTime" );
	
	auto& tiles = atlasTable.getTable( "tiles" );

	int x, y, sx, sy;
	for( int i = 0; i < tiles.getArrayLength(); ++i )
	{
		auto& tile = tiles.getTable( i );

		x = tile.getInt( 0 );
		y = tile.getInt( 1 );
		sx = tile.getInt( 2 );
		sy = tile.getInt( 3 );

		auto tiletex = make_unique<Texture>();

		tiletex->loadFromAtlas( atlas, x,y, sx,sy );

		addTexture( std::move(tiletex) );
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

void FrameSet::onUnload(bool soft) 
{
	DEBUG_ASSERT(loaded, "onUnload: this FrameSet is not loaded");

	for (int i = 0; i < frames.size(); ++i)
		frames.at(i)->onUnload(soft);

	loaded = false;
}

void FrameSet::addTexture(Texture& t) {
	frames.add(&t);
}

void FrameSet::addTexture(Unique<Texture> t) {
	DEBUG_ASSERT(t != nullptr, "Adding a NULL texture");
	DEBUG_ASSERT(t->getOwnerFrameSet() == NULL, "This Texture already has an owner FrameSet");
	
	t->_notifyOwnerFrameSet(this);

	addTexture(*t.release());
}

Texture* FrameSet::getRandomFrame() {
	return frames.at((int)Math::rangeRandom(0, (float)frames.size()));
}
