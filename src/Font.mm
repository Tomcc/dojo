#include "stdafx.h"

#include "Font.h"

using namespace Dojo;

bool Font::load()
{
	char* file;

	uint size = Platform::loadFileContent( file, filePath );
	
	if( file == NULL )
		return false;
	 
	char* buf = file;
	char* end = buf + size;
	
	//the first two values are pixel width and height of the font
	fontWidth = Utils::toInt( buf, end );
	fontHeight = Utils::toInt( buf, end );
	
	offset = Utils::toInt( buf, end );
	
	//get UV dimensions
	Texture* tex = fontSet->getFrame(0);
	
	cols = (uint)tex->getWidth() / fontWidth;
	rows = (uint)tex->getHeight() / fontHeight;
	
	totalCharNumber = rows*cols;
				
	//find height of a char in UV space
	fontUVSize.x = (float)fontWidth/(float)( tex->getWidth() );
	fontUVSize.y = (float)fontHeight/(float)( tex->getHeight() );			
	
	//get ints representing char width in pixels
	uint i = 0;
	for (; i < 256 && buf < end; ++i)
	{
		charWidth[i] = Utils::toInt( buf, end );
		
		DEBUG_OUT( charWidth[i] );
	}
		
	DEBUG_ASSERT( i == 256 );
	
	_createPage( 0 );			
				
	return true;
}