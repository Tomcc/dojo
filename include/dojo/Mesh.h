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
#include "TriangleMode.h"

namespace Dojo
{
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
	class Mesh : public Resource
	{
	public:
		typedef unsigned int IndexType;

		static const int VERTEX_PAGE_SIZE = 256;
		static const int INDEX_PAGE_SIZE = 256;

		static const int VERTEX_FIELD_SIZES[];

		///Creates a new empty Mesh
		Mesh( ResourceGroup* creator = NULL );

		///Creates a new Mesh bound to the file at filePath
		Mesh( ResourceGroup* creator, const String& filePath );

		virtual ~Mesh();

		///frees all CPU-side memory (done automatically on static meshes)
		void destroyBuffers();

		///sets the dimension of a single index in this mesh
		/**
			MUST be called only before the first begin ever!
		*/
		void setIndexByteSize( byte bytenumber );

		///enables a new VertexField
		void setVertexFieldEnabled( VertexField f );

		///enables a list of VertexFields
		void setVertexFields(const std::initializer_list<VertexField>& fs);

		///A dynamic mesh set as dynamic won't clear its CPU cache when loaded, allowing for quick editing
		void setDynamic( bool d);

		///Sets the primitive for the rendering of this mesh
		void setTriangleMode( TriangleMode m )	{	triangleMode = m;	}

		TriangleMode getTriangleMode()	{	return triangleMode;	}

		const Vector& getMax()
		{
			return max;
		}

		const Vector& getMin()
		{
			return min;
		}

		const Vector& getDimensions()
		{
			return dimensions;
		}

		const Vector& getCenter()
		{
			return center;
		}

		///tells if begin() has been called not followed by an end()
		bool isEditing()
		{
			return editing;
		}

		///begins editing this mesh. Vertex Fields have to be set at this point
		/**
		\param extimatedVertes number of vertices that have to be reserved
		*/
		void begin( int extimatedVerts = 1 );

		///starts editing a dynamic mesh that was already begin'd and end'ed
		/**
			pointers are placed at the bottom
		*/
		void beginAppend();

		///adds a vertex at the given position
		void vertex( float x, float y );

		///adds a vertex at the given position
		void vertex( float x, float y, float z );

		void vertex(const Vector& v);

		///sets the uv of the given UV set
		void uv( float u, float v, byte set = 0 );

		///sets the color of the current vertex
		void color( float r, float g, float b, float a  );

		void color( const Color& c );

		///adds a vertex at the given position
		void normal( float x, float y, float z );

		void normal( const Vector& n );

		///appends a raw blob of vertices to the vertex array
		void appendRawVertexData(void* data, IndexType vertexCount);

		///adds one index
		void index(IndexType idx);

		///adds 3 clockwise indices to make a triangle
		void triangle(unsigned int i1, unsigned int i2, unsigned int i3)
		{
			index(i1);
			index(i2);
			index(i3);
		}

		///adds 2 clockwise triangles (6 indices) to make a quad
		void quad(unsigned int i11, unsigned int i12, unsigned int i21, unsigned int i22)
		{
			triangle(i11,i21,i12);
			triangle(i21,i22,i12);
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
		virtual bool onLoad();

		virtual void onUnload( bool soft = false );

		///binds the mesh buffers with the vertex format from the specified shader
		virtual void bind( Shader* shader );

		bool isIndexed() const
		{
			return !indices.empty() || indexHandle;
		}

		GLenum getIndexGLType()
		{
			return indexGLType;
		}

		bool isVertexFieldEnabled( VertexField f )
		{
			return vertexFieldOffset[(unsigned char)f] != 0xff;
		}

		IndexType getVertexCount() const
		{
			return vertexCount;
		}

		int getIndexCount() const
		{
			return indexCount;
		}

		///returns the total triangle count in this mesh
		int getPrimitiveCount() const;

		Vector& getVertex(int idx);

		IndexType getIndex(int idxidx) const;

		void eraseIndex(int idxidx);

		void setIndex(int idxidx, IndexType idx);

		///Creates a new empty mesh with the same format of this one
		std::unique_ptr<Mesh> cloneWithSameFormat() const;

		///creates a new mesh from a slice of this one
		std::unique_ptr<Mesh> cloneFromSlice(IndexType vertexStart, IndexType vertexEnd, const Vector& offset = Vector::ZERO) const;

	protected:
		Vector max, min, center, dimensions;

		int vertexSize = 0;
		byte* currentVertex = nullptr;
		std::vector<byte> vertices;

		byte indexSize = 0;
		IndexType indexMaxValue = 0;
		GLenum indexGLType = 0;
		std::vector<byte> indices;//indices have varying size

		GLuint vertexArrayDesc = 0;
		GLuint vertexHandle = 0, indexHandle = 0;

		int vertexCount = 0, indexCount = 0;

		byte vertexFieldOffset[ (int)VertexField::_Count ];

		TriangleMode triangleMode = TriangleMode::TriangleStrip;

		bool dynamic = false;
		bool editing = false;

		void _prepareVertex(const Vector& v);

		///returns low level binding informations about a vertex field
		void _getVertexFieldData( VertexField field, int& outComponents, GLenum& outComponentsType, bool& outNormalized, void*& outOffset );

		///binds the attribute arrays and the Buffer Objects required to render the mesh
		void _bindAttribArrays( Shader* shader );

		byte& _offset(VertexField f);
		byte& _offset(VertexField f, int subID);
	};
}

