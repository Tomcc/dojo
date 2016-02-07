#include "Shader.h"

#include "ResourceGroup.h"
#include "Renderable.h"
#include "GameState.h"
#include "Viewport.h"
#include "Renderer.h"
#include "Texture.h"

#include "dojo_gl_header.h"

using namespace Dojo;

Shader::NameBuiltInUniformMap Shader::sBuiltiInUniformsNameMap; //TODO implement this with an initializer list when VS decides to work with it
Shader::NameBuiltInAttributeMap Shader::sBuiltInAttributeNameMap; //TODO ^

void Shader::_populateUniformNameMap() {
	DEBUG_ASSERT( sBuiltiInUniformsNameMap.empty(), "The name-> builtinuniform map should be empty when populating" );

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
	sBuiltiInUniformsNameMap["TARGET_PIXEL"] = BU_TARGET_PIXEL;
}

void Shader::_populateAttributeNameMap() {
	DEBUG_ASSERT( sBuiltInAttributeNameMap.empty(), "The name-> builtinattribute map should be empty when populating" );

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
	return (elem != sBuiltInAttributeNameMap.end()) ? elem->second : VertexField::None;
}

Shader::Shader(optional_ref<ResourceGroup> creator, const utf::string& filePath) :
	Resource(creator, filePath) {
	memset(pProgram, 0, sizeof( pProgram )); //init to null
}

void Shader::_assignProgram(const Table& desc, ShaderProgramType type) {
	static const utf::string typeKeyMap[] = {"vertexShader", "fragmentShader"};
	auto typeID = (unsigned char)type;

	//check if this program is immediate or not
	//it is file-based if the resource can be found in the current RG
	auto& keyValue = desc.getString(typeKeyMap[typeID]);

	DEBUG_ASSERT_INFO(keyValue.not_empty(), "No shader found in .shader file", "type = " + typeKeyMap[typeID]);
	
	if (auto program = getCreator().unwrap().getProgram(keyValue).cast()) {
		if (mPreprocessorHeader.size()) { //some preprocessor flags are set - copy the existing program and recompile it
			mOwnedPrograms.emplace_back(program.get().cloneWithHeader(mPreprocessorHeader));
			program = *mOwnedPrograms.back();
		}
		else {
			DEBUG_ASSERT_INFO(program.get().getType() == type, "The linked shader is of the wrong type", "expected type = " + typeKeyMap[typeID]);
		}
		pProgram[typeID] = program.get();
	}
	else {
		mOwnedPrograms.emplace_back(make_unique<ShaderProgram>(type, mPreprocessorHeader + keyValue.bytes()));
		pProgram[typeID] = *mOwnedPrograms.back();
	}
}

void Shader::setUniformCallback(const std::string& name, const UniformCallback& dataBinder) {
	for (auto&& uniform : mUniforms) {
		if (uniform.name == name) {
			uniform.customDataBinding = dataBinder;
			return;
		}
	}

	DEBUG_MESSAGE( "WARNING: can't find a Shader uniform named \"" + name + "\". Was it optimized away by the compiler?" );
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

	case BU_TARGET_PIXEL:
		tmpVec = {
			1.f / currentState.targetDimension.x,
			1.f / currentState.targetDimension.y
		};
		return &tmpVec;
	default: { //texture stuff
		if (builtin >= BU_TEXTURE_0 && builtin <= BU_TEXTURE_N) {
			tempInt[0] = builtin - BU_TEXTURE_0;
			return &tempInt;
		}
		else if (builtin >= BU_TEXTURE_0_DIMENSION && builtin <= BU_TEXTURE_N_DIMENSION) {
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

	CHECK_GL_ERROR;
}

bool Shader::onLoad() {
	DEBUG_ASSERT( !isLoaded(), "cannot reload an already loaded Shader" );

	loaded = false;

	//load the descriptor table
	auto desc = Table::loadFromFile(filePath);

	//compose preprocessor flags
	mPreprocessorHeader.clear();
	auto& defines = desc.getTable("defines");

	for (auto&& entry : defines) {
		mPreprocessorHeader += "#define " + entry.second->getAs<std::string>() + "\n";
	}

	//grab all types
	for (int i = 0; i < (int)ShaderProgramType::_Count; ++i) {
		_assignProgram(desc, (ShaderProgramType)i);
	}

	//ensure they're loaded
	for (auto&& program : pProgram) {
		if (!program.unwrap().isLoaded()) {
			if (!program.unwrap().onLoad()) { //one program was not loaded, the shader can't work
				return loaded;
			}
		}
	}

	//link the shaders together in this high level shader
	mGLProgram = glCreateProgram();

	for (auto&& program : pProgram) {
		glAttachShader(mGLProgram, program.unwrap().getGLShader());
	}

	glLinkProgram(mGLProgram);

	CHECK_GL_ERROR;

	//check if the linking went ok
	int linked;
	glGetProgramiv(mGLProgram, GL_LINK_STATUS, &linked);
	loaded = linked != 0;

	DEBUG_ASSERT(linked, "Could not link a shader program");

	if (linked) {
		GLchar namebuf[1024];
		int nameLength, size;
		uint32_t type;

		//get uniforms and their locations
		for (int i = 0; ; ++i) {
			glGetActiveUniform(mGLProgram, i, sizeof( namebuf ), &nameLength, &size, &type, namebuf);

			if (glGetError() != GL_NO_ERROR) { //check if this value existed
				break;
			}
			else { //store the Uniform data
				int loc = glGetUniformLocation(mGLProgram, namebuf);

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
		}

		//get attributes and their locations
		for (int i = 0;; ++i) {
			glGetActiveAttrib(mGLProgram, i, sizeof(namebuf), &nameLength, &size, &type, namebuf);

			if (glGetError() != GL_NO_ERROR) {
				break;
			}
			else {
				int loc = glGetAttribLocation(mGLProgram, namebuf);

				if (loc >= 0) {
					mAttributes.emplace_back(
						loc,
						size,
						_getAttributeForName(namebuf)
					);
				}
			}
		}
	}

	return loaded;
}

void Shader::onUnload(bool soft /* = false */) {
	DEBUG_ASSERT( isLoaded(), "This shader was already unloaded" );

	//only manage the programs that aren't shared
	for (auto&& program : mOwnedPrograms) {
		program->onUnload(false);
	}

	mOwnedPrograms.clear();

	loaded = false;
}
