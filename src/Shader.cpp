#include "stdafx.h"

#include "Shader.h"

#include "ResourceGroup.h"
#include "Renderable.h"
#include "GameState.h"
#include "Viewport.h"
#include "Renderer.h"
#include "Texture.h"

using namespace Dojo;

Shader::NameBuiltInUniformMap Shader::sBuiltiInUniformsNameMap; //TODO implement this with an initializer list when VS decides to work with it
Shader::NameBuiltInAttributeMap Shader::sBuiltInAttributeNameMap; //TODO ^

void Shader::_populateUniformNameMap()
{
	DEBUG_ASSERT( sBuiltiInUniformsNameMap.empty(), "The name-> builtinuniform map should be empty when populating" );
	for( int i = 0; i < DOJO_MAX_TEXTURES; ++i )
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
	sBuiltiInUniformsNameMap[ "WORLDVIEW" ] = BU_WORLDVIEW;
	sBuiltiInUniformsNameMap[ "WORLDVIEWPROJ" ] = BU_WORLDVIEWPROJ;
	sBuiltiInUniformsNameMap[ "VIEW_DIRECTION" ] = BU_VIEW_DIRECTION;
	sBuiltiInUniformsNameMap[ "OBJECT_COLOR" ] = BU_OBJECT_COLOR;
	sBuiltiInUniformsNameMap[ "TIME" ] = BU_TIME;
	sBuiltiInUniformsNameMap["TARGET_DIMENSION"] = BU_TARGET_DIMENSION;
	sBuiltiInUniformsNameMap["TARGET_PIXEL"] = BU_TARGET_PIXEL;
}

void Shader::_populateAttributeNameMap()
{
	DEBUG_ASSERT( sBuiltInAttributeNameMap.empty(), "The name-> builtinattribute map should be empty when populating" );
	for( int i = 0; i < DOJO_MAX_TEXTURES; ++i )
	{
		std::string base = "TEXCOORD_";
		base += '0' + (char)i;

		sBuiltInAttributeNameMap[base] = (VertexField)((byte)VertexField::UV0 + i);
	}

	sBuiltInAttributeNameMap["POSITION"] = VertexField::Position3D;
	sBuiltInAttributeNameMap["POSITION_2D"] = VertexField::Position2D;
	sBuiltInAttributeNameMap["NORMAL"] = VertexField::Normal;
	sBuiltInAttributeNameMap["COLOR"] = VertexField::Color;
}

Shader::BuiltInUniform Shader::_getUniformForName( const std::string& name )
{
	if( sBuiltiInUniformsNameMap.empty() ) //populate it the first time only //HACK because VS12 doesn't support initializing the map with an initializer list
		_populateUniformNameMap();

	auto elem = sBuiltiInUniformsNameMap.find( name );
	return ( elem != sBuiltiInUniformsNameMap.end() ) ? elem->second : BU_NONE;
}

VertexField Shader::_getAttributeForName( const std::string& name )
{
	if( sBuiltInAttributeNameMap.empty() )
		_populateAttributeNameMap();

	auto elem = sBuiltInAttributeNameMap.find( name );
	return (elem != sBuiltInAttributeNameMap.end()) ? elem->second : VertexField::None;
}

Shader::Shader( ResourceGroup* creator, const String& filePath ) :
	Resource( creator, filePath )
{
	memset( pProgram, 0, sizeof( pProgram ) ); //init to null
}

void Shader::_assignProgram( const Table& desc, ShaderProgramType type )
{
	static const String typeKeyMap[] =	{ "vertexShader", "fragmentShader" };
	auto typeID = (unsigned char)type;

	//check if this program is immediate or not
	//it is file-based if the resource can be found in the current RG
	auto& keyValue = desc.getString(typeKeyMap[typeID]);

	DEBUG_ASSERT_INFO(keyValue.size(), "No shader found in .shader file", "type = " + typeKeyMap[typeID]);
	
	ShaderProgram* program = getCreator()->getProgram( keyValue );

	mOwnsProgram[typeID] = (program == nullptr) && !mPreprocessorHeader.empty(); //if any preprocessor flag is defined, all programs are compiled as immediate

	if( !program ) //just load the immediate shader
		program = new ShaderProgram( type, mPreprocessorHeader + keyValue.ASCII() );

	else if( program && mPreprocessorHeader.size() ) //some preprocessor flags are set - copy the existing program and recompile it
		program = program->cloneWithHeader( mPreprocessorHeader );

	else
		DEBUG_ASSERT_INFO(program->getType() == type, "The linked shader is of the wrong type", "expected type = " + typeKeyMap[typeID]);

	pProgram[typeID] = program;
}

void Shader::setUniformCallback( const String& nameUTF, const UniformCallback& dataBinder )
{
	std::string name = nameUTF.ASCII();

	auto elem = mUniformMap.find( name );
	
	if( elem != mUniformMap.end() )
		elem->second.userUniformCallback = dataBinder; //assign the data source to the right uniform

	else
		DEBUG_MESSAGE( "WARNING: can't find a Shader uniform named \"" + name + "\". Was it optimized away by the compiler?" );
}

#ifdef DOJO_SHADERS_AVAILABLE

const void* Shader::_getUniformData( const Uniform& uniform, const Renderable& user )
{
	auto& r = Platform::singleton().getRenderer();

	static GLint tempInt[2];
	static Vector tmpVec;
    auto builtin = uniform.builtInUniform;
    switch ( builtin )
    {
		case BU_NONE:
			//TODO make global uniforms and remove this stuff
			return uniform.userUniformCallback( user ); //call the user callback and be happy
		case BU_WORLD:
			return &r.currentState.world;
		case BU_VIEW:
			return &r.currentState.view;
		case BU_PROJECTION:
			return &r.currentState.projection;
        case BU_WORLDVIEW:
            return &(r.currentState.worldView);
		case BU_WORLDVIEWPROJ:
			return &r.currentState.worldViewProjection;
		case BU_OBJECT_COLOR:
			return &user.color;
		case BU_VIEW_DIRECTION:
			return &r.currentState.viewDirection;
		case BU_TIME:
			DEBUG_TODO;
			return nullptr;
		case BU_TARGET_DIMENSION:
            return &r.currentState.targetDimension;
		case BU_TARGET_PIXEL:
			tmpVec = {
				1.f / r.currentState.targetDimension.x,
				1.f / r.currentState.targetDimension.y 
			};
			return &tmpVec;
		default: //texture stuff
        {
            if( builtin >= BU_TEXTURE_0 && builtin <= BU_TEXTURE_N )
            {
                tempInt[0] = builtin - BU_TEXTURE_0;
                return &tempInt;
            }
            else if( builtin >= BU_TEXTURE_0_DIMENSION && builtin <= BU_TEXTURE_N_DIMENSION )
            {
                Texture* t = user.getTexture( builtin - BU_TEXTURE_0_DIMENSION );
                tempInt[0] = t->getWidth();
                tempInt[1] = t->getHeight();
                return &tempInt;
            }
            else if( builtin >= BU_TEXTURE_0_TRANSFORM && builtin <= BU_TEXTURE_N_TRANSFORM )
            {
                return &user.getTextureUnit( builtin - BU_TEXTURE_0_TRANSFORM ).getTransform();
            }
			else
			{
				DEBUG_ASSERT(false, "Shader built-in not recognized");
				return nullptr;
			}
        }
    }
}

void Shader::use( const Renderable& user )
{
	DEBUG_ASSERT( isLoaded(), "tried to use a Shader that wasn't loaded" );

	glUseProgram( mGLProgram );

	//bind the uniforms and the attributes
	for( auto& uniform : mUniformMap )
	{
		const void* ptr = _getUniformData( uniform.second, user );

		if( ptr == nullptr ) //no data provided, skip
			break;

		//assign the data to the uniform
		//yes, this code is ugly...but don't be scared, it's as fast as a single glUniform in release :)
		//the types supported here are only the GLSL ES 2.0 types specified at 
		//http://www.khronos.org/registry/gles/specs/2.0/GLSL_ES_Specification_1.0.17.pdf, page 18
		switch ( uniform.second.type )
		{
		case GL_FLOAT:   glUniform1fv( uniform.second.location, uniform.second.count, (GLfloat*)ptr ); break;
		case GL_FLOAT_VEC2:   glUniform2fv( uniform.second.location, uniform.second.count, (GLfloat*)ptr ); break;
		case GL_FLOAT_VEC3:   glUniform3fv( uniform.second.location, uniform.second.count, (GLfloat*)ptr ); break;
		case GL_FLOAT_VEC4:   glUniform4fv( uniform.second.location, uniform.second.count, (GLfloat*)ptr ); break;
		case GL_INT: case GL_SAMPLER_2D:	case GL_SAMPLER_CUBE:	case GL_BOOL:
			glUniform1iv( uniform.second.location, uniform.second.count, (GLint*)ptr ); break; //this call also sets the samplers
		case GL_INT_VEC2:   case GL_BOOL_VEC2: glUniform2iv( uniform.second.location, uniform.second.count, (GLint*)ptr ); break;
		case GL_INT_VEC3:   case GL_BOOL_VEC3: glUniform3iv( uniform.second.location, uniform.second.count, (GLint*)ptr ); break;
		case GL_INT_VEC4:   case GL_BOOL_VEC4: glUniform4iv( uniform.second.location, uniform.second.count, (GLint*)ptr ); break;

		case GL_FLOAT_MAT2:   glUniformMatrix2fv( uniform.second.location, uniform.second.count, false, (GLfloat*)ptr ); break;
		case GL_FLOAT_MAT3:   glUniformMatrix3fv( uniform.second.location, uniform.second.count, false, (GLfloat*)ptr ); break;
		case GL_FLOAT_MAT4:   glUniformMatrix4fv( uniform.second.location, uniform.second.count, false, (GLfloat*)ptr ); break;

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
	auto desc =	Table::loadFromFile( filePath );

	//compose preprocessor flags
	mPreprocessorHeader.clear();
	auto& defines = desc.getTable( "defines" );

	for( auto& entry : defines )
		mPreprocessorHeader += std::string("#define ") + entry.second->getAsString().ASCII() + "\n";

	//grab all types
	for( int i = 0; i < (int)ShaderProgramType::_Count; ++i )
		_assignProgram( desc, (ShaderProgramType)i );

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

		//get attributes and their locations
		for( int i = 0;; ++i )
		{
			glGetActiveAttrib( mGLProgram, i, sizeof(namebuf), &nameLength, &size, &type, namebuf );

			if( glGetError() != GL_NO_ERROR )
				break;
			else
			{
				GLint loc = glGetAttribLocation( mGLProgram, namebuf );

				if( loc >= 0 )
				{
					mAttributeMap[ namebuf ] = VertexAttribute(
						loc,
						size,
						_getAttributeForName( namebuf ) );
				}
			}
		}
	}

	return loaded;
}

#else

void Shader::use( Renderable* user )
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
	for( int i = 0; i < (int)ShaderProgramType::_Count; ++i )
	{
		if( mOwnsProgram[i] )
		{
			pProgram[i]->onUnload( false );
			SAFE_DELETE( pProgram[i] );
		}
	}

	loaded = false;
}