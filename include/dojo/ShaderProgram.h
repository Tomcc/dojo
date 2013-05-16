#ifndef ShaderProgram_h__
#define ShaderProgram_h__

#include "dojo_common_header.h"

#include "Resource.h"

namespace Dojo
{
	///a ShaderProgram is a single programmable pipeline program step, loaded from a PSH, VSH or a string
	class ShaderProgram : public Resource
	{
	public:

		enum Type
		{
			SPT_VERTEX,
			SPT_FRAGMENT,

			_SPT_COUNT
		};

		///"real file" Resource constructor. When onLoad is called, it will use filePath to load its contents
		ShaderProgram( ResourceGroup* creator, const String& filePath );

		///"immediate" Resource constructor. When onLoad is called, it will use the String and type to load
		/**
		std::string is used because it has to be a pure ASCII string
		*/
		ShaderProgram( Type type, const std::string& contents ) :
			Resource( nullptr ),
			mContentString( contents ),
			mType( type )
		{
			DEBUG_ASSERT( mContentString.size(), "No shader code was defined (empty string)" );
		}

		GLuint getGLShader()
		{
			return mGLShader;
		}

		virtual bool onLoad();
		virtual void onUnload( bool soft = false );

	protected:

		std::string mContentString;

		Type mType;
		GLuint mGLShader;

		bool _load( const std::string& code );
	};
}

#endif // ShaderProgram_h__