#ifndef Shader_h__
#define Shader_h__

#include "dojo_common_header.h"

#include "Resource.h"
#include "Table.h"
#include "ShaderProgram.h"

namespace Dojo
{
	class RenderState;

	///A Shader is an object representing a VSH+PSH couple and its attributes.
	/**
	Each Renderable, at any moment, uses exactly one Shader, whether loaded from file (.dsh) or procedurally generated to fake the FF
	*/
	class Shader : public Resource
	{
	public:

		///A Binder is a type of ()-> void* function which returns a pointer to data to be bound to an uniform
		/**
		\remark the type of the data is deduced from the types in .ps and the .vs, so it is important to return the right kind of data
		*/
		typedef std::function< void*() > UniformCallback;

		///A built-in uniform is a uniform shader parameter which Dojo recognizes and provides to the shader being run
		enum BuiltInUniform
		{
			BU_NONE,

			BU_TEXTURE_0,	///The texture currently bound to unit 0

			BU_TEXTURE_0_DIMENSION = BU_TEXTURE_0 + DOJO_MAX_TEXTURE_UNITS, ///The dimensions in pixels of the texture currently bound to unit 0

			BU_TEXTURE_0_TRANSFORM = BU_TEXTURE_0_DIMENSION + DOJO_MAX_TEXTURE_UNITS,

			BU_WORLD = BU_TEXTURE_0_DIMENSION + DOJO_MAX_TEXTURE_UNITS,			///The world matrix
			BU_VIEW,			///The view matrix
			BU_PROJECTION,		///The projection matrix
			BU_WORLDVIEWPROJ,	///The complete transformation matrix
			BU_OBJECT_COLOR,	///The object's color
			
			BU_TIME,			///Time in seconds since the start of the program
		};

		///Creates a new Shader from a file path
		Shader( Dojo::ResourceGroup* creator, const String& filePath ) :
			Resource( creator, filePath )
		{
			memset( pProgram, 0, sizeof( pProgram ) ); //init to null
		}

		///Assigns this data source (Binder) to the Uniform with the given name
		/**
		the Binder will be executed each time something is rendered with this Shader
		*/
		void setUniformCallback( const String& name, const UniformCallback& dataBinder );

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
		virtual void use( RenderState* user );

		virtual bool onLoad();

		virtual void onUnload( bool soft = false );

	protected:

		struct Uniform 
		{
			GLint location;

			GLint count;
			GLenum type;

			BuiltInUniform builtInUniform;
			UniformCallback userUniformBinder;

			Uniform()
			{

			}

			Uniform( GLint loc, GLint elementCount, GLenum ty, BuiltInUniform biu ) :
				location( loc ),
				count( elementCount ),
				type( ty ),
				builtInUniform( biu )
			{
				DEBUG_ASSERT( location >= 0, "Invalid Uniform location" );
				DEBUG_ASSERT( count > 0, "Invalid element count" );

				//TODO: find the element count using type+size
			}
		};

		typedef std::unordered_map< std::string, Uniform > NameUniformMap;

		typedef std::unordered_map< std::string, BuiltInUniform > NameBuiltInUniformMap;

		static NameBuiltInUniformMap sBuiltiInUniformsNameMap;

		static void _populateUniformNameMap();

		static BuiltInUniform _getUniformForName( const std::string& name );

		NameUniformMap mUniformMap;

		GLuint mGLProgram;

		ShaderProgram* pProgram[ ShaderProgram::_SPT_COUNT ];
		bool mOwnsProgram[ ShaderProgram::_SPT_COUNT ];

		void _assignProgram( const Table& desc, ShaderProgram::Type type );

	private:
	};
}

#endif // Shader_h__
