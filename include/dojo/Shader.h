#ifndef Shader_h__
#define Shader_h__

#include "dojo_common_header.h"

#include "Resource.h"
#include "Table.h"
#include "ShaderProgram.h"

namespace Dojo
{
	///A Shader is an object representing a VSH+PSH couple and its attributes.
	/**
	Each Renderable, at any moment, uses exactly one Shader, whether loaded from file (.dsh) or procedurally generated to fake the FF
	*/
	class Shader : public Resource
	{
	public:

		///Creates a new Shader from a file path
		Shader( Dojo::ResourceGroup* creator, const String& filePath ) :
			Resource( creator, filePath )
		{
			memset( pProgram, 0, sizeof( pProgram ) ); //init to null
		}

		///returns the program currently bound for "type" pipeline pass
		ShaderProgram* getProgramFor( ShaderProgram::Type type )
		{
			return pProgram[ type ];
		}

		///returns the GL program handle
		GLuint getGLProgram()
		{
			return mGLProgram;
		}

		///binds the shader to the OpenGL state
		void bind();

		virtual bool onLoad();

		virtual void onUnload( bool soft = false );

	protected:

		GLuint mGLProgram;

		ShaderProgram* pProgram[ ShaderProgram::_SPT_COUNT ];
		bool mOwnsProgram[ ShaderProgram::_SPT_COUNT ];

		void _assignProgram( const Table& desc, ShaderProgram::Type type );

	private:
	};
}

#endif // Shader_h__
