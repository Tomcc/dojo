#pragma once

#include "dojo_common_header.h"

#include "Utils.h"
#include "Platform.h"

namespace Dojo
{
	class FontSystem
	{
	public:

		typedef std::unordered_map< String, FT_Face > FaceMap;
		typedef std::unordered_map< String, void* > FaceMemoryMap;

		FontSystem()
		{
			//launch FreeType
			int err = FT_Init_FreeType( &freeType );

			DEBUG_ASSERT( err == 0, "FreeType failed to initialize" );
		}

		virtual ~FontSystem()
		{
			for( auto e : memoryMap )
				free( e.second );

			FT_Done_FreeType( freeType );
		}

		FT_Face getFace( const String& fileName )
		{
			FaceMap::iterator where = faceMap.find( fileName );
			return where != faceMap.end() ? where->second : _createFaceForFile( fileName );
		}

		FT_Stroker getStroker( float width )
		{
			FT_Stroker s;
			FT_Stroker_New( freeType, &s );

			FT_Stroker_Set( s, 
				(FT_Fixed)(width * 64.f), 
				FT_STROKER_LINECAP_ROUND, 
				FT_STROKER_LINEJOIN_ROUND, 
				0 ); 

			return s;
		}

	protected:

		FaceMap faceMap;
		FaceMemoryMap memoryMap;

		FT_Library freeType;

		FT_Face _createFaceForFile( const String& fileName )
		{
			char* buf;
			FT_Long size = Platform::singleton().loadFileContent( buf, fileName );
			
			//create new face from memory - loading from memory is needed for zip loading
			FT_Face face;
			int err = FT_New_Memory_Face( freeType, (FT_Byte*)buf, size, 0, &face );
			faceMap[ fileName ] = face;
			memoryMap[ fileName ] = buf; //keep the memory

			DEBUG_ASSERT_INFO( err == 0, "FreeType could not load a Font file", "path = " + fileName );

			return face;
		}

	private:
	};
}
