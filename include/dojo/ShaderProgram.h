#pragma once

#include "dojo_common_header.h"

#include "Resource.h"
#include "ShaderProgramType.h"

namespace Dojo
{
	class ResourceGroup;

	///a ShaderProgram is a single programmable pipeline program step, loaded from a PSH, VSH or a string
	class ShaderProgram : public Resource
	{
	public:

		///"real file" Resource constructor. When onLoad is called, it will use filePath to load its contents
		ShaderProgram( ResourceGroup* creator, const String& filePath );

		///"immediate" Resource constructor. When onLoad is called, it will use the String and type to load
		/**
		std::string is used because it has to be a pure ASCII string
		*/
		ShaderProgram( ShaderProgramType type, const std::string& contents );

		///returns the usage type of this shader, ie. fragment or vertex shader
		ShaderProgramType getType()
		{
			return mType;
		}

		GLuint getGLShader()
		{
			return mGLShader;
		}

		///creates a new ShaderProgram using the source of this one, concatenated with the given preprocessor header
		ShaderProgram* cloneWithHeader( const std::string& preprocessorHeader );

		virtual bool onLoad();
		virtual void onUnload( bool soft = false );

	protected:

		std::string mContentString;

		ShaderProgramType mType;
		GLuint mGLShader;

		bool _load();
	};
}
