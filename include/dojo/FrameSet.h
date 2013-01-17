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
	///A FrameSet represents a sequence of Textures, with an unique "prefix name" used by Animations and ResourceGroups (which do not manage Textures directly)
	class FrameSet : public Resource
	{
	public:
		
		typedef Array<Texture*> TextureList;
		
		///Creates a single FrameSet with the given "prefix name"
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

		///sets the "preferred animation time" of this FrameSet
		/**
		It is the frame time used if this FrameSet is used for an animation and an explicit frame time is not specified.
		*/
		inline void setPreferredAnimationTime( float t )
		{
			DEBUG_ASSERT( t > 0 );

			mPreferredAnimationTime = t;
		}
		
		///Loads this FrameSet from an Atlas, or a Texture+Definition Table combo
		/**
		the required texture must have been already loaded inside atlasTextureProvider
		*/
		void setAtlas( Table* atlasTable, ResourceGroup* atlasTextureProvider );

		virtual bool onLoad();
		
		virtual void onUnload( bool soft ) //unload all of the content
		{
			DEBUG_ASSERT( loaded );
			
			for( int i = 0; i < frames.size(); ++i )
				frames.at(i)->onUnload( soft );
			
			loaded = false;
		}
				
		///adds a texture to this frame set
		/*
		\param t the texture
		\param owner specifies  if this FrameSet is the only owner for garbage collection purposes
		*/
		inline void addTexture( Texture* t, bool owner = false )
		{
			DEBUG_ASSERT( t );
			DEBUG_ASSERT( !owner || (owner && t->getOwnerFrameSet() == NULL) );
			DEBUG_ASSERT( !isLoaded() || (t->isLoaded() && isLoaded()) );
			
			if( owner )
				t->_notifyOwnerFrameSet( this );
			
			frames.add( t );
		}
		
		///returns the (looped!) frame at index i
		/** if i > number of frames, i is looped over n as in i % size() */
		inline Texture* getFrame( uint i )			
		{	
			return frames.at( i%frames.size() );	
		}
		
		///returns a random frame
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
		
		///returns how many Textures this FrameSet contains
		inline uint getFrameNumber()				{	return frames.size();	}
		
		///returns the Frame Index of this Texture if it belongs to the FS, or -1 if not
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