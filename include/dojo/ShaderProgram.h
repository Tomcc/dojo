#pragma once

#include "dojo_common_header.h"

#include "Resource.h"
#include "ShaderProgramType.h"

namespace Dojo {
	class ResourceGroup;

	///a ShaderProgram is a single programmable pipeline program step, loaded from a PSH, VSH or a string
	class ShaderProgram : public Resource {
	public:

		static const utf::string fragmentExtension, vertexExtension;

		///"real file" Resource constructor. When onLoad is called, it will use filePath to load its contents
		ShaderProgram(optional_ref<ResourceGroup> creator, const utf::string& filePath);

		///"immediate" Resource constructor. When onLoad is called, it will use the utf::string and type to load
		ShaderProgram(ShaderProgramType type, const std::string& contents);

		///returns the usage type of this shader, ie. fragment or vertex shader
		ShaderProgramType getType() {
			return mType;
		}

		uint32_t getGLShader() {
			return mGLShader;
		}

		const std::string& getSourceString() const {
			return mContentString;
		}

		///creates a new ShaderProgram using the source of this one, concatenated with the given preprocessor header
		Unique<ShaderProgram> cloneWithHeader(const std::string& preprocessorHeader);

		virtual bool onLoad();
		virtual void onUnload(bool soft = false);

	protected:

		std::string mContentString;

		ShaderProgramType mType;
		uint32_t mGLShader;

		bool _load();
	};
}
