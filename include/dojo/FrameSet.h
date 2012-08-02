/*
 *  Animation.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/25/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef FrameSet_h__
#define FrameSet_h__

#include "dojo_common_header.h"

#include "Array.h"
#include "dojomath.h"
#include "Texture.h"
#include "Table.h"

namespace Dojo 
{
	class FrameSet : public Resource
	{
	public:
		
		typedef Array<Texture*> TextureList;
		
		//crea un set di frames col nome dato + _1, _2, _3...
		FrameSet( ResourceGroup* creator, const String& prefixName ) :
		Resource( creator ),
		name( prefixName ),
		mPreferredAnimationTime( 0 )
		{
			
		}
		
		virtual ~FrameSet()
		{
			//destroy child textures
			for( int i = 0; i < frames.size(); ++i )
				SAFE_DELETE( frames[i] );
		}

		inline void setPreferredAnimationTime( float t )
		{
			DEBUG_ASSERT( t > 0 );

			mPreferredAnimationTime = t;
		}
		
		void setAtlas( Table* atlasTable, ResourceGroup* atlasTextureProvider );

		virtual bool onLoad();
		
		virtual void onUnload( bool soft ) //unload all of the content
		{
			DEBUG_ASSERT( loaded );
			
			for( int i = 0; i < frames.size(); ++i )
				frames.at(i)->onUnload( soft );
			
			loaded = false;
		}
				
		///adds a texture to this frame set, specifying if this frameset is the only owner
		inline void addTexture( Texture* t, bool owner = false )
		{
			DEBUG_ASSERT( t );
			DEBUG_ASSERT( !owner || (owner && t->getOwnerFrameSet() == NULL) );
			DEBUG_ASSERT( !isLoaded() || (t->isLoaded() && isLoaded()) );
			
			if( owner )
				t->_notifyOwnerFrameSet( this );
			
			frames.add( t );
		}
		
		//return the looped frame
		inline Texture* getFrame( uint i )			
		{	
			return frames.at( i%frames.size() );	
		}
		
		//return a random frame
		inline Texture* getRandomFrame()
		{
			return frames.at( (int)Math::rangeRandom( 0, (float)frames.size() ) );
		}

		inline float getPreferredAnimationTime()
		{
			return mPreferredAnimationTime;
		}

		inline bool hasPreferredAnimationTime()
		{
			return mPreferredAnimationTime >= 0;
		}
		
		uint getFrameNumber()				{	return frames.size();	}
		
		int getFrameIndex( Texture* frame )
		{
			return frames.getElementIndex( frame );
		}
		
		
	protected:
		String name;

		float mPreferredAnimationTime;

		TextureList frames;
	};
}

#endif