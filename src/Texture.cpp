#include "stdafx.h"

#include "Texture.h"
#include "dojomath.h"
#include "Platform.h"
#include "Timer.h"

using namespace Dojo;

Texture::Texture( ResourceGroup* creator, const String& path ) :
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
	glGenTextures( 1, &glhandle );
	
	DEBUG_ASSERT( glhandle );
	
	glGetError();
}

void Texture::bind( uint index )
{
	DEBUG_ASSERT( index < 8 );

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
	bind(0);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
}

void Texture::disableMipmaps() 
{
	bind(0);
	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR );
}

bool Texture::loadFromMemory( Dojo::byte* imageData, uint width, uint height )
{
	int err;
	
	DEBUG_ASSERT( !loaded );
	DEBUG_ASSERT( imageData );

	bind(0);
	
	internalWidth = Math::nextPowerOfTwo( width );
	internalHeight = Math::nextPowerOfTwo( height );	

	npot = ( internalWidth != width || internalHeight != height );

	xRatio = (float)width/(float)internalWidth;
	yRatio = (float)height/(float)internalHeight;

	size = internalWidth * internalHeight * 4;

	glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	
	glTexImage2D(
		GL_TEXTURE_2D, 
		0, 
		GL_RGBA, 
		internalWidth, 
		internalHeight, 
		0, 
		GL_RGBA, 
		GL_UNSIGNED_BYTE, 
		imageData);

	err = glGetError();
	loaded = err == GL_NO_ERROR;

	DEBUG_ASSERT( loaded );
	
	return loaded;
}

bool Texture::loadFromPNG( const String& path )
{
	DEBUG_ASSERT( !loaded );
	
	void* imageData = NULL;

	enableBilinearFiltering();
	
	Platform::getSingleton()->loadPNGContent( imageData, path, width, height );
	
	//guess if this is a texture or a sprite
	if( width == Math::nextPowerOfTwo( width ) && height == Math::nextPowerOfTwo( height ) )
	{
		enableTiling();
		enableMipmaps();
	}
		
	loadFromMemory( (byte*)imageData, width, height );

	free(imageData);

	return loaded;
}

bool Texture::load()
{	
	DEBUG_ASSERT( !loaded );
	
	return loadFromPNG( filePath );
}

bool Texture::loadFromAtlas( Texture* tex, uint x, uint y, uint sx, uint sy )
{
	DEBUG_ASSERT( tex );
	DEBUG_ASSERT( tex->isLoaded() );
	DEBUG_ASSERT( !isLoaded() );	
			
	loaded = true;
	parentAtlas = tex;
	
	width = sx;
	height = sy;
	internalWidth = tex->internalWidth;
	internalHeight = tex->internalHeight;
	
	DEBUG_ASSERT( sx && sy && internalWidth && internalHeight );
		
	//copy bind handle
	glhandle = tex->glhandle;
	
	//find uv coordinates
	xOffset = (float)x/(float)internalWidth;
	yOffset = (float)y/(float)internalHeight;
	
	//find uv size
	xRatio = (float)sx/(float)internalWidth;
	yRatio = (float)sy/(float)internalHeight;
	
	return true;
}

void Texture::unload()
{		
	DEBUG_ASSERT( loaded );
	
	if( OBB )
	{
		OBB->unload();
		
		SAFE_DELETE( OBB );
	}
	
	if( !parentAtlas ) //don't unload parent texture!
	{
		DEBUG_ASSERT( glhandle );
		glDeleteTextures(1, &glhandle );
		
		glhandle = 0;
	}			
			
	loaded = false;
}


void Texture::_buildOptimalBillboard()
{
	OBB = new Mesh();
	
	OBB->setVertexFieldEnabled( Mesh::VF_POSITION2D );
	OBB->setVertexFieldEnabled( Mesh::VF_UV );
	
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
