#include "stdafx.h"

#include "Shader.h"

#include "ResourceGroup.h"

using namespace Dojo;

void Shader::bind()
{
	DEBUG_ASSERT( isLoaded(), "tried to use a Shader that wasn't loaded" );

	glUseProgram( mGLProgram );
}

void Shader::_assignProgram( const Table& desc, ShaderProgram::Type type )
{
	static const String typeFileMap[] = { "vertexShaderFile", "fragmentShaderFile" };
	static const String typeKeyMap[] =	{ "vertexShader", "fragmentShader" };

	//check if this program is immediate or not
	mOwnsProgram[ type ] = desc.exists( typeFileMap[ type ] );

	if( mOwnsProgram[type] )
	{
		String name = desc.getString( typeFileMap[ type ] );
		pProgram[ type ] = getCreator()->getProgram( name );
		DEBUG_ASSERT_INFO( pProgram[ type ], "shader program not found", name );
	}
	else //load immediate
	{
		auto& content = desc.getString( typeKeyMap[type] );

		pProgram[ type ] = new ShaderProgram( type, content.ASCII() );
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

	return loaded;
}

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
}