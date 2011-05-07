//
//  OSXPlatform.mm
//  dojo
//
//  Created by Tommaso Checchi on 5/7/11.
//  Copyright 2011 none. All rights reserved.
//

#include "OSXPlatform.h"

using namespace Dojo;


void OSXPlatform::initialise()
{
    DEBUG_TODO;
}

void OSXPlatform::shutdown()
{
    DEBUG_TODO;    
}

void OSXPlatform::acquireContext()
{    
    DEBUG_TODO;
}
void OSXPlatform::present()
{    
    DEBUG_TODO;
}

void OSXPlatform::step( float dt )
{
    
    DEBUG_TODO;
}


void OSXPlatform::loop( float frameTime )
{
    DEBUG_TODO;
}



std::string OSXPlatform::getCompleteFilePath( const std::string& name, const std::string& type, const std::string& path )
{
    DEBUG_TODO;
    
    return "";
}


void OSXPlatform::getFilePathsForType( const std::string& type, const std::string& path, std::vector<std::string>& out )
{
    DEBUG_TODO;
}


uint OSXPlatform::OSXPlatform::loadFileContent( char*& bufptr, const std::string& path )
{
    return DEBUG_TODO;
}


void OSXPlatform::loadPNGContent( void*& bufptr, const std::string& path, uint& width, uint& height )
{
    DEBUG_TODO;
}



///loads the given file in a buffer - WARNING not every format is supported on every platform
uint OSXPlatform::loadAudioFileContent( ALuint& buffer, const std::string& path )
{
    return DEBUG_TODO;
}



void OSXPlatform::load( Table* dest, const std::string& relPath )
{
    DEBUG_TODO;
}

void OSXPlatform::save( Table* table, const std::string& relPath )
{
    DEBUG_TODO;
}

void OSXPlatform::openWebPage( const std::string& site )
{
    DEBUG_TODO;
}