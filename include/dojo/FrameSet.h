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
	/** 
	there are two ways to specify a FrameSet:

	-the easier one is to name more than one image with a common prefix and a sequential tag, ie:
	ninja_walk_1.png
	ninja_walk_2.png
	ninja_walk_3.png

	-the advanced way is to create an .atlasinfo file, each one specifying more than one FrameSet with a Table array;
	this is much more efficient as it is faster to load, cleaner to mantain and faster to run (due to less texture switches)
	the single FrameSet is defined with 
	{
		name = "frame set name"
		texture = "atlas texture name"
		--tiles contains an array of tables representing rects, { x y w h }
		tiles = { { 0 0 128 128 } { 128 0 128 128 } { 256 0 128 128 } { 384 0 128 128 } }
		animationFrameTime = 0.15 --the preferred animation time
	}

	*/
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
			DEBUG_ASSERT( t > 0, "setPreferredAnimationTime: t must be more than 0" );

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
			DEBUG_ASSERT( loaded, "onUnload: this FrameSet is not loaded" );
			
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
			DEBUG_ASSERT( t != nullptr, "Adding a NULL texture" );
			DEBUG_ASSERT( !owner || (owner && t->getOwnerFrameSet() == NULL), "This Texture already has an owner FrameSet" );
			
			if( owner )
				t->_notifyOwnerFrameSet( this );
			
			frames.add( t );
		}
		
		///returns the (looped!) frame at index i
		/** 
		if i > number of frames, i is looped over n as in i % size() */
		inline Texture* getFrame( int i )			
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