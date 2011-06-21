/*
 *  Texture.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/23/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef Texture_h__
#define Texture_h__

#include "dojo_common_header.h"

#include <string>


#include "Utils.h"
#include "Vector.h"

#include "Buffer.h"
#include "Mesh.h"

namespace Dojo 
{
	class Mesh;
	class FrameSet;
	
	class Texture : public Buffer
	{
	public:
		 
		Texture( ResourceGroup* creator, const String& path ) :
		Buffer( creator, path ),
		width(0),
		height(0),
		internalWidth(0),
		internalHeight(0),
		xRatio(0),
		yRatio(0),
		xOffset(0),
		yOffset(0),
		glhandle( 0 ),
		npot( false ),
		parentAtlas( NULL ),
		OBB( NULL ),
		ownerFrameSet( NULL )
		{

		}
		
		virtual ~Texture()
		{
			if( loaded )
				unload();
		}
		

		///loads the texture from a memory area with RGBA8 format
		bool loadFromMemory( byte* buf, uint width, uint height );

		///loads the texture from the png pointed by the filename
		bool loadFromPNG( const String& path );
		
		///loads the texture from the given area in a Texture Atlas, without duplicating data
		bool loadFromAtlas( Texture* tex, uint x, uint y, uint sx, uint sy );

		///loads the texture with the given parameters
		bool load();

		void unload();
		
		bool isLoaded()			{	return loaded;			}
						
		virtual void bind( uint index );
		
		void enableBilinearFiltering();
		
		void disableBilinearFiltering();

		void enableMipmaps();

		void enableTiling();

		void disableTiling();
				
		inline uint getWidth()					{	return width;		}
		inline uint getHeight()					{	return height;		}
		
		inline uint getInternalWidth()
		{
			return internalWidth;
		}
		inline uint getInternalHeight()
		{
			return internalHeight;
		}
		
		inline Texture* getParentAtlas()		{	return parentAtlas;	}
		inline FrameSet* getOwnerFrameSet()		{	return ownerFrameSet;}
		
		inline const Vector& getScreenSize()	{	return screenSize;	}
		
		///obtain the optimal billboard to use this texture as a sprite!
		inline Mesh* getOptimalBillboard()
		{
			if( !OBB )
				_buildOptimalBillboard();
			
			return OBB;
		}
		
		inline bool isNonPowerOfTwo()			{	return npot;		}
		inline bool isAtlasTile()				{	return parentAtlas != NULL;	}
		
		inline void _notifyScreenSize( const Vector& ss )
		{
			screenSize.x = ss.x;
			screenSize.y = ss.y;
		}
		
		inline void _notifyOwnerFrameSet( FrameSet* s )
		{
			DEBUG_ASSERT( ownerFrameSet == NULL );
			DEBUG_ASSERT( s );
			
			ownerFrameSet = s;
		}
				
	protected:
				
		bool npot;
		uint width, height, internalWidth, internalHeight;
		float xRatio, yRatio;
		float xOffset, yOffset;
		
		Texture* parentAtlas;
		FrameSet* ownerFrameSet;
		
		Mesh* OBB;
		
		uint glhandle;
			
		Vector screenSize;

		///builds the optimal billboard for this texture, used in AnimatedQuads
		void _buildOptimalBillboard();
	};
}

#endif