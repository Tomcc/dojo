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

#include "dojo_config.h"

#include <string>

#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>

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
		 
		Texture( ResourceGroup* creator, const std::string& path ) :
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
		requiresAlpha( true ),
		OBB( NULL ),
		ownerFrameSet( NULL )
		{
			textureType = Utils::getFileExtension( filePath );
		}
		
		virtual ~Texture()
		{
			if( loaded )
				unload();
		}
		
		///loads the texture with the given parameters
		bool load();
		
		///loads the texture from the given area in a Texture Atlas, without duplicating data
		bool loadFromAtlas( Texture* tex, uint x, uint y, uint sx, uint sy );
		
		void unload();
		
		void setRequiresAlpha( bool a )	{	requiresAlpha = a;	}
		
		bool isLoaded()			{	return loaded;			}
		
		bool isAlphaRequired()	{	return requiresAlpha;	}
						
		virtual void bind()
		{
			glBindTexture( GL_TEXTURE_2D, glhandle );
		}
		
		inline void enableBilinearFiltering()
		{					
			glBindTexture( GL_TEXTURE_2D, glhandle );
			
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); 
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
		}
		
		inline void disableBilinearFiltering()
		{						
			glBindTexture( GL_TEXTURE_2D, glhandle );
			
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST ); 
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST );		
		}
		
		inline bool isNonPowerOfTwo()			{	return npot;		}
		
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
		
		inline float _getXTextureUVRatio()		{	return xRatio;		}
		inline float _getYTextureUVRatio()		{	return yRatio;		}
		
		inline float _getXTextureOffset()		{	return xOffset;		}
		inline float _getYTextureOffset()		{	return yOffset;		}
		
		inline void _notifyScreenSize( const Vector& ss )
		{
			screenSize.x = ss.x;
			screenSize.y = ss.y;
		}
		
		inline void _notifyOwnerFrameSet( FrameSet* s )
		{
			DOJO_ASSERT( ownerFrameSet == NULL );
			DOJO_ASSERT( s );
			
			ownerFrameSet = s;
		}
				
	protected:
				
		bool npot, requiresAlpha;
		uint width, height, internalWidth, internalHeight;
		float xRatio, yRatio;
		float xOffset, yOffset;
		
		Texture* parentAtlas;
		FrameSet* ownerFrameSet;
		
		Mesh* OBB;
		
		uint glhandle;
			
		Vector screenSize;
		
		std::string textureType;
						
		bool _loadPNGToBoundTexture( NSString* path );
		
		bool _loadPVRTCToBoundTexture( NSString* path );
		
		///builds the optimal billboard for this texture, used in AnimatedQuads
		void _buildOptimalBillboard();
	};
}

#endif