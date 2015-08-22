#include "ShaderProgram.h"

#include "Platform.h"
#include "FileStream.h"
#include "Path.h"

using namespace Dojo;

const utf::string ShaderProgram::fragmentExtension = "fragment", ShaderProgram::vertexExtension = "vertex";

///"real file" Resource constructor. When onLoad is called, it will use filePath to load its contents
ShaderProgram::ShaderProgram(ResourceGroup* creator, const utf::string& filePath) :
	Resource(creator, filePath) {
	//guess the type from the extension
	utf::string ext = Path::getFileExtension(filePath);

	if (ext == vertexExtension) {
		mType = ShaderProgramType::VertexShader;
	}
	else if (ext == fragmentExtension) {
		mType = ShaderProgramType::FragmentShader;
	}
	else {
		FAIL( "Unsupported shader type" );
	}
}

ShaderProgram::ShaderProgram(ShaderProgramType type, const std::string& contents) :
	Resource(nullptr),
	mContentString(contents),
	mType(type) {
	DEBUG_ASSERT(mContentString.size(), "No shader code was defined (empty string)");
}

bool ShaderProgram::_load() {
	static const GLuint typeGLTypeMap[] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};

	GLint compiled, sourceLength = mContentString.size();
	const char* src = mContentString.c_str();

	mGLShader = glCreateShader(typeGLTypeMap[(byte)mType]);

	glShaderSource(mGLShader, 1, &src, &sourceLength); //load the program source

	glCompileShader(mGLShader);

	CHECK_GL_ERROR;

	glGetShaderiv(mGLShader, GL_COMPILE_STATUS, &compiled);

	loaded = compiled != 0;

	if (!loaded) {
		//grab some info about the error
		GLint blen = 0;
		GLsizei slen = 0;

		glGetShaderiv(mGLShader, GL_INFO_LOG_LENGTH, &blen);

		if (blen > 1) {
			utf::string compilerLog(blen, 0);
			glGetShaderInfoLog(mGLShader, blen, &slen, (GLchar*)compilerLog.bytes().data());
			DEBUG_MESSAGE( "Compiler error:\n" + compilerLog );
		}

		FAIL("A shader program failed to compile!");
	}

	return loaded;
}

void ShaderProgram::onUnload(bool soft /* = false */) {
	DEBUG_ASSERT( isLoaded(), "This Shader Program is already unloaded" );

	glDeleteShader(mGLShader);
	mGLShader = GL_NONE;

	loaded = false;
}

bool ShaderProgram::onLoad() {
	DEBUG_ASSERT( !isLoaded(), "Cannot reload an already loaded program" );

	if (getFilePath().size()) { //try loading from file
		auto file = Platform::singleton().getFile(filePath);

		if (file->open(Stream::Access::Read)) {
			auto size = file->getSize();
			mContentString.resize((size_t)size);

			file->read((byte*)mContentString.data(), size);

			loaded = _load(); //load from the temp buffer
		}
	}
	else { //load from the in-memory string
		loaded = _load();
	}

	return loaded;
}

Unique<ShaderProgram> ShaderProgram::cloneWithHeader(const std::string& preprocessorHeader) {
	DEBUG_ASSERT(preprocessorHeader.size(), "The preprocessor header can't be empty");

	return make_unique<ShaderProgram>(mType, preprocessorHeader + mContentString);
}
