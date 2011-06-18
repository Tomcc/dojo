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

#include "dojo/dojo_common_header.h"

#include "dojo/Array.h"
#include "dojo/Math.h"
#include "dojo/Texture.h"

namespace Dojo 
{
	class FrameSet : public Buffer
	{
	public:
		
		typedef Array<Texture*> TextureList;
		
		//crea un set di frames col nome dato + _1, _2, _3...
		FrameSet( ResourceGroup* creator, const String& prefixName ) :
		Buffer( creator, prefixName )
		{
			
		}
		
		virtual ~FrameSet()
		{
			if( loaded )
				unload();
		}
		
		virtual bool load();
		
		bool loadAtlas( const String& infoFile, Texture* atlas = NULL );
		
		virtual void unload() //delete all of the content
		{
			DEBUG_ASSERT( loaded );
			
			for( uint i = 0; i < frames.size(); ++i )
			{
				if( frames.at(i)->getOwnerFrameSet() == this )
					delete frames.at(i);
			}				
			
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
			return frames.at( Math::rangeRandom( 0, frames.size() ) );
		}
		
		uint getFrameNumber()				{	return frames.size();	}
		
		int getFrameIndex( Texture* frame )
		{
			return frames.getElementIndex( frame );
		}
		
		
	protected:
		TextureList frames;
	};
}

#endif