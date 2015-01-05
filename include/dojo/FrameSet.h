/*
 *  Animation.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/25/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "dojo_common_header.h"

#include "Array.h"
#include "Resource.h"

namespace Dojo 
{
	class Texture;
	class Table;

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
		FrameSet( ResourceGroup* creator, const String& prefixName );
		
		virtual ~FrameSet();

		///sets the "preferred animation time" of this FrameSet
		/**
		It is the frame time used if this FrameSet is used for an animation and an explicit frame time is not specified.
		*/
		void setPreferredAnimationTime( float t );
		
		///Loads this FrameSet from an Atlas, or a Texture+Definition Table combo
		/**
		the required texture must have been already loaded inside atlasTextureProvider
		*/
		void setAtlas( const Table& atlasTable, ResourceGroup& atlasTextureProvider );

		virtual bool onLoad();
		
		///unload all of the content;
		virtual void onUnload(bool soft);
				
		///adds a non-owned texture to this frame set
		/*
		\param t the texture
		*/
		void addTexture( Texture& t );

		///adds a owned texture to this frame set
		/*
		\param t the texture
		*/
		void addTexture(Unique<Texture> t);
		
		///returns the (looped!) frame at index i
		/** 
		if i > number of frames, i is looped over n as in i % size() */
		Texture* getFrame( int i )			
		{	
			return frames.at( i%frames.size() );	
		}
		
		///returns a random frame
		Texture* getRandomFrame();

		float getPreferredAnimationTime()
		{
			return mPreferredAnimationTime;
		}

		bool hasPreferredAnimationTime()
		{
			return mPreferredAnimationTime >= 0;
		}
		
		///returns how many Textures this FrameSet contains
		int getFrameNumber()				{	return frames.size();	}
		
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

