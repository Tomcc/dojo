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

		///Resource::DataProvider is a virtual interface for resource loading
		/**
		It allows non-file-baseed resources to be unloaded and reloaded on the fly
		*/
		class DataProvider
		{
		public:

			///onLoad is called when a registered resource needs to be loaded
			virtual void onLoad( Resource* )=0;
		};
		
		Resource(  ResourceGroup* group = NULL ) :
		creator( group ),
		loaded( false ),
		size( 0 ),
		pDataProvider( NULL )
		{

		}

		Resource( ResourceGroup* creatorGroup, const String& path ) :
		creator( creatorGroup ),
		loaded( false ),
		size( 0 ),
		filePath( path ),
		pDataProvider( NULL )
		{
			DEBUG_ASSERT( path.size() );
		}

		Resource( ResourceGroup* group, DataProvider* source ) :
		creator( group ),
		loaded( false ),
		size( 0 ),
		pDataProvider( source )
		{
			DEBUG_ASSERT( source );
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

		inline DataProvider* getDataProvider()
		{
			return pDataProvider;
		}

		inline bool isFiledBased()
		{
			return filePath.size() > 0;
		}

		inline bool isReloadable()
		{
			return isFiledBased() || getDataProvider();
		}
		
	protected:
		
		ResourceGroup* creator;
		
		bool loaded;
		uint size;
		
		String filePath;
		DataProvider* pDataProvider;
	};
}

#endif