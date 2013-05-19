#include "stdafx.h"

#include "Shader.h"

#include "ResourceGroup.h"
#include "RenderState.h"

using namespace Dojo;

Shader::NameBuiltInUniformMap Shader::sBuiltiInUniformsNameMap; //TODO implement this with an initializer list when VS decides to work with it

void Shader::_populateUniformNameMap()
{
	DEBUG_ASSERT( sBuiltiInUniformsNameMap.empty(), "The name-> builtinuniform map should be empty when populating" );
	for( int i = 0; i < DOJO_MAX_TEXTURE_UNITS; ++i )
	{
		std::string base = "TEXTURE_";
		base += '0' + (char)i;

		sBuiltiInUniformsNameMap[ base ] = BuiltInUniform( BU_TEXTURE_0 + i );
		sBuiltiInUniformsNameMap[ base + "_DIMENSION" ] = BuiltInUniform( BU_TEXTURE_0_DIMENSION + i );
		sBuiltiInUniformsNameMap[ base + "_TRANSFORM" ] = BuiltInUniform( BU_TEXTURE_0_TRANSFORM + i );
	}

	sBuiltiInUniformsNameMap[ "WORLD" ] = BU_WORLD;
	sBuiltiInUniformsNameMap[ "VIEW" ] = BU_VIEW;
	sBuiltiInUniformsNameMap[ "PROJECTION" ] = BU_PROJECTION;
	sBuiltiInUniformsNameMap[ "WORLDVIEWPROJ" ] = BU_WORLDVIEWPROJ;
	sBuiltiInUniformsNameMap[ "OBJECT_COLOR" ] = BU_OBJECT_COLOR;
	sBuiltiInUniformsNameMap[ "TIME" ] = BU_TIME;
}

Shader::BuiltInUniform Shader::_getUniformForName( const std::string& name )
{
	if( sBuiltiInUniformsNameMap.empty() ) //populate it the first time only //HACK because VS12 doesn't support initializing the map with an initializer list
		_populateUniformNameMap();

	auto elem = sBuiltiInUniformsNameMap.find( name );
	return ( elem != sBuiltiInUniformsNameMap.end() ) ? elem->second : BU_NONE;
}

void Shader::_assignProgram( const Table& desc, ShaderProgram::Type type )
{
	static const String typeKeyMap[] =	{ "vertexShader", "fragmentShader" };

	//check if this program is immediate or not
	//it is file-based if the resource can be found in the current RG
	auto& keyValue = desc.getString( typeKeyMap[ type ] );
	ShaderProgram* program = getCreator()->getProgram( keyValue );

	mOwnsProgram[ type ] = (program == nullptr);

	if( !program )//load the immediate shader
		program = new ShaderProgram( type, keyValue.ASCII() );
	else
		DEBUG_ASSERT_INFO( program->getType() == type, "The linked shader is of the wrong type", "expected type = " + typeKeyMap[ type ] );

	pProgram[ type ] = program;
}

void Shader::setUniformCallback( const String& nameUTF, const UniformCallback& dataBinder )
{
	std::string name = nameUTF.ASCII();

	auto elem = mUniformMap.find( name );
	
	if( elem != mUniformMap.end() )
		elem->second.userUniformBinder = dataBinder; //assign the data source to the right uniform

	else
		DEBUG_MESSAGE( "WARNING: can't find a Shader uniform named \"" << name << "\". Was it optimized away by the compiler?" );
}

#ifdef DOJO_SHADERS_AVAILABLE

void Shader::use( RenderState* user )
{
	DEBUG_ASSERT( isLoaded(), "tried to use a Shader that wasn't loaded" );

	glUseProgram( mGLProgram );

	GLint tempInt;

	//bind the uniforms and the attributes
	for( auto& uniform : mUniformMap )
	{
		void* ptr = nullptr;

		BuiltInUniform biu = uniform.second.builtInUniform;
		switch ( biu )
		{
		case BU_NONE:
			ptr = uniform.second.userUniformBinder(); //call the user callback and be happy
			break;
		case BU_WORLD:
			DEBUG_TODO;
			break;
		case BU_VIEW:
			DEBUG_TODO;
			break;
		case BU_PROJECTION:
			DEBUG_TODO;
			break;
		case BU_WORLDVIEWPROJ:
			DEBUG_TODO;
			break;
		case BU_OBJECT_COLOR:
			ptr = &user->color;
			break;
		case BU_TIME:
			DEBUG_TODO;
			break;
		default: //texture stuff

			if( biu >= BU_TEXTURE_0 && biu <= BU_TEXTURE_0 + DOJO_MAX_TEXTURE_UNITS )
			{
				tempInt = biu - BU_TEXTURE_0;
				ptr = &tempInt;
			}
			else if( biu >= BU_TEXTURE_0_DIMENSION && biu <= BU_TEXTURE_0_DIMENSION + DOJO_MAX_TEXTURE_UNITS )
			{
				DEBUG_TODO;
			}
			else if( biu >= BU_TEXTURE_0_TRANSFORM && biu <= BU_TEXTURE_0_TRANSFORM + DOJO_MAX_TEXTURE_UNITS )
			{
				DEBUG_TODO;
			}

			break;
		}

		if( ptr == nullptr ) //no data provided, skip
			break;

		//assign the data to the uniform
		//yes, this code is ugly...but don't be scared, it's as fast as a single glUniform in release :)
		switch ( uniform.second.type )
		{
		case GL_FLOAT:   glUniform1fv( uniform.second.location, uniform.second.count, (GLfloat*)ptr ); break;
		case GL_FLOAT_VEC2:   glUniform2fv( uniform.second.location, uniform.second.count, (GLfloat*)ptr ); break;
		case GL_FLOAT_VEC3:   glUniform3fv( uniform.second.location, uniform.second.count, (GLfloat*)ptr ); break;
		case GL_FLOAT_VEC4:   glUniform4fv( uniform.second.location, uniform.second.count, (GLfloat*)ptr ); break;
		case GL_INT: case GL_SAMPLER_2D:	case GL_SAMPLER_1D:		case GL_SAMPLER_3D:	case GL_SAMPLER_CUBE:
			glUniform1iv( uniform.second.location, uniform.second.count, (GLint*)ptr ); break; //this call also sets the samplers
		case GL_INT_VEC2:   glUniform2iv( uniform.second.location, uniform.second.count, (GLint*)ptr ); break;
		case GL_INT_VEC3:   glUniform3iv( uniform.second.location, uniform.second.count, (GLint*)ptr ); break;
		case GL_INT_VEC4:   glUniform4iv( uniform.second.location, uniform.second.count, (GLint*)ptr ); break;
		case GL_UNSIGNED_INT:	case GL_BOOL:   glUniform1uiv( uniform.second.location, uniform.second.count, (GLuint*)ptr ); break;
		case GL_UNSIGNED_INT_VEC2:	case GL_BOOL_VEC2:   glUniform2uiv( uniform.second.location, uniform.second.count, (GLuint*)ptr ); break;
		case GL_UNSIGNED_INT_VEC3:  case GL_BOOL_VEC3: glUniform3uiv( uniform.second.location, uniform.second.count, (GLuint*)ptr ); break;
		case GL_UNSIGNED_INT_VEC4:  case GL_BOOL_VEC4: glUniform4uiv( uniform.second.location, uniform.second.count, (GLuint*)ptr ); break;

		case GL_FLOAT_MAT2:   glUniformMatrix2fv( uniform.second.location, uniform.second.count, false, (GLfloat*)ptr ); break;
		case GL_FLOAT_MAT3:   glUniformMatrix3fv( uniform.second.location, uniform.second.count, false, (GLfloat*)ptr ); break;
		case GL_FLOAT_MAT4:   glUniformMatrix4fv( uniform.second.location, uniform.second.count, false, (GLfloat*)ptr ); break;
		case GL_FLOAT_MAT2x3:   glUniformMatrix2x3fv( uniform.second.location, uniform.second.count, false, (GLfloat*)ptr ); break;
		case GL_FLOAT_MAT2x4:   glUniformMatrix2x4fv( uniform.second.location, uniform.second.count, false, (GLfloat*)ptr ); break;
		case GL_FLOAT_MAT3x2:   glUniformMatrix3x2fv( uniform.second.location, uniform.second.count, false, (GLfloat*)ptr ); break;
		case GL_FLOAT_MAT3x4:   glUniformMatrix3x4fv( uniform.second.location, uniform.second.count, false, (GLfloat*)ptr ); break;
		case GL_FLOAT_MAT4x2:   glUniformMatrix4x2fv( uniform.second.location, uniform.second.count, false, (GLfloat*)ptr ); break;
		case GL_FLOAT_MAT4x3:   glUniformMatrix4x3fv( uniform.second.location, uniform.second.count, false, (GLfloat*)ptr ); break;

		default:
			//not found... but it's not possible to warn each frame. Do place a brk here if unsure
			break;
		}

		CHECK_GL_ERROR;
	}
}

bool Shader::onLoad()
{
	DEBUG_ASSERT( !isLoaded(), "cannot reload an already loaded Shader" );

	loaded = false;

	//load the descriptor table
	Table desc;
	Table::loadFromFile( &desc, filePath );

	//grab all types
	for( int i = 0; i < ShaderProgram::_SPT_COUNT; ++i )
		_assignProgram( desc, (ShaderProgram::Type)i );

	//ensure they're loaded
	for( auto program : pProgram )
	{
		if( !program->isLoaded() )
		{
			if( !program->onLoad() ) //one program was not loaded, the shader can't work
				return loaded;
		}
	}

	//link the shaders together in this high level shader
	mGLProgram = glCreateProgram();

	for( auto program : pProgram )
		glAttachShader( mGLProgram, program->getGLShader() );

	glLinkProgram( mGLProgram );

	CHECK_GL_ERROR;

	//check if the linking went ok
	GLint linked;
	glGetProgramiv( mGLProgram, GL_LINK_STATUS, &linked);
	loaded = linked != 0;
	if (!linked)
	{
		DEBUG_FAIL( "Could not link a shader program" );
	}
	else
	{
		GLchar namebuf[1024];
		GLint nameLength, size;
		GLenum type;

		//get uniforms and their locations
		for( int i = 0; ; ++i)
		{
			glGetActiveUniform( mGLProgram, i, sizeof( namebuf ), &nameLength, &size, &type, namebuf );

			if( glGetError() != GL_NO_ERROR ) //check if this value existed
				break;
			else  //store the Uniform data
			{
				GLint loc = glGetUniformLocation( mGLProgram, namebuf );

				if( loc >= 0 )  //loc < 0 means that this is a OpenGL-builtin such as gl_WorldViewProjectionMatrix
				{
					mUniformMap[ namebuf ] = Uniform( 
						loc,
						size, 
						type, 
						_getUniformForName( namebuf ) );
				}
			}
		}
	}

	return loaded;
}

#else

void Shader::use()
{
	DEBUG_FAIL( "Shaders not supported" );
}

bool Shader::onLoad()
{
	DEBUG_FAIL( "Shaders not supported" );
	return false;
}

#endif


void Shader::onUnload( bool soft /* = false */ )
{
	DEBUG_ASSERT( isLoaded(), "This shader was already unloaded" );

	//only manage the programs that aren't shared
	for( int i = 0; i < ShaderProgram::_SPT_COUNT; ++i )
	{
		if( mOwnsProgram[i] )
		{
			pProgram[i]->onUnload( false );
			SAFE_DELETE( pProgram[i] );
		}
	}

	loaded = false;
}