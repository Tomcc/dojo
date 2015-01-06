#pragma once

#include "dojo_common_header.h"

#include "Resource.h"
#include "ShaderProgramType.h"
#include "VertexField.h"

namespace Dojo
{
	class Renderable;
	class ShaderProgram;
	class Table;

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
		typedef std::function< const void*( const Renderable& ) > UniformCallback;

		///A built-in uniform is a uniform shader parameter which Dojo recognizes and provides to the shader being run
		enum BuiltInUniform
		{
			BU_NONE,

			BU_TEXTURE_0,	///<The texture currently bound to unit 0
			BU_TEXTURE_N = BU_TEXTURE_0 + DOJO_MAX_TEXTURES-1,

			BU_TEXTURE_0_DIMENSION, ///<The dimensions in pixels of the texture currently bound to unit 0 (vec2)
			BU_TEXTURE_N_DIMENSION = BU_TEXTURE_0_DIMENSION + DOJO_MAX_TEXTURES-1,

			BU_TEXTURE_0_TRANSFORM,
			BU_TEXTURE_N_TRANSFORM = BU_TEXTURE_0_TRANSFORM + DOJO_MAX_TEXTURES-1,

			BU_WORLD = BU_TEXTURE_0_DIMENSION + DOJO_MAX_TEXTURES,			///<The world matrix
			BU_VIEW,		///<The view matrix
			BU_PROJECTION,		///<The projection matrix
        		BU_WORLDVIEW,           ///<The world view matrix
			BU_WORLDVIEWPROJ,	///<The complete transformation matrix
			BU_OBJECT_COLOR,	///<The object's color (vec4)

			BU_VIEW_DIRECTION,	///<The current world-space direction of the view (vec3)
			
			BU_TIME,		///<Time in seconds since the start of the program (float)
			BU_TARGET_DIMENSION,	///<The dimensions in pixels of the currently bound target (vec2)
			BU_TARGET_PIXEL   ///<The dimension in the UV space of one pixel
		};

		///A VertexAttribute represents a "attribute" binding in a vertex shader
		struct VertexAttribute
		{
			GLint location;
			GLint count; ///<The array size *for a single vertex*

			VertexField builtInAttribute;

			VertexAttribute()
			{

			}

			VertexAttribute( GLint loc, GLint size, VertexField bia ) :
				location( loc ),
				count( size ),
				builtInAttribute( bia )
			{
				DEBUG_ASSERT( location >= 0, "Invalid VertexAttribute location" );
				DEBUG_ASSERT( count > 0, "Invalid element count" );
			}
		};

		typedef std::unordered_map< std::string, VertexAttribute > NameAttributeMap;

		///Creates a new Shader from a file path
		Shader( ResourceGroup* creator, const String& filePath );

		///Assigns this data source (Binder) to the Uniform with the given name
		/**
		the Binder will be executed each time something is rendered with this Shader
		*/
		void setUniformCallback( const String& name, const UniformCallback& dataBinder );

		///returns the program currently bound for "type" pipeline pass
		ShaderProgram* getProgramFor(ShaderProgramType type)
		{
			return pProgram[ (unsigned char)type ];
		}

		///returns the GL program handle
		GLuint getGLProgram()
		{
			return mGLProgram;
		}

		const NameAttributeMap& getAttributes()
		{
			return mAttributeMap;
		}

		///binds the shader to the OpenGL state with the object that is using it
		virtual void use( const Renderable& user );

		virtual bool onLoad();

		virtual void onUnload( bool soft = false );

	protected:

		struct Uniform 
		{
			GLint location;

			GLint count;
			GLenum type;

			BuiltInUniform builtInUniform;
			UniformCallback userUniformCallback;

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
			}
		};

		typedef std::unordered_map< std::string, Uniform > NameUniformMap;

		typedef std::unordered_map< std::string, BuiltInUniform > NameBuiltInUniformMap;
		typedef std::unordered_map< std::string, VertexField > NameBuiltInAttributeMap;

		static NameBuiltInUniformMap sBuiltiInUniformsNameMap;
		static NameBuiltInAttributeMap sBuiltInAttributeNameMap;

		static void _populateUniformNameMap();
		static void _populateAttributeNameMap();

		static BuiltInUniform _getUniformForName( const std::string& name );
		static VertexField _getAttributeForName( const std::string& name );

		std::string mPreprocessorHeader;

		NameUniformMap mUniformMap;
		NameAttributeMap mAttributeMap;

		GLuint mGLProgram;

		ShaderProgram* pProgram[ (byte)ShaderProgramType::_Count ];
		bool mOwnsProgram[ (byte)ShaderProgramType::_Count ];

		void _assignProgram(const Table& desc, ShaderProgramType type);
        
        const void* _getUniformData( const Uniform& uniform, const Renderable& user );

	private:
	};
}


