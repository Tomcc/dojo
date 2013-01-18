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
	xRatio(0),
	yRatio(0),
	xOffset(0),
	yOffset(0),
	glhandle( 0 ),
	npot( false ),
	parentAtlas( NULL ),
	OBB( NULL ),
	ownerFrameSet( NULL ),
	mMipmapsEnabled( true )
{			

}

Texture::Texture( ResourceGroup* creator, const String& path ) :
Resource( creator, path ),
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
ownerFrameSet( NULL ),
mMipmapsEnabled( true )
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
		glGetError();
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

bool Texture::loadFromMemory( Dojo::byte* imageData, int width, int height, GLenum sourceFormat, GLenum destFormat )
{
	int err;
	
	DEBUG_ASSERT( imageData );
	DEBUG_ASSERT( width > 0 );
	DEBUG_ASSERT( height > 0 ); 

	bind(0);
	
	byte* paddedData = NULL;
	
	//Assume formats are either GL_RGB or GL_RGBA
	int sourcePixelSize = (sourceFormat == GL_RGBA) ? 4 : 3;
	int destPixelSize = (destFormat == GL_RGBA) ? 4 : 3;

	int POTwidth = Math::nextPowerOfTwo( width );
	int POTheight = Math::nextPowerOfTwo( height );

	//if the platforms supports NPOT, or the dimensions are already POT, direct copy
	if( (width == POTwidth && height == POTheight ) || Platform::getSingleton()->isNPOTEnabled() )
	{
		internalWidth = width;
		internalHeight = height;
	}
	else
	{
		internalWidth = POTwidth;
		internalHeight = POTheight;
		
		//need to pad imageData to a POT buffer
		paddedData = (byte*)malloc( internalWidth * internalHeight * sourcePixelSize );
		
		memset( paddedData, 0, internalWidth * internalHeight * sourcePixelSize );
		for( int i = 0; i < height; ++i )
			memcpy( paddedData + i*internalWidth*sourcePixelSize, 
					imageData + i*width*sourcePixelSize, 
					width*sourcePixelSize );
		
		imageData = paddedData;
	}
												
	xRatio = (float)width/(float)internalWidth;
	yRatio = (float)height/(float)internalHeight;	
	npot = ( Math::nextPowerOfTwo( width ) != width || Math::nextPowerOfTwo( height ) != height );
	
	size = internalWidth * internalHeight * destPixelSize;
	
	glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, mMipmapsEnabled );
	
	//HACK
	glTexImage2D(
		GL_TEXTURE_2D, 
		0, 
		destFormat,
		internalWidth, 
		internalHeight, 
		0, 
		sourceFormat,
		GL_UNSIGNED_BYTE, 
		imageData);

	err = glGetError();
	loaded = (err == GL_NO_ERROR);

	DEBUG_ASSERT( loaded );
	
	//aaand now, kill the temp padding buffer
	if( paddedData )
		free( paddedData );
	
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
	xOffset = (float)mAtlasOriginX/(float)internalWidth;
	yOffset = (float)mAtlasOriginY/(float)internalHeight;

	//find uv size
	xRatio = (float)width/(float)internalWidth;
	yRatio = (float)height/(float)internalHeight;

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
			OBB->onUnload(); //always destroy the OBB

		if( !parentAtlas ) //don't unload parent texture!
		{
			DEBUG_ASSERT( glhandle );
			glDeleteTextures(1, &glhandle );
		
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
	OBB->uv( xOffset, 
			 yOffset + yRatio );
	
	OBB->vertex( 0.5, -0.5 );		
	OBB->uv( xOffset + xRatio, 
			 yOffset + yRatio );
	
	OBB->vertex( -0.5, 0.5 );		
	OBB->uv( xOffset, 
			 yOffset );
	
	OBB->vertex( 0.5, 0.5 );
	OBB->uv( xOffset + xRatio, 
			 yOffset );
	
	OBB->end();			
}
