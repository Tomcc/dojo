#include "Mesh.h"

#include "Platform.h"
#include "Shader.h"
#include "dojomath.h"
#include "PrimitiveMode.h"
#include "enum_cast.h"

#include "glad/glad.h"

using namespace Dojo;

struct VertexFieldInfo {
	uint32_t type;
	uint8_t components;
	bool normalized;
	uint8_t bytes;
}

static const VERTEX_FIELD_INFO[enum_cast(VertexField::_Count)] = {
	{ GL_FLOAT, 2, false, 2 * sizeof(GLfloat) },	// 	Position2D,
	{ GL_FLOAT, 3, false, 3 * sizeof(GLfloat) },	// 	Position3D,
	{ GL_UNSIGNED_BYTE, 4, true, 4 * sizeof(GLubyte) },	// 	Color,
	{ GL_INT_2_10_10_10_REV, 4, true, 1 * sizeof(GLuint) },	// 	Normal

	{ GL_HALF_FLOAT, 2, false, 2 * sizeof(GLshort) },	// 	UV
	{ GL_HALF_FLOAT, 2, false, 2 * sizeof(GLshort) },	// 	UV
};

bool Mesh::gBufferBindingsDirty = true;

Mesh::Mesh(optional_ref<ResourceGroup> creator /*= nullptr */) :
	Resource(creator) {
	//set all fields to max
	for (auto&& offset : vertexFieldOffset) {
		offset = 0xff;
	}

	//default index size is 16
	setIndexByteSize(sizeof(GLushort));
}

Mesh::Mesh(optional_ref<ResourceGroup> creator, utf::string_view filePath) :
	Resource(creator, filePath) {
	//set all fields to max
	for (auto&& offset : vertexFieldOffset) {
		offset = 0xff;
	}

	//default index size is 16
	setIndexByteSize(sizeof(GLushort));
}

Mesh::~Mesh() {
	if (loaded) {
		onUnload();
	}
}

void Mesh::destroyBuffers() {
	auto cleanup = std::move(vertices);
	cleanup = std::move(indices);
}

void Mesh::begin(IndexType extimatedVerts /*= 1 */) {
	//be sure that we aren't already building
	DEBUG_ASSERT(extimatedVerts > 0, "begin: extimated vertices for this batch must be more than 0");
	DEBUG_ASSERT(not isEditing(), "begin: this Mesh is already in Edit mode");
	DEBUG_ASSERT(indexMaxValue > extimatedVerts, "The index format chosen is too small");

	vertices.clear();
	indices.clear();
	vertices.reserve(extimatedVerts * vertexSize);

	vertexCount = indexCount = 0;
	currentVertex = nullptr;

	bounds = AABB::Invalid;
	vertexTransparency = false;

	editing = true;
}

void Mesh::beginAppend() {
	DEBUG_ASSERT(not isEditing(), "begin: this Mesh is already in Edit mode");
	DEBUG_ASSERT(dynamic, "can't call append() on a static mesh");
	DEBUG_ASSERT(vertices.size() > 0, "This mesh was never begin'd!");

	editing = true;
}

void Mesh::setIndexByteSize(uint8_t bytenumber) {
	DEBUG_ASSERT(not editing, "setIndexByteSize must be called BEFORE begin!");
	DEBUG_ASSERT(
		bytenumber == 1 or
		bytenumber == 2 or
		bytenumber == 4, "setIndexByteSize: byteNumber must be either 1,2 or 4");

	indexSize = bytenumber;

	if (indexSize == 1) {
		indexGLType = GL_UNSIGNED_BYTE;
		indexMaxValue = 0xff;
	}
	else if (indexSize == 2) {
		indexGLType = GL_UNSIGNED_SHORT;
		indexMaxValue = 0xffff;
	}
	else if (indexSize == 4) {
		indexGLType = GL_UNSIGNED_INT;
		indexMaxValue = 0xffffffff;
	}
}

void Mesh::setVertexFieldEnabled(VertexField f) {
	DEBUG_ASSERT(not editing, "setVertexFieldEnabled must be called BEFORE begin!");

	vertexFieldOffset[enum_cast(f)] = vertexSize;
	vertexSize += VERTEX_FIELD_INFO[enum_cast(f)].bytes;
}

void Mesh::setVertexFields(const std::initializer_list<VertexField>& fs) {
	for (auto&& f : fs) {
		setVertexFieldEnabled(f);
	}
}


void Mesh::setDynamic(bool d) {
	dynamic = d;
}

void Mesh::index(IndexType idx) {
	DEBUG_ASSERT(isEditing(), "index: this Mesh is not in Edit mode");
	DEBUG_ASSERT(idx <= indexMaxValue, "index: the index passed is too big to be contained in this mesh's index format, see setIndexByteSize");

	auto curSize = indices.size();
	indices.resize(curSize + indexSize);

	switch (indexSize) {
	case 1:
		*((uint8_t*)(indices.data() + curSize)) = (uint8_t)idx;
		break;

	case 2:
		*((uint16_t*)(indices.data() + curSize)) = (uint16_t)idx;
		break;

	case 4:
		*((uint32_t*)(indices.data() + curSize)) = (uint32_t)idx;
		break;
	}

	++indexCount;
}


void Mesh::_prepareVertex(const Vector& v) {
	DEBUG_ASSERT(isEditing(), "_prepareVertex: this Mesh is not in Edit mode");

	//grow the buffer to the needed size
	auto curSize = vertices.size();
	vertices.resize(curSize + vertexSize);

	currentVertex = (uint8_t*)vertices.data() + curSize;

	bounds = bounds.expandToFit(v);

	++vertexCount;
}

Mesh::IndexType Mesh::vertex(const Vector& v) {
	_prepareVertex(v);

	if (isVertexFieldEnabled(VertexField::Position3D)) {
		_field<glm::vec3>(VertexField::Position3D) = v;
	}
	else {
		_field<glm::vec2>(VertexField::Position2D) = { v.x, v.y };
	}

	return getVertexCount() - 1;
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
	case PrimitiveMode::TriangleList:
		return elemCount / 3;

	case PrimitiveMode::TriangleStrip:
		return elemCount - 2;

	case PrimitiveMode::LineStrip:
		return elemCount - 1;

	case PrimitiveMode::LineList:
		return elemCount / 2;

	case PrimitiveMode::PointList:
		return elemCount;

	default:
		FAIL("Invalid triangle mode");
	}
}

void Mesh::uv(float u, float v, uint8_t set /*= 0 */) {
	DEBUG_ASSERT(isEditing(), "uv: this Mesh is not in Edit mode");

	_field<GLuint>(VertexField::UV0, set) = glm::packHalf2x16({ u,v });
}

void Mesh::uv(const Vector& uv, uint8_t set /* = 0 */) {
	self.uv(uv.x, uv.y, set);
}

void Mesh::color(const Color& c) {
	DEBUG_ASSERT(isEditing(), "color: this Mesh is not in Edit mode");

	vertexTransparency |= c.a < 1.f;
	_field<GLuint>(VertexField::Color) = c.toRGBA();
}

void Mesh::normal(const Vector& n) {
	DEBUG_ASSERT(std::abs(n.x) <= 1.f and std::abs(n.y) <= 1.f and std::abs(n.z) <= 1.f, "normal is too long, cannot pack");
	DEBUG_ASSERT(isEditing(), "normal: this Mesh is not in Edit mode");

	auto& val = _field<GLuint>(VertexField::Normal);
	val = 0;
	val |= (Math::packNormalized<int>(n.z, 511) << 20);
	val |= (Math::packNormalized<int>(n.y, 511) << 10);
	val |= (Math::packNormalized<int>(n.x, 511) << 0);
}

void Mesh::bindVertexFormat(const Shader& shader) {
	for (auto&& attribute : shader.getAttributes()) {
		DEBUG_ASSERT(isVertexFieldEnabled(attribute.builtInAttribute), "This mesh doesn't provide a required attribute");

		auto offset = (void*)vertexFieldOffset[enum_cast(attribute.builtInAttribute)];
		auto& field = VERTEX_FIELD_INFO[enum_cast(attribute.builtInAttribute)];

		glEnableVertexAttribArray(attribute.location);
		glVertexAttribPointer(
			attribute.location,
			field.components,
			field.type,
			field.normalized,
			vertexSize,
			offset);
	}
}

bool Mesh::end() {
	DEBUG_ASSERT(editing, "Can't call end() before begin()!");
	editing = false;

	DEBUG_ASSERT(not isLoaded() or dynamic, "Can't update a static mesh");

	//don't load empty meshes
	if (getVertexCount() == 0) {
		return false;
	}

	//create the VBO
	if (not vertexHandle) {
		glGenBuffers(1, &vertexHandle);
	}

	uint32_t usage = (dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
	glBindBuffer(GL_ARRAY_BUFFER, vertexHandle);
	glBufferData(GL_ARRAY_BUFFER, vertices.size(), vertices.data(), usage);


	//create the IBO
	if (isIndexed()) { //we support unindexed meshes
		if (not indexHandle) {
			glGenBuffers(1, &indexHandle);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexHandle);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size(), indices.data(), usage);

	}

	loaded = true;

	currentVertex = nullptr;

	//geometric hints
	center = bounds.getCenter();
	dimensions = bounds.getSize();

	if (not dynamic) { //won't be updated ever again
		destroyBuffers();
	}

	gBufferBindingsDirty = true;
	return loaded;
}

void Mesh::bind() {
	glBindBuffer(GL_ARRAY_BUFFER, vertexHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, isIndexed() ? indexHandle : 0); //only bind the index buffer if existing (duh)

	gBufferBindingsDirty = false;
}

bool Mesh::onLoad() {
	DEBUG_ASSERT(not isLoaded(), "onLoad: Mesh is already loaded");

	if (not isReloadable()) {
		return false;
	}

	//load binary mesh
	auto buf = Platform::singleton().loadFileContent(filePath);

	DEBUG_ASSERT_INFO(buf.size() > 0, "onLoad: cannot find or read file", "path = " + filePath);

	uint8_t* ptr = buf.data();

	//index size
	setIndexByteSize(*ptr++);

	//triangle mode
	setTriangleMode((PrimitiveMode)*ptr++);

	//fields
	for (int i = 0; i < (int)VertexField::_Count; ++i) {
		if (*ptr++) {
			setVertexFieldEnabled((VertexField)i);
		}
	}

	//max and min
	Vector loadedMax;
	memcpy(&loadedMax, ptr, sizeof(Vector));
	ptr += sizeof(Vector);

	Vector loadedMin;
	memcpy(&loadedMin, ptr, sizeof(Vector));
	ptr += sizeof(Vector);

	//vertex count
	IndexType vc = *((IndexType*)ptr);
	ptr += sizeof(IndexType);

	//index count
	uint32_t ic = *((uint32_t*)ptr);
	ptr += sizeof(uint32_t);

	setDynamic(false);

	begin(vc);

	//grab vertex data
	vertices.resize(vc * vertexSize);
	memcpy((char*)vertices.data(), ptr, vc * vertexSize);
	ptr += vc * vertexSize;

	//grab index data
	if (ic) {
		indices.resize(ic * indexSize);
		memcpy((char*)indices.data(), ptr, ic * indexSize);
	}

	bounds.max = loadedMax;
	bounds.min = loadedMin;

	vertexCount = vc;
	indexCount = ic;

	//push over to GPU
	return end();
}

void Mesh::onUnload(bool soft /*= false */) {
	DEBUG_ASSERT(isLoaded(), "onUnload: Mesh is not loaded");

	//when soft unloading, only unload file-based meshes
	if (not soft or isReloadable()) {
		glDeleteBuffers(1, &vertexHandle);
		glDeleteBuffers(1, &indexHandle);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		vertexHandle = indexHandle = 0;

		destroyBuffers(); //free CPU side memory

		gBufferBindingsDirty = true;
		loaded = false;
	}
}

Vector& Mesh::getVertex(int idx) {
	auto field = isVertexFieldEnabled(VertexField::Position3D) ? VertexField::Position3D : VertexField::Position3D;
	auto offset = vertexFieldOffset[enum_cast(field)];
	uint8_t* ptr = (uint8_t*)vertices.data() + (idx * vertexSize) + offset;

	return *(Vector*)ptr;
}

void Mesh::setIndex(int idxidx, IndexType idx) {
	DEBUG_ASSERT(idxidx >= 0 and idxidx < getIndexCount(), "Index out of bounds");

	switch (indexSize) {
	case 1:
		((uint8_t*)indices.data())[idxidx] = (uint8_t)idx;
		return;

	case 2:
		((uint16_t*)indices.data())[idxidx] = (uint16_t)idx;
		return;

	default:
		((uint32_t*)indices.data())[idxidx] = (uint32_t)idx;
		return;
	}
}


Mesh::IndexType Mesh::getIndex(int idxidx) const {
	DEBUG_ASSERT(idxidx >= 0 and idxidx < getIndexCount(), "Index out of bounds");

	switch (indexSize) {
	case 1:
		return indices[idxidx];

	case 2:
		return ((uint16_t*)indices.data())[idxidx];

	default:
		return ((uint32_t*)indices.data())[idxidx];
	}
}

void Mesh::eraseIndex(int idxidx) {
	DEBUG_ASSERT(idxidx >= 0 and idxidx < getIndexCount(), "Index out of bounds");

	auto i = indices.begin() + (idxidx * indexSize);
	indices.erase(i, i + indexSize);
	--indexCount;
}


void Mesh::cutSection(IndexType i1, IndexType i2) {
	DEBUG_ASSERT(isEditing(), "cutSection: this Mesh is not in Edit mode");
	DEBUG_ASSERT(i1 < getVertexCount() and i2 <= getVertexCount() and i1 <= i2, "Invalid indices passed");

	//easy part: cut out the vertex data
	auto diff = i2 - i1;
	auto size = diff * vertexSize;
	auto start = vertices.begin() + i1 * vertexSize;
	vertices.erase(start, start + size);

	//remove the indices
	if (isIndexed()) {

		for (int i = 0; i < getIndexCount(); ++i) {
			auto idx = getIndex(i);

			if (idx >= i1 and idx < i2) {
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
	c->vertexFieldOffset = vertexFieldOffset;

	return c;
}

Unique<Mesh> Mesh::cloneFromSlice(IndexType vertexStart, IndexType vertexEnd, const Vector& translation /*= Vector::ZERO*/) const {
	DEBUG_ASSERT(not vertices.empty(), "This mesh is empty");
	DEBUG_ASSERT(vertexStart < getVertexCount() and vertexEnd <= getVertexCount() and vertexStart <= vertexEnd, "Indices out of bounds");

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

			c->bounds.max = Vector::max(c->bounds.max, v);
			c->bounds.min = Vector::min(c->bounds.min, v);
		}
	}

	//find the indices that were pointing to these vertices
	//and place new indices that replicate the same structure
	if (isIndexed()) {
		for (int i = 0; i < getIndexCount(); ++i) {
			auto idx = getIndex(i);

			if (idx >= vertexStart and idx < vertexEnd) {
				c->index(idx - vertexStart);
			}
		}
	}


	return c;
}

bool Mesh::supportsShader(const Shader& shader) const {
	for (auto&& attribute : shader.getAttributes()) {
		if (not isVertexFieldEnabled(attribute.builtInAttribute))
			return false;
	}
	return true;
}
