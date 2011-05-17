//
//  ApplePlatform.h
//  dojo
//
//  Created by Tommaso Checchi on 5/16/11.
//  Copyright 2011 none. All rights reserved.
//


#ifndef __ApplePlatform_h
#define __ApplePlatform_h

#include "dojo_common_header.h"

#include "Platform.h"

#ifdef __OBJC__
#import <Foundation/NSAutoreleasePool.h>
#endif

namespace Dojo
{
	
#ifndef __OBJC__
	class NSAutoreleasePool;
	class NSString;
#endif
	
    class ApplePlatform : public Platform
    {
    public:
		
        ApplePlatform();
		
		virtual ~ApplePlatform();
        
		virtual void step( float dt );
		
		virtual std::string getCompleteFilePath( const std::string& name, const std::string& type, const std::string& path );
		virtual void getFilePathsForType( const std::string& type, const std::string& path, std::vector<std::string>& out );
		virtual uint loadFileContent( char*& bufptr, const std::string& path );
		virtual void loadPNGContent( void*& bufptr, const std::string& path, uint& width, uint& height );
		
		uint loadAudioFileContent( ALuint& buffer, const std::string& path );
		
		
		virtual void load( Table* dest, const std::string& absPath = "" );
		virtual void save( Table* table, const std::string& absPath = "" );
		
		virtual void openWebPage( const std::string& site );
        
    protected:
		
		NSString* _getDestinationFilePath( Table* t, const std::string& absPath = "" );
		
		//these always exists because .cpp and .mm compiling this header could get different sizes for the class!!!
       	NSAutoreleasePool* pool;
    };
}

#endif