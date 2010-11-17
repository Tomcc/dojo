//
//  Texture.mm
//  NinjaTraining
//
//  Created by Tommaso Checchi on 4/25/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#include "Texture.h"

#include "Math.h"

#import <OpenGLES/ES1/glext.h>

using namespace Dojo;

bool Texture::load()
{
	DOJO_ASSERT( !loaded );
		
	glGenTextures(1, &glhandle );
	
	DOJO_ASSERT( glhandle );
	
	glBindTexture( GL_TEXTURE_2D, glhandle );
		
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		
	NSString *path = Utils::toNSString( filePath );
	
	if( textureType == "png" )
		loaded = _loadPNGToBoundTexture( path );
	
	else if( textureType == "pvrtc" )
		loaded = _loadPVRTCToBoundTexture( path );
	
	if( !loaded )
		unload();
	
	//force disable filtering and alpha on too big textures
	if( width > 128 || height > 128 )
	{
		requiresAlpha = false;
		
		disableBilinearFiltering();
	}
	else
	{		
		enableBilinearFiltering();
	}
	
	//compute size
	size = internalWidth * internalHeight * sizeof( float ) * 2;
	
	return loaded;
}

bool Texture::loadFromAtlas( Texture* tex, uint x, uint y, uint sx, uint sy )
{
	DOJO_ASSERT( tex );
	DOJO_ASSERT( tex->isLoaded() );
	DOJO_ASSERT( !isLoaded() );	
			
	loaded = true;			
	textureType = "atlas";
	parentAtlas = tex;
	
	width = sx;
	height = sy;
	internalWidth = tex->internalWidth;
	internalHeight = tex->internalHeight;
	
	DOJO_ASSERT( sx && sy && internalWidth && internalHeight );
	
	//bigger tiles have alpha disabled by default
	requiresAlpha = sx < 128 || sy < 128;			
	
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
	DOJO_ASSERT( loaded );
	
	if( OBB )
	{
		OBB->unload();
		
		delete OBB;
		OBB = NULL;
	}
	
	if( !parentAtlas ) //don't unload parent texture!
	{
		DOJO_ASSERT( glhandle );
		glDeleteTextures(1, &glhandle );
		
		glhandle = 0;
	}			
			
	loaded = false;
}

bool Texture::_loadPNGToBoundTexture( NSString* path )
{
	//magic OBJC code
	NSData *texData = [[NSData alloc] initWithContentsOfFile:path];
	UIImage *image = [[UIImage alloc] initWithData:texData];
	if (image == nil)
	{
		NSLog(@"Error loading image");
		return false;
	}
	
	width = CGImageGetWidth(image.CGImage);
	height = CGImageGetHeight(image.CGImage);
	uint channels = 4;//CGImageGetBitsPerPixel(image.CGImage) / 8;
	
	internalWidth = Math::nextPowerOfTwo( width );
	internalHeight = Math::nextPowerOfTwo( height );
	
	xRatio = (float)width/(float)internalWidth;
	yRatio = (float)height/(float)internalHeight;
	
	npot = ( internalWidth != width || internalHeight != height );
	
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	void *imageData = malloc( internalWidth * internalHeight * channels );
	memset( imageData, 0, internalWidth * internalHeight * channels );
	
	CGContextRef context = CGBitmapContextCreate(imageData, 
												 internalWidth, 
												 internalHeight, 
												 8, 
												 channels * internalWidth, 
												 colorSpace, 
												 kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big );
	CGColorSpaceRelease( colorSpace );
	CGContextClearRect( context, CGRectMake( 0, 0, internalWidth, internalHeight ) );
	CGContextTranslateCTM( context, 0, internalHeight - height );
	CGContextDrawImage( context, CGRectMake( 0, 0, width, height ), image.CGImage );
	
	GLenum format = (channels==4) ? GL_RGBA : GL_RGB;
	
	//correct premultiplied alpha
	int pixelcount = internalWidth*internalHeight;
	unsigned char* off = (unsigned char*)imageData;
	for( int pi=0; pi<pixelcount; ++pi )
	{
		unsigned char alpha = off[3];
		if( alpha!=255 && alpha!=0 )
		{
			off[0] = ((int)off[0])*255/alpha;
			off[1] = ((int)off[1])*255/alpha;
			off[2] = ((int)off[2])*255/alpha;
		}
		off += 4;
	}
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, internalWidth, internalHeight, 0, format, GL_UNSIGNED_BYTE, imageData);
	
	CGContextRelease(context);
	
	free(imageData);
	
	//HACK
	/*[image release];
	[texData release];*/
	
	return true;
}

void Texture::_buildOptimalBillboard()
{
	OBB = new Mesh();
	
	OBB->setVertexFieldEnabled( Mesh::VF_POSITION2D, true );
	OBB->setVertexFieldEnabled( Mesh::VF_UV, true );
	
	OBB->begin( 4 );
	
	OBB->vertex( -0.5, -0.5 );		
	OBB->uv( _getXTextureOffset(), 
			 _getYTextureOffset() + _getYTextureUVRatio() );
	
	OBB->vertex( 0.5, -0.5 );		
	OBB->uv( _getXTextureOffset() + _getXTextureUVRatio(), 
			 _getYTextureOffset() + _getYTextureUVRatio() );
	
	OBB->vertex( -0.5, 0.5 );		
	OBB->uv( _getXTextureOffset(), 
			 _getYTextureOffset() );
	
	OBB->vertex( 0.5, 0.5 );
	OBB->uv( _getXTextureOffset() + _getXTextureUVRatio(), 
			 _getYTextureOffset() );
	
	OBB->end();			
}

struct PVRTexHeader
{
	uint32_t headerLength;
	uint32_t height;
	uint32_t width;
	uint32_t numMipmaps;
	uint32_t flags;
	uint32_t dataLength;
	uint32_t bpp;
	uint32_t bitmaskRed;
	uint32_t bitmaskGreen;
	uint32_t bitmaskBlue;
	uint32_t bitmaskAlpha;
	uint32_t pvrTag;
	uint32_t numSurfs;
};

bool Texture::_loadPVRTCToBoundTexture( NSString* path )
{
	static uint PVR_TEXTURE_FLAG_TYPE_MASK = 0xff;
	
	enum
	{
		kPVRTextureFlagTypePVRTC_2 = 24,
		kPVRTextureFlagTypePVRTC_4
	};
	
	NSData *data = [NSData dataWithContentsOfFile:path];
	
	if( data == nil )
		return false;
			
	PVRTexHeader *header = NULL;
	uint32_t flags, pvrTag;
	uint32_t dataLength = 0, dataSize = 0;
	uint32_t blockSize = 0, widthBlocks = 0, heightBlocks = 0;
	uint32_t bpp = 4;
	char *bytes = NULL;
	uint32_t formatFlags;
	GLenum _internalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
	
	header = (PVRTexHeader *)[data bytes];
	
	pvrTag = CFSwapInt32LittleToHost(header->pvrTag);
	
	if ('P' != ((pvrTag >>  0) & 0xff) ||
		'V' != ((pvrTag >>  8) & 0xff) ||
		'R' != ((pvrTag >> 16) & 0xff) ||
		'!' != ((pvrTag >> 24) & 0xff))
	{
		return FALSE;
	}
	
	flags = CFSwapInt32LittleToHost(header->flags);
	formatFlags = flags & PVR_TEXTURE_FLAG_TYPE_MASK;
	
	if (formatFlags == kPVRTextureFlagTypePVRTC_4 || formatFlags == kPVRTextureFlagTypePVRTC_2)
	{		
		if (formatFlags == kPVRTextureFlagTypePVRTC_4)
			_internalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
		else if (formatFlags == kPVRTextureFlagTypePVRTC_2)
			_internalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
		
		width = CFSwapInt32LittleToHost(header->width);
		height = CFSwapInt32LittleToHost(header->height);
		
		requiresAlpha = CFSwapInt32LittleToHost(header->bitmaskAlpha);
		
		dataLength = CFSwapInt32LittleToHost(header->dataLength);
		
		bytes = (char *)[data bytes] + sizeof(PVRTexHeader);
		
		//load only the first level
		if (formatFlags == kPVRTextureFlagTypePVRTC_4)
		{
			blockSize = 4 * 4; // Pixel by pixel block size for 4bpp
			widthBlocks = width / 4;
			heightBlocks = height / 4;
			bpp = 4;
		}
		else
		{
			blockSize = 8 * 4; // Pixel by pixel block size for 2bpp
			widthBlocks = width / 8;
			heightBlocks = height / 4;
			bpp = 2;
		}
		
		// Clamp to minimum number of blocks
		if (widthBlocks < 2)			widthBlocks = 2;
		if (heightBlocks < 2)			heightBlocks = 2;
		
		dataSize = widthBlocks * heightBlocks * ((blockSize  * bpp) / 8);
		
		//load in GL		
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, _internalFormat, width, height, 0, dataSize, bytes );
						
		return glGetError() == GL_NO_ERROR;
	}
	
	return false;	
}



