/*
 *  Buffer.h
 *  Ninja Training
 *
 *  Created by Tommaso Checchi on 8/10/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */

#ifndef Buffer_h__
#define Buffer_h__

#include "dojo_config.h"

#include <string>

namespace Dojo 
{
	class ResourceGroup;
	
	class Buffer
	{
	public:
		
		Buffer( ResourceGroup* creatorGroup, const std::string& path ) :
		creator( creatorGroup ),
		filePath( path ),
		loaded( false ),
		size( 0 )
		{
			
		}
		
		virtual ~Buffer()
		{
			/*if( isLoaded() )
				unload();*/
		}
		
		virtual bool load()=0;
		virtual void unload()=0;
		
		virtual void bind()=0;
		
		inline bool isLoaded()
		{
			return loaded;
		}
		
		inline uint getByteSize()
		{
			DOJO_ASSERT( size );
			
			return size;
		}
		
		inline ResourceGroup* getCreator()
		{			
			return creator;
		}
		
		inline const std::string& getFilePath()
		{
			return filePath;
		}
		
	protected:
		
		ResourceGroup* creator;
		
		bool loaded;
		uint size;
		
		std::string filePath;
	};
}

#endif