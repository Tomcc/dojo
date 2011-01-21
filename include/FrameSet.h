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

#include "dojo_config.h"

#include <string>

#include "Array.h"
#include "dojomath.h"
#include "Texture.h"

namespace Dojo 
{
	class FrameSet : public Buffer
	{
	public:
		
		typedef Array<Texture*> TextureList;
		
		//crea un set di frames col nome dato + _1, _2, _3...
		FrameSet( ResourceGroup* creator, const std::string& prefixName ) :
		Buffer( creator, prefixName )
		{
			
		}
		
		virtual ~FrameSet()
		{
			if( loaded )
				unload();
			
			//release all the textures
			//HACK
			/*for( uint i = 0; i < frames.size(); ++i )
			{
				Texture* frame = frames.at(i);
				
				if( frame->getOwnerFrameSet() == this )
					delete frame;
			}			*/	
		}
		
		virtual bool load();
		
		bool loadAtlas( const std::string& infoFile, Texture* atlas = NULL );
		
		virtual void unload() //delete all of the content
		{
			DOJO_ASSERT( loaded );
			
			for( uint i = 0; i < frames.size(); ++i )
			{
				if( frames.at(i)->getOwnerFrameSet() == this && frames.at(i)->isLoaded() )
					frames.at(i)->unload();
			}				
			
			loaded = false;
		}
		
		///just binds the first frame!
		virtual void bind()
		{
			DOJO_ASSERT( frames.at(0) );
			
			if( loaded )
				frames.at(0)->bind();
		}
		
		///adds a texture to this frame set, specifying if this frameset is the only owner
		inline void addTexture( Texture* t, bool owner = false )
		{
			DOJO_ASSERT( t );
			DOJO_ASSERT( !owner || (owner && t->getOwnerFrameSet() == NULL) );
			DOJO_ASSERT( !isLoaded() || (t->isLoaded() && isLoaded()) );
			
			if( owner )
				t->_notifyOwnerFrameSet( this );
			
			frames.addElement( t );
		}
		
		//return the looped frame
		inline Texture* getFrame( uint i )			
		{	
			return frames.at( i%frames.size() );	
		}
		
		//return a random frame
		inline Texture* getRandomFrame()
		{
			return frames.at( Math::rangeRandom(0, frames.size() ) );
		}
		
		uint getFrameNumber()				{	return frames.size();	}
		
		
	protected:
		TextureList frames;
	};
}

#endif