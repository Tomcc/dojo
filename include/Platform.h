#ifndef Platform_h__
#define Platform_h__

#include "dojo_common_header.h"

#include "BaseObject.h"

namespace Dojo 
{
	class Platform : public BaseObject
	{
	public:
		
		Platform()
		{
			
		}
		
		static void getFilePathsForType( const std::string& type, const std::string& path, std::vector<std::string>& out );
		
		static uint loadFileContent( char*& bufptr, const std::string& path );
		
	protected:
	};
}

#endif/*
 *  Platform.h
 *  Drafted
 *
 *  Created by Tommaso Checchi on 1/24/11.
 *  Copyright 2011 none. All rights reserved.
 *
 */

