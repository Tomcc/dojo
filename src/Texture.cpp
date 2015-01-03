#include "stdafx.h"

#include "Texture.h"
#include "dojomath.h"
#include "Platform.h"
#include "ResourceGroup.h"
#include "Mesh.h"

using namespace Dojo;

Texture::Texture( ResourceGroup* creator ) :
	Resource( creator ),
	width(0),
	height(0),
	internalWidth(0),
	internalHeight(0),
	glhandle( 0 ),
	npot( false ),
	parentAtlas( NULL ),
	OBB( NULL ),
	ownerFrameSet( NULL ),
	mMipmapsEnabled( true ),
	internalFormat( GL_NONE ),
	mFBO( GL_NONE )
{			

}

Texture::Texture( ResourceGroup* creator, const String& path ) :
	Resource( creator, path ),
	width(0),
	height(0),
	internalWidth(0),
	internalHeight(0),
	glhandle( 0 ),
	npot( false ),
	parentAtlas( NULL ),
	OBB( NULL ),
	ownerFrameSet( NULL ),
	mMipmapsEnabled( true ),
	internalFormat( GL_NONE ),
	mFBO( GL_NONE )
{			

}

Texture::~Texture()
{
	if( OBB )
		SAFE_DELETE( OBB );

	if (loaded)
		onUnload();
}

void Texture::bind( GLuint index )
{	
	//create the gl texture if still not created!
	if( !glhandle )
	{
		glGenTextures( 1, &glhandle );

		CHECK_GL_ERROR;

		DEBUG_ASSERT( glhandle, "OpenGL Error, no texture handle was generated" );
	}

	glActiveTexture( GL_TEXTURE0 + index );
	glEnable( GL_TEXTURE_2D );

	glBindTexture( GL_TEXTURE_2D, glhandle );
}

void Texture::bindAsRenderTarget( bool depthBuffer )
{
	DEBUG_ASSERT(!mMipmapsEnabled, "Can't use a texture with mipmaps as a rendertarget");

	if( !mFBO ) //create a new RT on the fly at the first request
	{
		bind(0);

		glGenFramebuffers(1, &mFBO); 
		glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

		CHECK_GL_ERROR;

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glhandle, 0);

		CHECK_GL_ERROR;

		//create the depth attachment if needed
		if( depthBuffer )
		{
			glGenRenderbuffersEXT(1, &mDepthBuffer);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mDepthBuffer);
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT16, width, height);
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, mDepthBuffer);
		}

		CHECK_GL_ERROR;

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		DEBUG_ASSERT( status == GL_FRAMEBUFFER_COMPLETE, "The framebuffer is incomplete" );
	}
	else
		glBindFramebuffer( GL_FRAMEBUFFER, mFBO );	
}

void Texture::enableAnisotropicFiltering( float level )
{
	bind(0);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, level );
}

void Texture::disableAnisotropicFiltering()
{
	bind(0);
	glTexParameterf( GL_TEXTURE_2D, GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, 0 );
}

void Texture::enableBilinearFiltering()
{					
	bind(0);
	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

}

void Texture::disableBilinearFiltering()
{						
	bind(0);
	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST );		
}

void Texture::enableTiling()
{
	bind(0);
	
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
}

void Texture::disableTiling()
{
	bind(0);
	
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
}

void Texture::enableMipmaps() 
{
	mMipmapsEnabled = true;
	
	if( glhandle )
	{
		bind(0);		
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );	
	}
}

void Texture::disableMipmaps() 
{
	mMipmapsEnabled = false;

	if (glhandle)
	{
		bind(0);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	}
}

bool Texture::loadEmpty( int w, int h, GLenum destFormat )
{
	width = w;
	height = h;

	DEBUG_ASSERT( width > 0, "Width must be more than 0" );
	DEBUG_ASSERT( height > 0, "Height must be more than 0" ); 
	DEBUG_ASSERT( destFormat > 0, "the desired internal image format is undefined" );

	bind(0);

	if (mMipmapsEnabled)
		glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, true );
	else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//Assume formats are either GL_RGB or GL_RGBA
	int destPixelSize = (destFormat == GL_RGBA) ? 4 : 3;

	int POTwidth = Math::nextPowerOfTwo( width );
	int POTheight = Math::nextPowerOfTwo( height );

	npot = width != POTwidth && height != POTheight;

	int destWidth, destHeight;

	//if the platforms supports NPOT, or the dimensions are already POT, direct copy
	if( !npot || Platform::singleton().isNPOTEnabled() )
	{
		destWidth = width;
		destHeight = height;
	}
	else
	{
		destWidth = POTwidth;
		destHeight = POTheight;
	}

	//check if the texture has to be recreated (changed dimensions)
	if( destWidth != internalWidth || destHeight != internalHeight || internalFormat != destFormat )
	{
		internalWidth = destWidth;
		internalHeight = destHeight;
		internalFormat = destFormat;
		size = internalWidth * internalHeight * destPixelSize;

        std::string dummyData(size, 0 ); //needs to preallocate the storage if this tex is used as rendertarget (TODO avoid this if we have data)
        
        //create an empty GPU mem space
		glTexImage2D(
			GL_TEXTURE_2D, 
			0, 
			internalFormat,
			internalWidth, 
			internalHeight,
			0, 
			internalFormat,
			GL_UNSIGNED_BYTE, 
			dummyData.c_str() );
	}

	UVSize.x = (float)width/(float)internalWidth;
	UVSize.y = (float)height/(float)internalHeight;

	GLenum err = glGetError();
	loaded = (err == GL_NO_ERROR);
	DEBUG_ASSERT( loaded, "Cannot load an empty texture" );

	return loaded;
}

bool Texture::loadFromMemory( byte* imageData, int width, int height, GLenum sourceFormat, GLenum destFormat )
{
	DEBUG_ASSERT( imageData, "null image data" );

	loadEmpty( width, height, destFormat );

	glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, width, height, sourceFormat, GL_UNSIGNED_BYTE, imageData );

	loaded = (glGetError() == GL_NO_ERROR);
	DEBUG_ASSERT( loaded, "OpenGL error, cannot load a Texture from memory" );	
	return loaded;
}

bool Texture::loadFromFile( const String& path )
{
	DEBUG_ASSERT( !isLoaded(), "The Texture is already loaded" );
	
	void* imageData = NULL;

	GLenum sourceFormat = 0, destFormat;
	int pixelSize;
	sourceFormat = Platform::singleton().loadImageFile( imageData, path, width, height, pixelSize );
	
	DEBUG_ASSERT_INFO( sourceFormat, "Cannot load an image file", "path = " + path );
	
	if( creator && creator->disableBilinear )	
		disableBilinearFiltering();
	else
		enableBilinearFiltering();
	
	bool isSurface = width == Math::nextPowerOfTwo( width ) && height == Math::nextPowerOfTwo( height );
	
	//guess if this is a texture or a sprite
	if( !isSurface || (creator && creator->disableMipmaps ) )
		disableMipmaps();
	else
		enableMipmaps();
	
	if( !isSurface || (creator && creator->disableTiling ) )
		disableTiling();
	else
		enableTiling();

	if( isSurface ) //TODO query anisotropic level
	{
		GLfloat aniso;
		glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso );
		enableAnisotropicFiltering( aniso/2 );
	}
	
	destFormat = sourceFormat;
	
#ifdef DOJO_GAMMA_CORRECTION_ENABLED
	if( sourceFormat == GL_RGBA )		destFormat = GL_SRGB8_ALPHA8;
	else if( sourceFormat == GL_RGB )	destFormat = GL_SRGB8;
#endif
		
	loadFromMemory( (byte*)imageData, width, height, sourceFormat, destFormat );

	free(imageData);

	return loaded;
}

bool Texture::_setupAtlas()
{
	DEBUG_ASSERT( parentAtlas, "Tried to load a Texture as an atlas tile but the parent atlas is null" );

	if( !parentAtlas->isLoaded() )
		return (loaded = false);

	internalWidth = parentAtlas->getInternalWidth();
	internalHeight = parentAtlas->getInternalHeight();

	DEBUG_ASSERT( width > 0 && height > 0 && internalWidth > 0 && internalHeight > 0, "One or more texture dimensions are invalid (less or equal to 0)" );

	//copy bind handle
	glhandle = parentAtlas->glhandle;

	//find uv coordinates
	UVOffset.x = (float)mAtlasOriginX/(float)internalWidth;
	UVOffset.y = (float)mAtlasOriginY/(float)internalHeight;

	//find uv size
	UVSize.x = (float)width/(float)internalWidth;
	UVSize.y = (float)height/(float)internalHeight;

	return (loaded = true);
}

bool Texture::loadFromAtlas( Texture* tex, int x, int y, int sx, int sy )
{
	DEBUG_ASSERT( tex, "null atlas texture" );
	DEBUG_ASSERT( !isLoaded(), "The Texture is already loaded" );	

	parentAtlas = tex;

	width = sx;
	height = sy;

	mAtlasOriginX = x;
	mAtlasOriginY = y;

	//actual lazy loading is in _setupAtlas
	
	return false;
}

bool Texture::onLoad()
{	
	DEBUG_ASSERT( !isLoaded(), "The texture is already loaded" );

	if( OBB )  //rebuild and reload the OBB if it was purged
		_buildOptimalBillboard();
	
	if( isReloadable() )
		return loadFromFile( filePath );
	else if( parentAtlas )
		return _setupAtlas();
	else
		return false;
}

void Texture::onUnload( bool soft )
{		
	DEBUG_ASSERT( isLoaded(), "The Texture is not loaded" );
	
	if( !soft || isReloadable() )
	{
		if( OBB )
		{
			OBB->onUnload();
		}

		if( !parentAtlas ) //don't unload parent texture!
		{
			DEBUG_ASSERT( glhandle, "Tried to unload a texture but the texture handle was invalid" );
			glDeleteTextures(1, &glhandle );

			internalWidth = internalHeight = 0;
			internalFormat = GL_NONE;
			glhandle = 0;

			if( mFBO ) //fbos are destroyed on unload, the user must care to rebuild their contents after a purge
			{
				glDeleteFramebuffers( 1, &mFBO );
				mFBO = GL_NONE;
			}
		}

		loaded = false;
	}
}


void Texture::_buildOptimalBillboard()
{
	if( !OBB )
	{
		OBB = new Mesh();
	
		//build or rebuild the OBB
		OBB->setVertexFields({ VertexField::Position2D, VertexField::UV0 });
	}
	
	OBB->begin( 4 );
	
	OBB->vertex( -0.5, -0.5 );		
	OBB->uv( UVOffset.x, 
			 UVOffset.y + UVSize.y );
	
	OBB->vertex( 0.5, -0.5 );		
	OBB->uv( UVOffset.x + UVSize.x, 
			 UVOffset.y + UVSize.y );
	
	OBB->vertex( -0.5, 0.5 );		
	OBB->uv( UVOffset.x, 
			 UVOffset.y );
	
	OBB->vertex( 0.5, 0.5 );
	OBB->uv( UVOffset.x + UVSize.x, 
			 UVOffset.y );
	
	OBB->end();			
}
