/*
 *  Resource.h
 *  Ninja Training
 *
 *  Created by Tommaso Checchi on 8/10/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */

#ifndef Resource_h__
#define Resource_h__

#include "dojo_common_header.h"

namespace Dojo 
{
	class ResourceGroup;
	
	class Resource 
	{
	public:
		
		Resource( ResourceGroup* creatorGroup, const String& path ) :
		creator( creatorGroup ),
		loaded( false ),
		size( 0 ),
		filePath( path )
		{
			
		}
		
		virtual ~Resource()
		{
			//must be unloaded at this point
			DEBUG_ASSERT( loaded == false );
		}
		
		virtual bool onLoad()=0;
		virtual void onUnload( bool soft = false )=0;
		
		inline bool isLoaded()
		{
			return loaded;
		}

		inline int getByteSize()
		{
			return size;
		}
		
		inline ResourceGroup* getCreator()
		{			
			return creator;
		}
		
		inline const String& getFilePath()
		{
			return filePath;
		}

		inline bool isFileBased()
		{
			return filePath.size() > 0;
		}
		
	protected:
		
		ResourceGroup* creator;
		
		bool loaded;
		uint size;
		
		String filePath;
	};
}

#endif