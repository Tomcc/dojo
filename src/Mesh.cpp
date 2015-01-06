#include "stdafx.h"

#include "Mesh.h"

#include "Utils.h"
#include "Platform.h"
#include "Shader.h"
#include "dojomath.h"
#include "TriangleMode.h"

using namespace Dojo;

const GLuint glFeatureStateMap[] =
{
	GL_VERTEX_ARRAY, //VF_POSITION2D,
	GL_VERTEX_ARRAY, //VF_POSITION3D,
	GL_COLOR_ARRAY, //VF_COLOR,
	GL_NORMAL_ARRAY, //VF_NORMAL,
	GL_TEXTURE_COORD_ARRAY, //VF_UV,
	GL_TEXTURE_COORD_ARRAY, //VF_UV_1,
};

const int Mesh::VERTEX_FIELD_SIZES[] = { 
	2 * sizeof( GLfloat ), //position 2D
	3 * sizeof( GLfloat ),  //position 3D
	4 * sizeof( GLubyte ),  //color
	3 * sizeof( GLfloat ),  //normal
	2 * sizeof( GLfloat ),  //uv0
	2 * sizeof( GLfloat )
};

Mesh::Mesh(ResourceGroup* creator /*= nullptr */) :
Resource(creator)
{
	//set all fields to zero
	memset(vertexFieldOffset, 0xff, sizeof(vertexFieldOffset));

	//default index size is 16
	setIndexByteSize(sizeof(GLushort));
}

Mesh::Mesh(ResourceGroup* creator, const String& filePath) :
Resource(creator, filePath) 
{
	//set all fields to zero
	memset(vertexFieldOffset, 0xff, sizeof(vertexFieldOffset));

	//default index size is 16
	setIndexByteSize(sizeof(GLushort));
}

Mesh::~Mesh() {
#ifndef DOJO_DISABLE_VAOS
	if (vertexArrayDesc)
		glDeleteVertexArrays(1, &vertexArrayDesc);
#endif

	if (loaded)
		onUnload();
}

void Mesh::destroyBuffers() {
	auto cleanup = std::move(vertices);
	cleanup = std::move(indices);
}

void Mesh::begin(int extimatedVerts /*= 1 */) {
	//be sure that we aren't already building
	DEBUG_ASSERT(extimatedVerts > 0, "begin: extimated vertices for this batch must be more than 0");
	DEBUG_ASSERT(!isEditing(), "begin: this Mesh is already in Edit mode");

	vertices.clear();
	indices.clear();
	vertices.reserve(extimatedVerts*vertexSize);

	vertexCount = indexCount = 0;
	currentVertex = nullptr;

	max = Vector::MIN;
	min = Vector::MAX;

	editing = true;
}

void Mesh::beginAppend() {
	DEBUG_ASSERT(!isEditing(), "begin: this Mesh is already in Edit mode");
	DEBUG_ASSERT(dynamic, "can't call append() on a static mesh");
	DEBUG_ASSERT(vertices.size() > 0, "This mesh was never begin'd!");

	editing = true;
}

void Mesh::setIndexByteSize(byte bytenumber) {
	DEBUG_ASSERT(!editing, "setIndexByteSize must be called BEFORE begin!");
	DEBUG_ASSERT(
		bytenumber == 1 ||
		bytenumber == 2 ||
		bytenumber == 4, "setIndexByteSize: byteNumber must be either 1,2 or 4");

	indexSize = bytenumber;

	if (indexSize == 1)
	{
		indexGLType = GL_UNSIGNED_BYTE;
		indexMaxValue = 0xff;
	}
	else if (indexSize == 2)
	{
		indexGLType = GL_UNSIGNED_SHORT;
		indexMaxValue = 0xffff;
	}
	else if (indexSize == 4)
	{
#ifdef DOJO_32BIT_INDICES_AVAILABLE
		indexGLType = GL_UNSIGNED_INT;
		indexMaxValue = 0xffffffff;
#else
		DEBUG_FAIL("32 bit indices are disabled (force enabled defining DOJO_32BIT_INDICES_AVAILABLE)");
#endif
	}
}

void Mesh::setVertexFieldEnabled(VertexField f) {
	DEBUG_ASSERT(!editing, "setVertexFieldEnabled must be called BEFORE begin!");

	_offset(f) = vertexSize;
	vertexSize += VERTEX_FIELD_SIZES[(byte)f];
}

void Mesh::setVertexFields(const std::initializer_list<VertexField>& fs) {
	for (auto f : fs)
		setVertexFieldEnabled(f);
}


void Mesh::setDynamic(bool d) {
	dynamic = d;
}

void Mesh::index(IndexType idx) {
	DEBUG_ASSERT(isEditing(), "index: this Mesh is not in Edit mode");
	DEBUG_ASSERT(idx <= indexMaxValue, "index: the index passed is too big to be contained in this mesh's index format, see setIndexByteSize");

	auto curSize = indices.size();
	indices.resize(curSize + indexSize);

	switch (indexSize)
	{
	case 1:
		indices.back() = idx;
		break;
	case 2:
		*((unsigned short*)(indices.data() + curSize)) = idx;
		break;
	case 4:
		*((unsigned int*)(indices.data() + curSize)) = idx;
		break;
	}

	++indexCount;
}


void Mesh::_prepareVertex(const Vector& v)
{
	DEBUG_ASSERT( isEditing(), "_prepareVertex: this Mesh is not in Edit mode" );

	//grow the buffer to the needed size
	auto curSize = vertices.size();
	vertices.resize(curSize + vertexSize);

	currentVertex = (byte*)vertices.data() + curSize;

	min = Math::min(v, min);
	max = Math::max(v, max);

	++vertexCount;
}

int Mesh::vertex( float x, float y )
{				
	_prepareVertex(Vector(x,y));

	float* ptr = (float*)currentVertex;

	ptr[0] = x;
	ptr[1] = y;

	return getVertexCount() - 1;
}

int Mesh::vertex(const Vector& v) 
{
	_prepareVertex(v);

	if (isVertexFieldEnabled(VertexField::Position3D))
		*((Vector*)currentVertex) = v;
	else {
		float* ptr = (float*)currentVertex;

		ptr[0] = v.x;
		ptr[1] = v.y;
	}

	return getVertexCount() - 1;
}

int Mesh::vertex(float x, float y, float z)
{
	return vertex(Vector(x, y, z));
}

byte& Mesh::_offset(VertexField f) {
	return vertexFieldOffset[(byte)f];
}


byte& Mesh::_offset(VertexField f, int subID) {
	return vertexFieldOffset[(byte)((int)f + subID)];
}


void Mesh::appendRawVertexData(void* data, IndexType count) {
	int blobSize = count * vertexSize;
	int oldSize = vertices.size();

	vertices.resize(oldSize + blobSize);

	auto start = vertices.data() + oldSize;
	memcpy(start, data, blobSize);

	//TODO max and min??

	vertexCount += count;
}

int Mesh::getPrimitiveCount() const {
	auto elemCount = isIndexed() ? getIndexCount() : getVertexCount();
	switch (triangleMode) {
	case TriangleMode::TriangleList:		return elemCount / 3;
	case TriangleMode::TriangleStrip:		return elemCount - 2;
	case TriangleMode::LineStrip:			return elemCount - 1;
	case TriangleMode::LineList:            return elemCount / 2;
	case TriangleMode::PointList:			return elemCount;
	default:
		DEBUG_FAIL("Invalid triangle mode");
		return 0;
	}
}

void Mesh::uv(float u, float v, byte set /*= 0 */) {
	DEBUG_ASSERT(isEditing(), "uv: this Mesh is not in Edit mode");

	float* ptr = (float*)(currentVertex + _offset(VertexField::UV0, set));
	ptr[0] = u;
	ptr[1] = v;
}

void Mesh::uv(const Vector& uv, byte set /* = 0 */) {
	this->uv(uv.x, uv.y, set);
}

void Mesh::color(const Color& c) {

	DEBUG_ASSERT(isEditing(), "color: this Mesh is not in Edit mode");

	*((Color::RGBAPixel*)(currentVertex + _offset(VertexField::Color))) = c.toRGBA();
}

void Mesh::color(float r, float g, float b, float a) {
	color(Color(r, g, b, a));
}

void Mesh::normal(const Vector& n) {
	*((Vector*)(currentVertex + _offset(VertexField::Normal))) = n;
}

void Mesh::normal(float x, float y, float z) {
	DEBUG_ASSERT(isEditing(), "normal: this Mesh is not in Edit mode");

	normal(Vector(x,y,z));
}

void Mesh::_getVertexFieldData(VertexField field, int& outComponents, GLenum& outComponentsType, bool& outNormalized, void*& outOffset) {
	outOffset = (void*)_offset(field);

	switch (field)
	{
	case VertexField::Position2D: outComponentsType = GL_FLOAT; outComponents = 2; outNormalized = false; break;
	case VertexField::Position3D: outComponentsType = GL_FLOAT; outComponents = 3; outNormalized = false; break;
	case VertexField::Color: outComponentsType = GL_UNSIGNED_BYTE; outComponents = 4; outNormalized = true; break;
	case VertexField::Normal: outComponentsType = GL_FLOAT; outComponents = 3; outNormalized = false; break;

	default: //textures
		outComponentsType = GL_FLOAT;
		outComponents = 2;
		outNormalized = false;
		break;
	}
}

void Mesh::_bindAttribArrays( Shader* shader )
{
	glBindBuffer( GL_ARRAY_BUFFER, vertexHandle );

#ifdef DOJO_SHADERS_AVAILABLE
	if( shader ) //use custom attributes only
	{
		GLint components;
		GLenum componentsType;
		bool normalized;
		void* offset;

		for( auto& attr : shader->getAttributes() )
		{
			if( attr.second.builtInAttribute == VertexField::None || !isVertexFieldEnabled( attr.second.builtInAttribute ) )		//skip non-provided attributes
				continue;

			_getVertexFieldData( attr.second.builtInAttribute, components, componentsType, normalized, offset );

			glEnableVertexAttribArray( attr.second.location );
			glVertexAttribPointer(
				attr.second.location,
				components,
				componentsType,
				normalized,
				vertexSize,
				offset );

			CHECK_GL_ERROR;
		}
	}
	else //fixed function vertex binding //TODO remove this
#endif
	{
		//construct attributes
		for( int i = 0; i < (int)VertexField::_Count; ++i )
		{
			GLenum state = glFeatureStateMap[i];
			VertexField ft = (VertexField)i;

			if (ft >= VertexField::UV0 && ft <= VertexField::UVMax) //a texture
				glClientActiveTexture(GL_TEXTURE0 + ((int)ft - (int)VertexField::UV0)); //bind the correct texture (this has to be called *before* EnableClientState

			if( isVertexFieldEnabled( ft ) )	//bind data and client states
			{
				glEnableClientState( state );
				CHECK_GL_ERROR;

				void* fieldOffset = (void*)_offset(ft);

				switch( ft )
				{
				case VertexField::Position3D:			glVertexPointer(3, GL_FLOAT, vertexSize, fieldOffset);	break;
				case VertexField::Position2D:			glVertexPointer(2, GL_FLOAT, vertexSize, fieldOffset); break;
				case VertexField::Normal:				glNormalPointer(GL_FLOAT, vertexSize, fieldOffset);	break;
				case VertexField::Color:				glColorPointer(4, GL_UNSIGNED_BYTE, vertexSize, fieldOffset);	break;
				default: 
					if (ft >= VertexField::UV0 && ft <= VertexField::UVMax) //texture binding						
						glTexCoordPointer(2, GL_FLOAT, vertexSize, fieldOffset );	
					break;
				};

				CHECK_GL_ERROR;
			}
			else if( state != GL_VERTEX_ARRAY ) //do not disable the position
				glDisableClientState( state );
		}
	}

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, isIndexed() ? indexHandle : 0 ); //only bind the index buffer if existing (duh)

	CHECK_GL_ERROR;
}

bool Mesh::end()
{
	DEBUG_ASSERT(editing, "Can't call end() before begin()!");
	editing = false;

	DEBUG_ASSERT(!isLoaded() || dynamic, "Can't update a static mesh");

	//don't load empty meshes
	if (getVertexCount() == 0)
		return false;

	//create the VBO
	if( !vertexHandle )
		glGenBuffers(1, &vertexHandle );

	GLenum usage = (dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
	glBindBuffer(GL_ARRAY_BUFFER, vertexHandle);
	glBufferData(GL_ARRAY_BUFFER, vertices.size(), vertices.data(), usage);

	CHECK_GL_ERROR;

	//create the IBO
	if( isIndexed() ) //we support unindexed meshes
	{				
		if( !indexHandle )
			glGenBuffers(1, &indexHandle );

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexHandle );
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size(), indices.data(), usage);

		CHECK_GL_ERROR;						
	}

#ifndef DOJO_DISABLE_VAOS //if we're using VAOs
	//create the VAO
	if( !vertexArrayDesc )
		glGenVertexArrays( 1, &vertexArrayDesc );

	glBindVertexArray( vertexArrayDesc );

	CHECK_GL_ERROR;

	_bindAttribArrays();

	glBindVertexArray( 0 );
#endif
	
	loaded = glGetError() == GL_NO_ERROR;
	
	currentVertex = nullptr;
	
	//geometric hints
	center = (max + min)*0.5f;
	
	dimensions = max - min;

	if( !dynamic ) //won't be updated ever again
		destroyBuffers();
	
	return loaded;
}

void Mesh::bind( Shader* shader )
{		
#ifndef DOJO_DISABLE_VAOS

	DEBUG_ASSERT( vertexArrayDesc );
	glBindVertexArray( vertexArrayDesc );
#else
	_bindAttribArrays( shader ); //bind attribs each frame! (costly)
#endif

	CHECK_GL_ERROR; 
}

bool Mesh::onLoad()
{
	DEBUG_ASSERT( !isLoaded(), "onLoad: Mesh is already loaded" );

	if( !isReloadable() )
		return false;

	//load binary mesh
	char* data;
	Platform::singleton().loadFileContent( data, filePath );
		
	DEBUG_ASSERT_INFO( data, "onLoad: cannot find or read file", "path = " + filePath );
	
	char* ptr = data;
	
	//index size
	setIndexByteSize( *ptr++ );
	
	//triangle mode
	setTriangleMode( (TriangleMode)*ptr++ );
	
	//fields
	for (int i = 0; i < (int)VertexField::_Count; ++i)
	{
		if( *ptr++ )
			setVertexFieldEnabled( (VertexField)i );
	}
	
	//max and min
	Vector loadedMax;
	memcpy( &loadedMax, ptr, sizeof( Vector ) );
	ptr += sizeof( Vector );
	
	Vector loadedMin;
	memcpy( &loadedMin, ptr, sizeof( Vector ) );
	ptr += sizeof( Vector );
	
	//vertex count
	IndexType vc = *((int*) ptr);
	ptr += sizeof( int );
	
	//index count
	int ic = *((int*)ptr);
	ptr += sizeof( int );
		
	setDynamic( false );
	
	begin( vc );
	
	//grab vertex data
	vertices.resize(vc*vertexSize);
	memcpy( (char*)vertices.data(), ptr, vc * vertexSize );
	ptr += vc * vertexSize;
	
	//grab index data
	if( ic )
	{
		indices.resize(ic*indexSize);
		memcpy( (char*)indices.data(), ptr, ic * indexSize );
	}
	
	max = loadedMax;
	min = loadedMin;

	vertexCount = vc;
	indexCount = ic;

	//push over to GPU
	return end();
}

void Mesh::onUnload(bool soft /*= false */) {
	DEBUG_ASSERT(isLoaded(), "onUnload: Mesh is not loaded");

	//when soft unloading, only unload file-based meshes
	if (!soft || isReloadable())
	{
		glDeleteBuffers(1, &vertexHandle);
		glDeleteBuffers(1, &indexHandle);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		vertexHandle = indexHandle = 0;

		destroyBuffers(); //free CPU side memory

		loaded = false;
	}
}

Vector& Mesh::getVertex(int idx) {
	int offset = isVertexFieldEnabled(VertexField::Position3D) ? _offset(VertexField::Position3D) : _offset(VertexField::Position2D);
	byte* ptr = (byte*)vertices.data() + (idx * vertexSize) + offset;

	return *(Vector*)ptr;
}

void Mesh::setIndex(int idxidx, IndexType idx) {
	DEBUG_ASSERT(idxidx >= 0 && idxidx < getIndexCount(), "Index out of bounds");

	switch (indexSize)
	{
	case 1:
		indices[idxidx] = idx;
		return;
	case 2:
		((unsigned short*)indices.data())[idxidx] = idx;
		return;
	default:
		((unsigned int*)indices.data())[idxidx] = idx;
		return;
	}
}


Mesh::IndexType Mesh::getIndex(int idxidx) const {
	DEBUG_ASSERT(idxidx >= 0 && idxidx < getIndexCount(), "Index out of bounds");

	switch (indexSize)
	{
	case 1:
		return indices[idxidx];
	case 2:
		return ((unsigned short*)indices.data())[idxidx];
	default:
		return ((unsigned int*)indices.data())[idxidx];
	}
}

void Mesh::eraseIndex(int idxidx) {
	DEBUG_ASSERT(idxidx >= 0 && idxidx < getIndexCount(), "Index out of bounds");

	auto i = indices.begin() + (idxidx * indexSize);
	indices.erase(i, i + indexSize);
	--indexCount;
}


void Mesh::cutSection(IndexType i1, IndexType i2) {
	DEBUG_ASSERT(isEditing(), "cutSection: this Mesh is not in Edit mode");
	DEBUG_ASSERT(i1 < getVertexCount() && i2 <= getVertexCount() && i1 <= i2, "Invalid indices passed");

	//easy part: cut out the vertex data
	auto diff = i2 - i1;
	auto size = diff * vertexSize;
	auto start = vertices.begin() + i1 * vertexSize;
	vertices.erase(start, start + size);

	//remove the indices
	if (isIndexed()) {

		for (int i = 0; i < getIndexCount(); ++i) {
			auto idx = getIndex(i);

			if (idx >= i1 && idx < i2) {
				eraseIndex(i);
				--i;
			}
			else if (idx >= i2) { //offset the new value
				setIndex(i, idx - diff);
			}
		}
	}

	//TODO recompute max and min
}

Unique<Mesh> Mesh::cloneWithSameFormat() const {
	auto c = make_unique<Mesh>();

	c->setIndexByteSize(indexSize);
	c->setTriangleMode(triangleMode);
	c->vertexSize = vertexSize;
	memcpy(c->vertexFieldOffset, vertexFieldOffset, sizeof(vertexFieldOffset));

	return c;
}

Unique<Mesh> Mesh::cloneFromSlice(IndexType vertexStart, IndexType vertexEnd, const Vector& translation /*= Vector::ZERO*/) const {
	DEBUG_ASSERT(!vertices.empty(), "This mesh is empty");
	DEBUG_ASSERT(vertexStart < getVertexCount() && vertexEnd <= getVertexCount() && vertexStart <= vertexEnd, "Indices out of bounds");

	auto c = cloneWithSameFormat();

	int n = vertexEnd - vertexStart;
	int off = vertexStart * vertexSize;
	int size = n * vertexSize;

	c->begin(n);

	//copy the vertices in the new mesh
	{
		c->vertexCount = n;
		c->vertices.resize(c->vertexCount * vertexSize);

		memcpy(c->vertices.data(), vertices.data() + off, size);

		for (int i = 0; i < c->vertexCount; ++i) {
			auto& v = c->getVertex(i);
			v += translation;

			c->max = Math::max(c->max, v);
			c->min = Math::min(c->min, v);
		}
	}

	//find the indices that were pointing to these vertices
	//and place new indices that replicate the same structure
	if (isIndexed()) {
		for (int i = 0; i < getIndexCount(); ++i) {
			auto idx = getIndex(i);
			if (idx >= vertexStart && idx < vertexEnd)
				c->index(idx - vertexStart);
		}
	}


	return c;
}
