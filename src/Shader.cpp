#include "Shader.h"

#include "ResourceGroup.h"
#include "Renderable.h"
#include "GameState.h"
#include "Viewport.h"
#include "Renderer.h"
#include "Texture.h"

#include "dojo_gl_header.h"
#include "range.h"
#include "TinySHA1.h"
#include "Base64.h"
#include "FileStream.h"
#include "Path.h"

using namespace Dojo;

Shader::NameBuiltInUniformMap Shader::sBuiltiInUniformsNameMap; //TODO implement this with an initializer list when VS decides to work with it
Shader::NameBuiltInAttributeMap Shader::sBuiltInAttributeNameMap; //TODO ^

void Shader::_populateUniformNameMap() {
	DEBUG_ASSERT(sBuiltiInUniformsNameMap.empty(), "The name-> builtinuniform map should be empty when populating");

	for (int i = 0; i < DOJO_MAX_TEXTURES; ++i) {
		std::string base = "TEXTURE_";
		base += '0' + (char)i;

		sBuiltiInUniformsNameMap[base] = BuiltInUniform(BU_TEXTURE_0 + i);
		sBuiltiInUniformsNameMap[base + "_DIMENSION"] = BuiltInUniform(BU_TEXTURE_0_DIMENSION + i);
	}

	sBuiltiInUniformsNameMap["WORLD"] = BU_WORLD;
	sBuiltiInUniformsNameMap["VIEW"] = BU_VIEW;
	sBuiltiInUniformsNameMap["PROJECTION"] = BU_PROJECTION;
	sBuiltiInUniformsNameMap["WORLDVIEW"] = BU_WORLDVIEW;
	sBuiltiInUniformsNameMap["WORLDVIEWPROJ"] = BU_WORLDVIEWPROJ;
	sBuiltiInUniformsNameMap["VIEW_DIRECTION"] = BU_VIEW_DIRECTION;
	sBuiltiInUniformsNameMap["OBJECT_COLOR"] = BU_OBJECT_COLOR;
	sBuiltiInUniformsNameMap["TIME"] = BU_TIME;
	sBuiltiInUniformsNameMap["TARGET_DIMENSION"] = BU_TARGET_DIMENSION;
	sBuiltiInUniformsNameMap["TARGET_DIMENSION_INV"] = BU_TARGET_DIMENSION_INV;
}

void Shader::_populateAttributeNameMap() {
	DEBUG_ASSERT(sBuiltInAttributeNameMap.empty(), "The name-> builtinattribute map should be empty when populating");

	for (int i = 0; i < DOJO_MAX_TEXTURES; ++i) {
		std::string base = "TEXCOORD_";
		base += '0' + (char)i;

		sBuiltInAttributeNameMap[base] = (VertexField)((byte)VertexField::UV0 + i);
	}

	sBuiltInAttributeNameMap["POSITION"] = VertexField::Position3D;
	sBuiltInAttributeNameMap["POSITION_2D"] = VertexField::Position2D;
	sBuiltInAttributeNameMap["NORMAL"] = VertexField::Normal;
	sBuiltInAttributeNameMap["COLOR"] = VertexField::Color;
}

Shader::BuiltInUniform Shader::_getUniformForName(const std::string& name) {
	if (sBuiltiInUniformsNameMap.empty()) { //populate it the first time only //HACK because VS12 doesn't support initializing the map with an initializer list
		_populateUniformNameMap();
	}

	auto elem = sBuiltiInUniformsNameMap.find(name);
	return (elem != sBuiltiInUniformsNameMap.end()) ? elem->second : BU_NONE;
}

VertexField Shader::_getAttributeForName(const std::string& name) {
	if (sBuiltInAttributeNameMap.empty()) {
		_populateAttributeNameMap();
	}

	auto elem = sBuiltInAttributeNameMap.find(name);
	DEBUG_ASSERT(elem != sBuiltInAttributeNameMap.end(), "Invalid attribute name");
	return elem != sBuiltInAttributeNameMap.end() ? elem->second : VertexField::None;
}

Shader::Shader(optional_ref<ResourceGroup> creator, const utf::string& filePath) :
	Resource(creator, filePath) {
	memset(pProgram, 0, sizeof(pProgram)); //init to null
}

ShaderProgram& Shader::_assignProgram(const Table& desc, ShaderProgramType type) {
	static const utf::string typeKeyMap[] = { "vertexShader", "fragmentShader" };
	auto typeID = (unsigned char)type;

	//check if this program is immediate or not
	//it is file-based if the resource can be found in the current RG
	auto& keyValue = desc.getString(typeKeyMap[typeID]);

	DEBUG_ASSERT_INFO(keyValue.not_empty(), "No shader found in .shader file", "type = " + typeKeyMap[typeID]);

	if (auto program = getCreator().unwrap().getProgram(keyValue).to_ref()) {
		if (mPreprocessorHeader.size()) { //some preprocessor flags are set - copy the existing program and recompile it
			mOwnedPrograms.emplace_back(program.get().cloneWithHeader(mPreprocessorHeader));
			program = *mOwnedPrograms.back();
		}
		else {
			DEBUG_ASSERT_INFO(program.get().getType() == type, "The linked shader is of the wrong type", "expected type = " + typeKeyMap[typeID]);
		}
		return (pProgram[typeID] = program.get()).unwrap();
	}
	else {
		mOwnedPrograms.emplace_back(make_unique<ShaderProgram>(type, mPreprocessorHeader + keyValue.bytes()));
		return (pProgram[typeID] = *mOwnedPrograms.back()).unwrap();
	}
}

void Shader::setUniformCallback(const std::string& name, const UniformCallback& dataBinder) {
	for (auto&& uniform : mUniforms) {
		if (uniform.name == name) {
			uniform.customDataBinding = dataBinder;
			return;
		}
	}

	DEBUG_MESSAGE("WARNING: can't find a Shader uniform named \"" + name + "\". Was it optimized away by the compiler?");
}

static int tempInt[2];
static Vector tmpVec;
static Matrix tmpMat;

const void* Shader::_getUniformData(const GlobalUniformData& currentState, const Uniform& uniform, const RenderState& user) {
	auto builtin = uniform.builtInUniform;

	switch (builtin) {
	case BU_NONE:
		//TODO make global uniforms and remove this stuff
		return uniform.customDataBinding(user); //call the user callback and be happy

	case BU_WORLD:
		return &user.getTransform();

	case BU_VIEW:
		return &currentState.view;

	case BU_PROJECTION:
		return &currentState.projection;

	case BU_WORLDVIEW:
		return &currentState.worldView;

	case BU_WORLDVIEWPROJ:
		return &currentState.worldViewProjection;

	case BU_OBJECT_COLOR:
		return &user.color;

	case BU_VIEW_DIRECTION:
		return &currentState.viewDirection;

	case BU_TIME:
		DEBUG_TODO;
		return nullptr;

	case BU_TARGET_DIMENSION:
		return &currentState.targetDimension;

	case BU_TARGET_DIMENSION_INV:
		tmpVec = {
			1.f / currentState.targetDimension.x,
			1.f / currentState.targetDimension.y
		};
		return &tmpVec;
	default: { //texture stuff
		if (builtin >= BU_TEXTURE_0 and builtin <= BU_TEXTURE_N) {
			tempInt[0] = builtin - BU_TEXTURE_0;
			return &tempInt;
		}
		else if (builtin >= BU_TEXTURE_0_DIMENSION and builtin <= BU_TEXTURE_N_DIMENSION) {
			auto& t = user.getTexture(builtin - BU_TEXTURE_0_DIMENSION).unwrap();
			tempInt[0] = t.getWidth();
			tempInt[1] = t.getHeight();
			return &tempInt;
		}
		else {
			FAIL("Shader built-in not recognized");
		}
	}
	}
}

void Shader::bind() const {
	DEBUG_ASSERT(isLoaded(), "tried to use a Shader that wasn't loaded");

	glUseProgram(mGLProgram);
}

void Shader::loadUniforms(const GlobalUniformData& currentState, const RenderState& user) {
	DEBUG_ASSERT(isLoaded(), "tried to use a Shader that wasn't loaded");

	//bind the uniforms and the attributes
	for (auto&& uniform : mUniforms) {
		const void* ptr = _getUniformData(currentState, uniform, user);

		if (ptr == nullptr) { //no data provided, skip
			break;
		}

		//assign the data to the uniform
		//yes, this code is ugly...but don't be scared, it's as fast as a single glUniform in release :)
		//the types supported here are only the GLSL ES 2.0 types specified at
		//http://www.khronos.org/registry/gles/specs/2.0/GLSL_ES_Specification_1.0.17.pdf, page 18
		switch (uniform.type) {
		case GL_FLOAT:
			glUniform1fv(uniform.location, uniform.count, (GLfloat*)ptr);
			break;

		case GL_FLOAT_VEC2:
			glUniform2fv(uniform.location, uniform.count, (GLfloat*)ptr);
			break;

		case GL_FLOAT_VEC3:
			glUniform3fv(uniform.location, uniform.count, (GLfloat*)ptr);
			break;

		case GL_FLOAT_VEC4:
			glUniform4fv(uniform.location, uniform.count, (GLfloat*)ptr);
			break;

		case GL_INT:
		case GL_SAMPLER_2D:
		case GL_SAMPLER_CUBE:
		case GL_BOOL:
			glUniform1iv(uniform.location, uniform.count, (int*)ptr);
			break; //this call also sets the samplers

		case GL_INT_VEC2:
		case GL_BOOL_VEC2:
			glUniform2iv(uniform.location, uniform.count, (int*)ptr);
			break;

		case GL_INT_VEC3:
		case GL_BOOL_VEC3:
			glUniform3iv(uniform.location, uniform.count, (int*)ptr);
			break;

		case GL_INT_VEC4:
		case GL_BOOL_VEC4:
			glUniform4iv(uniform.location, uniform.count, (int*)ptr);
			break;

		case GL_FLOAT_MAT2:
			glUniformMatrix2fv(uniform.location, uniform.count, false, (GLfloat*)ptr);
			break;

		case GL_FLOAT_MAT3:
			glUniformMatrix3fv(uniform.location, uniform.count, false, (GLfloat*)ptr);
			break;

		case GL_FLOAT_MAT4:
			glUniformMatrix4fv(uniform.location, uniform.count, false, (GLfloat*)ptr);
			break;

		default:
			//not found... but it's not possible to warn each frame. Do place a brk here if unsure
			break;
		}
	}

}

utf::string _getCachedBinaryPath(SHA1& sha) {
	//decide the filename of the binary
	SHA1::digest8_t digest;
	return Platform::singleton().getShaderCachePath() + "/" + Path::removeInvalidChars(Base64::fromBytes(sha.getDigestBytes(digest), sizeof(digest)));
}

Shader::Binary Shader::_getCachedBinary(const utf::string& path) {
	auto file = Platform::singleton().getFile(path);
	if (file->open(Stream::Access::Read)) {
		std::string content((size_t)file->getSize(), 0);
		file->readToFill(content);

		return{ content.substr(4), *(uint32_t*)(content.data()) };
	}
	return{};
}

std::string Shader::Binary::toString() const {
	return std::string((char*)&format, 4) + bytes;
}

void Shader::_storeCachedBinary(const utf::string& path, const Shader::Binary& binary) const {
	auto file = Platform::singleton().getFile(path);
	if (file->open(Stream::Access::WriteOnly)) {
		file->write(binary.toString());
	}
}

bool Shader::onLoad() {
	DEBUG_ASSERT(not isLoaded(), "cannot reload an already loaded Shader");

	loaded = false;

	int linked = 0;

	//load the descriptor table
	auto desc = Table::loadFromFile(filePath);

	//compose preprocessor flags
	mPreprocessorHeader.clear();
	auto& defines = desc.getTable("defines");

	for (auto&& entry : defines) {
		mPreprocessorHeader += "#define " + entry.second->getAs<std::string>() + "\n";
	}

	auto sha = SHA1{};

	//grab all types
	for (int i = 0; i < (int)ShaderProgramType::_Count; ++i) {
		auto& source = _assignProgram(desc, (ShaderProgramType)i).getSourceString();
		sha.processBytes(source.data(), source.length());
	}
	auto cachedPath = _getCachedBinaryPath(sha);

	//link the shaders together in this high level shader
	mGLProgram = glCreateProgram();

	auto binary = _getCachedBinary(cachedPath);
	if (binary) {
		glProgramBinary(mGLProgram, binary.format, binary.bytes.data(), binary.bytes.length());

		glGetProgramiv(mGLProgram, GL_LINK_STATUS, &linked);
	}

	if (linked == 0) {
		//ensure they're loaded
		for (auto&& program : pProgram) {
			if (not program.unwrap().isLoaded()) {
				if (not program.unwrap().onLoad()) { //one program was not loaded, the shader can't work
					return loaded;
				}
			}
		}

		for (auto&& program : pProgram) {
			glAttachShader(mGLProgram, program.unwrap().getGLShader());
		}
		glLinkProgram(mGLProgram);

		//now as it was successful, store the compiled shader for next time
		Binary obj;
		GLsizei length;

		glGetProgramiv(mGLProgram, GL_PROGRAM_BINARY_LENGTH, &length);
		obj.bytes.resize(length);
		glGetProgramBinary(mGLProgram, length, nullptr, &obj.format, (char*)obj.bytes.data());

		//check if the linking went ok
		glGetProgramiv(mGLProgram, GL_LINK_STATUS, &linked);

		if (linked) {
			_storeCachedBinary(cachedPath, obj);
		}
	}


	loaded = linked != 0;
	DEBUG_ASSERT(linked, "Could not link a shader program");

	if (linked) {
		GLchar namebuf[1024];
		int nameLength, size;
		uint32_t type;
		GLint elemCount;

		//get uniforms and their locations
		glGetProgramiv(mGLProgram, GL_ACTIVE_UNIFORMS, &elemCount);

		for (auto i : range(elemCount)) {
			glGetActiveUniform(mGLProgram, i, sizeof(namebuf), &nameLength, &size, &type, namebuf);
			auto loc = glGetUniformLocation(mGLProgram, namebuf);

			if (loc >= 0) { //loc < 0 means that this is a OpenGL-builtin such as gl_WorldViewProjectionMatrix
				mUniforms.emplace_back(
					namebuf,
					loc,
					size,
					type,
					_getUniformForName(namebuf)
				);
			}
		}

		//get attributes and their locations
		glGetProgramiv(mGLProgram, GL_ACTIVE_ATTRIBUTES, &elemCount);

		for (auto i : range(elemCount)) {
			glGetActiveAttrib(mGLProgram, i, sizeof(namebuf), &nameLength, &size, &type, namebuf);
			auto loc = glGetAttribLocation(mGLProgram, namebuf);

			if (loc >= 0) {
				mAttributes.emplace_back(
					loc,
					size,
					_getAttributeForName(namebuf)
				);
			}
		}
	}

	return loaded;
}

void Shader::onUnload(bool soft /* = false */) {
	DEBUG_ASSERT(isLoaded(), "This shader was already unloaded");

	//only manage the programs that aren't shared
	for (auto&& program : mOwnedPrograms) {
		program->onUnload(false);
	}

	mOwnedPrograms.clear();

	loaded = false;
}

