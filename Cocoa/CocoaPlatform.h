//
//  CocoaPlatform.h
//  dojo
//
//  Created by Tommaso Checchi on 5/16/11.
//  Copyright 2011 none. All rights reserved.
//


#ifndef __CocoaPlatform_h
#define __CocoaPlatform_h

#include "dojo_common_header.h"

#include "Platform.h"

#ifdef __OBJC__
#import <Foundation/NSAutoreleasePool.h>
#import "GenericListener.h"
#endif

namespace Dojo
{
	
#ifndef __OBJC__
	class NSAutoreleasePool;
#endif	
	
    class CocoaPlatform : public Platform
    {
    public:
		
        CocoaPlatform();
		
		virtual ~CocoaPlatform();
        
		virtual void step( float dt );
		
		virtual std::string getCompleteFilePath( const std::string& name, const std::string& type, const std::string& path );
		virtual void getFilePathsForType( const std::string& type, const std::string& path, std::vector<std::string>& out );
		virtual uint loadFileContent( char*& bufptr, const std::string& path );
		virtual void loadPNGContent( void*& bufptr, const std::string& path, uint& width, uint& height );
		
		virtual void load( Table* dest, const std::string& relPath = "" );
		virtual void save( Table* table, const std::string& relPath = "" );
		
		virtual void openWebPage( const std::string& site );
        
    protected:
		
		//these always exists because .cpp and .mm compiling this header could get different sizes for the class!!!
       	NSAutoreleasePool* pool;
    };
}

#endif