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

		///returns the usage type of this shader, ie. fragment or vertex shader
		Type getType()
		{
			return mType;
		}

		GLuint getGLShader()
		{
			return mGLShader;
		}

		///creates a new ShaderProgram using the source of this one, concatenated with the given preprocessor header
		ShaderProgram* cloneWithHeader( const std::string& preprocessorHeader )
		{
			DEBUG_ASSERT( preprocessorHeader.size(), "The preprocessor header can't be empty" );
			
			return new ShaderProgram( mType, preprocessorHeader + mContentString );
		}

		virtual bool onLoad();
		virtual void onUnload( bool soft = false );

	protected:

		std::string mContentString;

		Type mType;
		GLuint mGLShader;

		bool _load();
	};
}

#endif // ShaderProgram_h__
