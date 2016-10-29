//
//  ApplePlatform.h
//  dojo
//
//  Created by Tommaso Checchi on 5/16/11.
//  Copyright 2011 none. All rights reserved.
//


#pragma once

#include "../Platform.h"

#undef self 

#ifdef __OBJC__
	#import <Foundation/NSAutoreleasePool.h>
#endif

#include "Timer.h"

#include "../dojo_common_header.h"

namespace Dojo {

#ifndef __OBJC__
	class NSAutoreleasePool;
	class NSString;
#endif

	class ApplePlatform : public Platform {
	public:

		ApplePlatform( const Table& config );

		virtual ~ApplePlatform();

		virtual void step( float dt ) override;

		virtual PixelFormat loadImageFile(std::vector<uint8_t>& imageData, utf::string_view path, uint32_t& width, uint32_t& height, int& pixelSize) override;

	protected:

		Timer frameTimer;

		enum CGImageType {
			CGIT_PNG,
			CGIT_JPG
		};

		void _createApplicationDirectory();

		//these always exists because .cpp and .mm compiling this header could get different sizes for the class!!!
		NSAutoreleasePool* pool;


		PixelFormat loadImageContent( void*& bufptr, const String& path, int& width, int& height, CGImageType type, bool correctPremult );
	};
}

