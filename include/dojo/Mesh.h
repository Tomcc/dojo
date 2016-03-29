/*
 *  Mesh.h
 *  Ninja Training
 *
 *  Created by Tommaso Checchi on 8/10/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */

#pragma once

#include "dojo_common_header.h"

#include "Resource.h"
#include "Vector.h"
#include "VertexField.h"
#include "PrimitiveMode.h"
#include "AABB.h"

namespace Dojo {
	class Color;
	class ResourceGroup;
	class Shader;

	///A Mesh is the only primitive Dojo can render.
	/**
	But, it is generic enough to allow for fast rendering using GPU-side buffers.

	A Mesh has a Vertex Format defined by the VertexFields enabled on it; only one between VF_POSITION2D and VF_POSITION3D must be choosen,
	but other fields can be enabled when needed.

	After a Vertex Format has been defined, a Mesh can be procedurally generated by calling the vertex() method which adds a new vertex,
	and specifying vertex features using color(), normal() and uv() methods.

	Calling end() is required before the mesh can be used, so that its data is loaded to the GPU.
	*/
	class Mesh : public Resource {
	public:
		static bool gBufferBindingsDirty;
		typedef unsigned int IndexType;

		static const int VERTEX_PAGE_SIZE = 256;
		static const int INDEX_PAGE_SIZE = 256;

		///Creates a new empty Mesh
		explicit Mesh(optional_ref<ResourceGroup> creator = {});

		///Creates a new Mesh bound to the file at filePath
		Mesh(optional_ref<ResourceGroup> creator, const utf::string& filePath);

		virtual ~Mesh();

		///frees all CPU-side memory (done automatically on static meshes)
		void destroyBuffers();

		///sets the dimension of a single index in this mesh
		/**
			MUST be called only before the first begin ever!
		*/
		void setIndexByteSize(byte bytenumber);

		///enables a new VertexField
		void setVertexFieldEnabled(VertexField f);

		///enables a list of VertexFields
		void setVertexFields(const std::initializer_list<VertexField>& fs);

		///A dynamic mesh set as dynamic won't clear its CPU cache when loaded, allowing for quick editing
		void setDynamic(bool d);

		///Sets the primitive for the rendering of this mesh
		void setTriangleMode(PrimitiveMode m) {
			triangleMode = m;
		}

		PrimitiveMode getTriangleMode() {
			return triangleMode;
		}

		const AABB& getBounds() const {
			return bounds;
		}

		const Vector& getDimensions() const {
			return dimensions;
		}

		const Vector& getCenter() const {
			return center;
		}

		bool hasVertexTransparency() const {
			return vertexTransparency;
		}

		///tells if begin() has been called not followed by an end()
		bool isEditing() const {
			return editing;
		}

		///begins editing this mesh. Vertex Fields have to be set at this point
		/**
		\param extimatedVertes number of vertices that have to be reserved
		*/
		void begin(IndexType extimatedVerts = 1);

		///starts editing a dynamic mesh that was already begin'd and end'ed
		/**
			pointers are placed at the bottom
		*/
		void beginAppend();

		///adds a vertex at the given position
		IndexType vertex(const Vector& v);

		///sets the uv of the given UV set
		void uv(float u, float v, byte set = 0);

		///sets the uv of the given UV set
		void uv(const Vector& uv, byte set = 0);

		///sets the color of the current vertex
		void color(const Color& c);

		///adds a vertex at the given position
		void normal(const Vector& n);

		///appends a raw blob of vertices to the vertex array
		void appendRawVertexData(void* data, IndexType vertexCount);

		///adds one index
		void index(IndexType idx);

		///adds 3 clockwise indices to make a triangle
		void triangle(IndexType i1, IndexType i2, IndexType i3) {
			index(i1);
			index(i2);
			index(i3);
		}

		///adds 2 clockwise triangles (6 indices) to make a quad
		void quad(IndexType i11, IndexType i12, IndexType i21, IndexType i22) {
			triangle(i11, i21, i12);
			triangle(i21, i22, i12);
		}

		///loads the data on the device
		/**
		-will discard all the data if the buffer is static
		-if the buffer is dynamic, this can be called again to update device data
		*/
		bool end();

		//Removes the given vertices from the mesh
		void cutSection(IndexType i1, IndexType i2);

		///loads the whole file passed in the constructor
		virtual bool onLoad() override;

		virtual void onUnload(bool soft = false) override;

		///binds the mesh buffers with the vertex format from the specified shader
		virtual void bind();

		///binds the attribute arrays and the Buffer Objects required to render the mesh
		void bindVertexFormat(const Shader& shader);


		bool isIndexed() const {
			return !indices.empty() || indexHandle;
		}

		uint32_t getIndexGLType() const {
			return indexGLType;
		}

		bool isVertexFieldEnabled(VertexField f) const {
			return vertexFieldOffset[(unsigned char)f] != 0xff;
		}

		IndexType getVertexCount() const {
			return vertexCount;
		}

		int getIndexCount() const {
			return indexCount;
		}

		///returns the total triangle count in this mesh
		int getPrimitiveCount() const;

		Vector& getVertex(int idx);

		IndexType getIndex(int idxidx) const;

		void eraseIndex(int idxidx);

		void setIndex(int idxidx, IndexType idx);

		bool supportsShader(const Shader& shader) const;

		///Creates a new empty mesh with the same format of this one
		Unique<Mesh> cloneWithSameFormat() const;

		///creates a new mesh from a slice of this one
		Unique<Mesh> cloneFromSlice(IndexType vertexStart, IndexType vertexEnd, const Vector& offset = Vector::Zero) const;

	protected:
		Vector center, dimensions;
		AABB bounds;

		byte vertexSize = 0;
		byte* currentVertex = nullptr;
		std::vector<byte> vertices;

		byte indexSize = 0;
		IndexType indexMaxValue = 0;
		uint32_t indexGLType = 0;
		std::vector<byte> indices;//indices have varying size

		uint32_t vertexArrayDesc = 0;
		uint32_t vertexHandle = 0, indexHandle = 0;

		int vertexCount = 0, indexCount = 0;

		byte vertexFieldOffset[ (int)VertexField::_Count ];

		PrimitiveMode triangleMode = PrimitiveMode::TriangleStrip;

		bool dynamic = false;
		bool editing = false;
		bool vertexTransparency = false;

		void _prepareVertex(const Vector& v);

		///returns low level binding informations about a vertex field
		void _getVertexFieldData(VertexField field, int& outComponents, uint32_t& outComponentsType, bool& outNormalized, void*& outOffset);

		byte& _offset(VertexField f);
		byte& _offset(VertexField f, int subID);
	};
}
