#include "ShaderProgram.h"

#include "Platform.h"
#include "FileStream.h"
#include "Path.h"

#include <glad/glad.h>

using namespace Dojo;

const utf::string_view ShaderProgram::fragmentExtension = "fragment", ShaderProgram::vertexExtension = "vertex";

///"real file" Resource constructor. When onLoad is called, it will use filePath to load its contents
ShaderProgram::ShaderProgram(optional_ref<ResourceGroup> creator, utf::string_view filePath) :
	Resource(creator, filePath) {
	//guess the type from the extension
	auto ext = Path::getFileExtension(filePath);

	if (ext == vertexExtension) {
		mType = ShaderProgramType::VertexShader;
	}
	else if (ext == fragmentExtension) {
		mType = ShaderProgramType::FragmentShader;
	}
	else {
		FAIL("Unsupported shader type");
	}
}

ShaderProgram::ShaderProgram(ShaderProgramType type, std::string&& contents) :
	Resource(),
	mContentString(std::move(contents)),
	mType(type) {
	DEBUG_ASSERT(mContentString.size(), "No shader code was defined (empty string)");
}

bool ShaderProgram::_load() {
	static const uint32_t typeGLTypeMap[] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };

	loaded = false;

	auto currentDir = Path::getParentDirectory(filePath);
	while (true) {
		auto idx = mContentString.find("#include");
		if (idx == std::string::npos) {
			break;
		}

		auto start = mContentString.find('"', idx) + 1;
		auto end = mContentString.find('"', start);
		auto path = currentDir + utf::string(mContentString.substr(start, end - start));

		auto file = Platform::singleton().getFile(path);

		if (not file->open(Stream::Access::Read)) {
			DEBUG_MESSAGE("Cannot include file:\n" + path);
			return false;
		}

		auto size = file->getSize();
		//create enough space in the string
		mContentString.replace(idx, end - idx + 1, (size_t)size, 'X');

		file->read((uint8_t*)mContentString.data() + idx, size);
	}

	//finally, append the version in front
	auto buildUnit = "#version 100\n" + mContentString;

	int compiled, sourceLength = buildUnit.size();
	const char* src = buildUnit.c_str();

	mGLShader = glCreateShader(typeGLTypeMap[(uint8_t)mType]);

	glShaderSource(mGLShader, 1, &src, &sourceLength); //load the program source

	glCompileShader(mGLShader);

	glGetShaderiv(mGLShader, GL_COMPILE_STATUS, &compiled);

	loaded = compiled != 0;

	if (not loaded) {
		//grab some info about the error
		int blen = 0;
		GLsizei slen = 0;

		glGetShaderiv(mGLShader, GL_INFO_LOG_LENGTH, &blen);

		if (blen > 1) {
			utf::string compilerLog(blen, 0);
			glGetShaderInfoLog(mGLShader, blen, &slen, (GLchar*)compilerLog.bytes().data());
			DEBUG_MESSAGE("Compiler error:\n" + compilerLog);
		}
	}

	return loaded;
}

void ShaderProgram::onUnload(bool soft /* = false */) {
	DEBUG_ASSERT(isLoaded(), "This Shader Program is already unloaded");

	glDeleteShader(mGLShader);
	mGLShader = GL_NONE;

	loaded = false;
}

bool ShaderProgram::onLoad() {
	DEBUG_ASSERT(not isLoaded(), "Cannot reload an already loaded program");

	if (getFilePath().not_empty()) { //try loading from file
		auto file = Platform::singleton().getFile(filePath);

		if (file->open(Stream::Access::Read)) {
			auto size = file->getSize();
			mContentString.resize((size_t)size);

			file->read((uint8_t*)mContentString.data(), size);
			file->close(); //close as soon as possible to release the file if there's an error

			loaded = _load(); //load from the temp buffer
		}
	}
	else { //load from the in-memory string
		loaded = _load();
	}

	DEBUG_ASSERT(loaded, "A shader program failed to compile!");

	return loaded;
}

Unique<ShaderProgram> ShaderProgram::cloneWithHeader(const std::string& preprocessorHeader) {
	DEBUG_ASSERT(preprocessorHeader.size(), "The preprocessor header can't be empty");

	return make_unique<ShaderProgram>(mType, preprocessorHeader + mContentString);
}
