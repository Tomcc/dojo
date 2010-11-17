/*
 *  FrameSet.cpp
 *  Ninja Training
 *
 *  Created by Tommaso Checchi on 8/14/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */

#include "FrameSet.h"

#include "ResourceGroup.h"

using namespace Dojo;

bool FrameSet::load()
{			
	if( isLoaded() )
		return true;
	
	for( uint i = 0; i < frames.size(); ++i )
	{
		if( frames.at(i)->load() )
	
			// count bytesize
			size += frames.at(i)->getByteSize();
	}		
		
	loaded = true;		
		
	return true;	
}


bool FrameSet::loadAtlas( const std::string& infoFile, Texture* atlas )
{
	if( isLoaded() )
		return true;
	
	//can obtain the atlas?
	if( !creator && !atlas )
		return NULL;
	
	NSString* NSPath = Utils::toNSString( infoFile );
	NSData* data = [[NSData alloc] initWithContentsOfFile:NSPath ];
	
	if( !data )
		return NULL;
	
	char* buf = (char*)[data bytes];
	char* end = buf + [data length];
		
	//retrieve the correctly named texture
	if( !atlas )
	{		
		FrameSet* atlasSet = creator->getFrameSet( Utils::toString( buf , end ) );
		//check atlas existence
		if( !atlasSet || atlasSet->getFrameNumber() == 0 )
			return false;
		
		atlas = atlasSet->getFrame(0);
	}	
	uint x, y, sx, sy;
	Texture* tex;
	while( buf < end )
	{
		//for each 4 ints extracted, create a new tile texture
		x = Utils::toInt( buf, end );
		y = Utils::toInt( buf, end );
		
		sx = Utils::toInt( buf, end );
		sy = Utils::toInt( buf, end );
		
		tex = new Texture( creator, "none" );
		
		if( tex->loadFromAtlas( atlas, x,y, sx,sy ) )			
			addTexture( tex, true );
		
	}
	
	//loaded at least one?
	loaded = frames.size() > 0;
	
	//HACK
	//[data release];
	
	return true;
}

