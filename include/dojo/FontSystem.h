#ifndef FontSystem_h__
#define FontSystem_h__

#include "dojo_common_header.h"

#include "Utils.h"

namespace Dojo
{
	class FontSystem
	{
	public:

		typedef std::unordered_map< String, FT_Face > FaceMap;

		FontSystem()
		{
			//launch FreeType
			int err = FT_Init_FreeType( &freeType );

			DEBUG_ASSERT( err == 0 );
		}

		virtual ~FontSystem()
		{
			FT_Done_FreeType( freeType );
		}

		FT_Face getFace( const String& fileName )
		{
			FaceMap::iterator where = faceMap.find( fileName );

			if( where == faceMap.end() )
				return _createFaceForFile( fileName );
			else
				return where->second;
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

		FT_Library freeType;

		FT_Face _createFaceForFile( const String& fileName )
		{
			//create new face
			FT_Face face;
			int err = FT_New_Face( freeType, fileName.ASCII().c_str(), 0, &face );
			faceMap[ fileName ] = face;

			DEBUG_ASSERT( err == 0 );

			return face;
		}

	private:
	};
}
#endif // FontSystem_h__