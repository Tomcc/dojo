#include "stdafx.h"

#include "Texture.h"

#include "dojomath.h"

#include "Platform.h"

using namespace Dojo;

void Texture::bind( uint index )
{
	DEBUG_ASSERT( index < 8 );

	glActiveTexture( GL_TEXTURE0 + index );
	glEnable( GL_TEXTURE_2D );

	glBindTexture( GL_TEXTURE_2D, glhandle );
	
	//apply offset for the atlas if needed
	
	glMatrixMode( GL_TEXTURE );
	glLoadIdentity();
	
	if( isAtlasTile() )
	{
		glScalef( xRatio, yRatio, 1 );
		glTranslatef(xOffset, yOffset, 0 );
	}
}

void Texture::enableBilinearFiltering()
{					
    glBindTexture( GL_TEXTURE_2D, glhandle );
    
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
}

void Texture::disableBilinearFiltering()
{						
    glBindTexture( GL_TEXTURE_2D, glhandle );
    
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST ); 
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST );		
}

void Texture::enableTiling()
{
    glBindTexture( GL_TEXTURE_2D, glhandle );
    
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
}

void Texture::disableTiling()
{
    glBindTexture( GL_TEXTURE_2D, glhandle );
    
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
}

bool Texture::load()
{
	DEBUG_ASSERT( !loaded );
	DEBUG_ASSERT( textureType == "png" );
		
	glGenTextures(1, &glhandle );
	
	DEBUG_ASSERT( glhandle );
	
	enableTiling();
		
	
	loaded = _loadPNGToBoundTexture();

	if( !loaded )
		unload();
	
	//force disable filtering and alpha on too big textures
	enableBilinearFiltering();
		
	return loaded;
}

bool Texture::loadFromAtlas( Texture* tex, uint x, uint y, uint sx, uint sy )
{
	DEBUG_ASSERT( tex );
	DEBUG_ASSERT( tex->isLoaded() );
	DEBUG_ASSERT( !isLoaded() );	
			
	loaded = true;			
	textureType = "atlas";
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
		
		delete OBB;
		OBB = NULL;
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

bool Texture::_loadPNGToBoundTexture()
{
	void* imageData = NULL;
	
	Platform::getSingleton()->loadPNGContent( imageData, filePath, width, height );
		
	internalWidth = Math::nextPowerOfTwo( width );
	internalHeight = Math::nextPowerOfTwo( height );	
	
	npot = ( internalWidth != width || internalHeight != height );
	
	xRatio = (float)width/(float)internalWidth;
	yRatio = (float)height/(float)internalHeight;

	size = internalWidth * internalHeight * 4;
							 
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, internalWidth, internalHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
		
	free(imageData);
	
	return true;
}