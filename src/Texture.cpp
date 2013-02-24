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
	internalFormat( GL_NONE )
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
internalFormat( GL_NONE )
{			

}

Texture::~Texture()
{
	if( OBB )
		SAFE_DELETE( OBB );
}

void Texture::bind( uint index )
{
	DEBUG_ASSERT( index < DOJO_MAX_TEXTURE_UNITS );
	
	//create the gl texture if still not created!
	if( !glhandle )
	{
		glGenTextures( 1, &glhandle );
		DEBUG_ASSERT( glGetError() == GL_NO_ERROR );
		DEBUG_ASSERT( glhandle );
	}

	glActiveTexture( GL_TEXTURE0 + index );
	glEnable( GL_TEXTURE_2D );

	glBindTexture( GL_TEXTURE_2D, glhandle );
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
	
	if( glhandle )
	{
		bind(0);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	}
}

bool Texture::loadEmpty( int width, int height, GLenum destFormat )
{
	DEBUG_ASSERT( width > 0 );
	DEBUG_ASSERT( height > 0 ); 

	bind(0);

	//Assume formats are either GL_RGB or GL_RGBA
	int destPixelSize = (destFormat == GL_RGBA) ? 4 : 3;

	int POTwidth = Math::nextPowerOfTwo( width );
	int POTheight = Math::nextPowerOfTwo( height );

	npot = width != POTwidth && height != POTheight;

	int destWidth, destHeight;

	//if the platforms supports NPOT, or the dimensions are already POT, direct copy
	if( !npot || Platform::getSingleton()->isNPOTEnabled() )
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

		//create an empty GPU mem space
		glTexImage2D(
			GL_TEXTURE_2D, 
			0, 
			internalFormat,
			internalWidth, 
			internalHeight,
			0, 
			internalFormat, //it's not like we're loading anything anyway
			GL_UNSIGNED_BYTE, 
			NULL );
	}

	UVSize.x = (float)width/(float)internalWidth;
	UVSize.y = (float)height/(float)internalHeight;

	loaded = (glGetError() == GL_NO_ERROR);
	DEBUG_ASSERT( loaded );
	return loaded;
}

bool Texture::loadFromMemory( byte* imageData, int width, int height, GLenum sourceFormat, GLenum destFormat )
{
	DEBUG_ASSERT( imageData );

	loadEmpty( width, height, destFormat );
	
	glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, mMipmapsEnabled );

	//paste the actual data inside the image, works with NPOT devices too
	glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, width, height, sourceFormat, GL_UNSIGNED_BYTE, imageData );

	loaded = (glGetError() == GL_NO_ERROR);
	DEBUG_ASSERT( loaded );	
	return loaded;
}

bool Texture::loadFromFile( const String& path )
{
	DEBUG_ASSERT( !loaded );
	
	void* imageData = NULL;

	GLenum sourceFormat = 0, destFormat;
	int pixelSize;
	sourceFormat = Platform::getSingleton()->loadImageFile( imageData, path, width, height, pixelSize );
	
	DEBUG_ASSERT( sourceFormat );
	
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
	DEBUG_ASSERT( parentAtlas );

	if( !parentAtlas->isLoaded() )
		return (loaded = false);

	internalWidth = parentAtlas->getInternalWidth();
	internalHeight = parentAtlas->getInternalHeight();

	DEBUG_ASSERT( width && height && internalWidth && internalHeight );

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

bool Texture::loadFromAtlas( Texture* tex, uint x, uint y, uint sx, uint sy )
{
	DEBUG_ASSERT( tex );
	DEBUG_ASSERT( !isLoaded() );	

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
	DEBUG_ASSERT( !loaded );

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
	DEBUG_ASSERT( loaded );
	
	if( !soft || isReloadable() )
	{
		if( OBB )
		{
			OBB->onUnload();
			SAFE_DELETE( OBB );
		}

		if( !parentAtlas ) //don't unload parent texture!
		{
			DEBUG_ASSERT( glhandle );
			glDeleteTextures(1, &glhandle );

			internalWidth = internalHeight = 0;
			internalFormat = GL_NONE;
			glhandle = 0;
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
		OBB->setVertexFieldEnabled( Mesh::VF_POSITION2D );
		OBB->setVertexFieldEnabled( Mesh::VF_UV );
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
