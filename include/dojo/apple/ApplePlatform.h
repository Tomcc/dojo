//
//  ApplePlatform.h
//  dojo
//
//  Created by Tommaso Checchi on 5/16/11.
//  Copyright 2011 none. All rights reserved.
//


#pragma once

#include "../dojo_common_header.h"

#include "../Platform.h"

#ifdef __OBJC__
	#import <Foundation/NSAutoreleasePool.h>
#endif

#include "Timer.h"

namespace Dojo {

#ifndef __OBJC__
	class NSAutoreleasePool;
	class NSString;
#endif

	class ApplePlatform : public Platform {
	public:

		ApplePlatform( const Table& config );

		virtual ~ApplePlatform();

		virtual void step( float dt );

		virtual GLenum loadImageFile( void*& bufptr, const String& path, int& width, int& height, int& pixelSize );

	protected:

		Timer frameTimer;

		enum CGImageType {
			CGIT_PNG,
			CGIT_JPG
		};

		void _createApplicationDirectory();

		//these always exists because .cpp and .mm compiling this header could get different sizes for the class!!!
		NSAutoreleasePool* pool;


		GLenum loadImageContent( void*& bufptr, const String& path, int& width, int& height, CGImageType type, bool correctPremult );
	};
}

