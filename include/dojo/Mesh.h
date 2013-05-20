/*
 *  Mesh.h
 *  Ninja Training
 *
 *  Created by Tommaso Checchi on 8/10/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */

#ifndef Mesh_h__
#define Mesh_h__

#include "dojo_common_header.h"

#include "Resource.h"
#include "Vector.h"
#include "Color.h"

namespace Dojo 
{
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
		
		enum VertexField
		{
			VF_POSITION2D,
			VF_POSITION3D,
			VF_COLOR,
			VF_NORMAL,
			
			VF_UV_0,
			VF_UV_MAX = VF_UV_0 + DOJO_MAX_TEXTURE_COORDS-1,

			VF_NONE,
			_VF_COUNT = VF_NONE
		};
		
		static const uint VERTEX_PAGE_SIZE = 256;
		static const uint INDEX_PAGE_SIZE = 256;
		
		static const uint VERTEX_FIELD_SIZES[];
		
		enum TriangleMode
		{
			TM_STRIP,
			TM_LIST,
			TM_LINE_STRIP,
			TM_LINE_LIST
		};
		
		///Creates a new empty Mesh
		Mesh( ResourceGroup* creator = NULL ) :
		Resource( creator ),
			vertexSize(0),
			vertexCount( 0 ),		
			vertexMaxCount( 0 ),
			currentVertex( NULL ),
			vertices( NULL ),
			indexCount( 0 ),
			indexMaxCount( 0 ),
			indexByteSize(0),
			indexMaxValue(0),
			indexGLType(0),
			indices( NULL ),
			triangleCount(0),
			triangleMode( TM_STRIP ),
			vertexHandle(0),
			indexHandle(0),
			dynamic( false ),
			editing( false ),
			mDestroyBuffersOnEnd( true ),
			vertexArrayDesc( 0 )
		{
			//set all fields to zero
			memset( vertexFields, 0, sizeof( vertexFields ) );
			memset( vertexFieldOffset, 0, sizeof( vertexFieldOffset ) );

			//default index size is 16
			setIndexByteSize( sizeof(GLushort) );
		}

		///Creates a new Mesh bound to the file at filePath
		Mesh( ResourceGroup* creator, const String& filePath ) :
		Resource( creator, filePath ),
		vertexSize(0),
		vertexCount( 0 ),		
		vertexMaxCount( 0 ),
		currentVertex( NULL ),
		vertices( NULL ),
		indexCount( 0 ),
		indexMaxCount( 0 ),
		indexByteSize(0),
		indexMaxValue(0),
		indexGLType(0),
		indices( NULL ),
		triangleCount(0),
		triangleMode( TM_STRIP ),
		vertexHandle(0),
		indexHandle(0),
		dynamic( false ),
		editing( false ),
		mDestroyBuffersOnEnd( true )
		{
			//set all fields to zero
			memset( vertexFields, 0, sizeof( vertexFields ) );
			memset( vertexFieldOffset, 0, sizeof( vertexFieldOffset ) );

			//default index size is 16
			setIndexByteSize( sizeof(GLushort) );
		}
		
		virtual ~Mesh()
		{
		#ifndef DOJO_DISABLE_VAOS
			if( vertexArrayDesc )
				glDeleteVertexArrays( 1, &vertexArrayDesc );
		#endif
		}

		///sets the dimension of a single index in this mesh
		/**
			MUST be called only before the first begin ever!
		*/
		inline void setIndexByteSize( byte bytenumber )
		{
			DEBUG_ASSERT( !indices, "setIndexByteSize must be called BEFORE begin!" );
			DEBUG_ASSERT( 
				bytenumber == 1 || 
				bytenumber == 2 || 
				bytenumber == 4, "setIndexByteSize: byteNumber must be either 1,2 or 4" );

			indexByteSize = bytenumber;

			if( indexByteSize == 1 )
			{
				indexGLType = GL_UNSIGNED_BYTE;
				indexMaxValue = 0xff;
			}
			else if( indexByteSize == 2 )
			{
				indexGLType = GL_UNSIGNED_SHORT;
				indexMaxValue = 0xffff;
			}
			else if( indexByteSize == 4 )
			{
#ifdef DOJO_32BIT_INDICES_AVAILABLE
				indexGLType = GL_UNSIGNED_INT;
				indexMaxValue = 0xffffffff;
#else
				DEBUG_FAIL( "32 bit indices are disabled (force enabled defining DOJO_32BIT_INDICES_AVAILABLE)" );
#endif
			}
		}		
		
		///enables a new VertexField
		void setVertexFieldEnabled( VertexField f )
		{
			DEBUG_ASSERT( !vertices, "setVertexFieldEnabled must be called BEFORE begin!" );

			vertexFields[f] = true;
			vertexSize += VERTEX_FIELD_SIZES[ f ];
		}
		
		///reserves count elements on the vertex buffer, to avoid costly reallocations
		/** 
		only effective after the right vertex fields have been enabled! */
		void setVertexCap( uint count );

		///reserves count indices on the index buffer, to avoid costly reallocations
		void setIndexCap( uint count );
		
		///A dynamic mesh set as dynamic won't clear its CPU cache when loaded, allowing for quick editing
		inline void setDynamic( bool d, bool destroyBuffersOnEnd = true )
		{
			dynamic = d;
			mDestroyBuffersOnEnd = destroyBuffersOnEnd;
		}		
		
		///Sets the primitive for the rendering of this mesh
		inline void setTriangleMode( TriangleMode m )	{	triangleMode = m;	}
		
		inline TriangleMode getTriangleMode()	{	return triangleMode;	}

		inline const Vector& getMax()
		{
			return max;
		}

		inline const Vector& getMin()
		{
			return min;
		}

		inline const Vector& getDimensions()
		{
			return dimensions;
		}

		inline const Vector& getCenter()
		{
			return center;
		}
		
		///tells if begin() has been called not followed by an end()
		inline bool isEditing()
		{
			return editing;
		}

		///begins editing this mesh. Vertex Fields have to be set at this point
		/**
		\param extimatedVertes number of vertices that have to be reserved
		*/
		inline void begin( uint extimatedVerts = 1 )
		{			
			//be sure that we aren't already building
			DEBUG_ASSERT( extimatedVerts > 0, "begin: extimated vertices for this batch must be more than 0" );
			DEBUG_ASSERT( !isEditing(), "begin: this Mesh is already in Edit mode" );

			//the buffer is too small for extimated vertex count?
			setVertexCap( extimatedVerts );
			
			currentVertex = NULL;
			vertexCount = 0;
			indexCount = 0;

			max.x = max.y = max.z = -FLT_MAX;
			min.x = min.y = min.z = FLT_MAX;
			
			editing = true;			
		}

		///starts the update after the last added vertex - useful for sequential updates
		inline void append()
		{
			DEBUG_ASSERT( !isEditing(), "append: this Mesh is already in Edit mode" );

			currentVertex = vertices + vertexSize * (vertexCount-1);

			editing = true;
		}
					
		///adds a vertex at the given position
		void vertex( float x, float y );
		
		///adds a vertex at the given position
		void vertex( float x, float y, float z );

		inline void vertex( const Vector& v )
		{
			vertex( v.x, v.y, v.z );
		}
				
		///sets the uv of the given UV set				
		inline void uv( float u, float v, byte set = 0 )
		{			
			DEBUG_ASSERT( isEditing(), "uv: this Mesh is not in Edit mode" );
			
			float* ptr = (float*)( currentVertex + vertexFieldOffset[ VF_UV_0 + set ] );
			ptr[0] = u;
			ptr[1] = v;
		}

		///sets the color of the current vertex		
		inline void color( float r, float g, float b, float a  )
		{		
			DEBUG_ASSERT( isEditing(), "color: this Mesh is not in Edit mode" );
			
			GLubyte* ptr = (GLubyte*)( currentVertex + vertexFieldOffset[ VF_COLOR ] );
			ptr[0] = (GLubyte)(r*255);
			ptr[1] = (GLubyte)(g*255);
			ptr[2] = (GLubyte)(b*255);
			ptr[3] = (GLubyte)(a*255);
		}

		inline void color( const Color& c )
		{
			color( c.r, c.g, c.b, c.a );
		}
		
		///adds a vertex at the given position
		inline void normal( float x, float y, float z )
		{		
			DEBUG_ASSERT( isEditing(), "normal: this Mesh is not in Edit mode" );
			
			float* ptr = (float*)( currentVertex + vertexFieldOffset[ VF_NORMAL ] );
			
			ptr[0] = x;
			ptr[1] = y;
			ptr[2] = z;
		}
		
		inline void normal( const Vector& n )
		{
			normal( n.x, n.y, n.z );
		}

		///adds one index
		inline void index( uint idx )
		{		
			DEBUG_ASSERT( isEditing(), "index: this Mesh is not in Edit mode" );
			DEBUG_ASSERT( idx <= indexMaxValue, "index: the index passed is too big to be contained in this mesh's index format, see setIndexByteSize" );
			
			if( indexCount >= indexMaxCount )
				setIndexCap( indexCount + 1 );

			switch( indexByteSize )
			{
			case 1:
				indices[ indexCount ] = idx;
				break;
			case 2:
				((unsigned short*)indices)[ indexCount ] = idx;
				break;
			case 4:
				((uint*)indices)[ indexCount ] = idx;
				break;
			}

			++indexCount;
		}
		
		///adds 3 clockwise indices to make a triangle
		inline void triangle( uint i1, uint i2, uint i3 )
		{
			index(i1);
			index(i2);
			index(i3);
		}

		///adds 2 clockwise triangles (6 indices) to make a quad
		inline void quad( uint i11, uint i12, uint i21, uint i22 )
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
		
		///loads the whole file passed in the constructor
		virtual bool onLoad();
		
		virtual void onUnload( bool soft = false )
		{
			DEBUG_ASSERT( isLoaded(), "onUnload: Mesh is not loaded" );

			//when soft unloading, only unload file-based meshes
			if( !soft || isReloadable() )
			{
				glDeleteBuffers( 1, &vertexHandle );
				glDeleteBuffers( 1, &indexHandle );

				glBindBuffer(GL_ARRAY_BUFFER, 0);

				vertexHandle = indexHandle = 0;

				_destroyBuffers(); //free CPU side memory

				loaded = false;
			}
		}
				
		///binds the mesh buffers with the vertex format from the specified shader
		virtual void bind( Shader* shader );
				
		inline bool isIndexed()
		{
			return indices || indexHandle;
		}

		inline GLenum getIndexGLType()
		{
			return indexGLType;
		}
		
		inline bool isVertexFieldEnabled( VertexField f )
		{
			return vertexFields[f];
		}
		
		inline int getVertexCount()
		{
			return vertexCount;
		}
		
		inline uint getIndexCount()
		{
			return indexCount;
		}

		///returns the total triangle count in this mesh
		inline uint getTriangleCount()
		{
			return triangleCount;
		}
				
		///returns a pointer to the memory of the vertex with index i
		inline float* _getVertex( int i )
		{			
			DEBUG_ASSERT( i >= 0, "i is negative" );
			DEBUG_ASSERT( i < vertexCount, "i is OOB" );
			
			return (float*)(vertices + vertexSize * i);
		}
				
	protected:

		Vector max, min, center, dimensions;
				
		uint vertexSize, vertexCount, vertexMaxCount;
		byte* currentVertex;
		byte* vertices;
		
		uint indexCount, indexMaxCount;
		byte indexByteSize;
		uint indexMaxValue;
		GLenum indexGLType;
		byte* indices;//indices have varying size

		GLuint vertexArrayDesc;

		uint triangleCount;
		
		bool vertexFields[ _VF_COUNT ];		
		uint vertexFieldOffset[ _VF_COUNT ];
		
		TriangleMode triangleMode;
		
		uint vertexHandle, indexHandle;
		
		bool dynamic;
		bool mDestroyBuffersOnEnd;
		bool editing;
				
		void _buildFieldOffsets()
		{
			uint offset = 0;
			for( uint i = 0; i < _VF_COUNT; ++i )
			{
				if( isVertexFieldEnabled( (VertexField)i ) )
				{
					vertexFieldOffset[ i ] = offset;
					offset += VERTEX_FIELD_SIZES[ i ];
				}
			}			
		}
				
		void _destroyBuffers()
		{
			if( vertices )
			{
				free( vertices );
				vertices = NULL;
				vertexMaxCount = 0;
			}
			
			if( indices )
			{
				free( indices );
				indices = NULL;
				indexMaxCount = 0;
			}
		}

		void _prepareVertex( float x, float y, float z );

		///returns low level binding informations about a vertex field
		inline void _getVertexFieldData( VertexField field, int& outComponents, GLenum& outComponentsType, bool& outNormalized, void*& outOffset )
		{
			outOffset = (void*)vertexFieldOffset[ field ];
			outNormalized = false;

			switch ( field )
			{
			case VF_POSITION2D: outComponentsType = GL_FLOAT; outComponents = 2; outNormalized = false; break;
			case VF_POSITION3D: outComponentsType = GL_FLOAT; outComponents = 3; outNormalized = false; break;
			case VF_COLOR: outComponentsType = GL_UNSIGNED_BYTE; outComponents = 4; outNormalized = true; break;
			case VF_NORMAL: outComponentsType = GL_FLOAT; outComponents = 3; outNormalized = false; break;

			default: //textures
				outNormalized = true;
				outComponentsType = GL_FLOAT;
				outComponents = 2;
				outNormalized = false;
				break;
			}
		}

		///binds the attribute arrays and the Buffer Objects required to render the mesh
		void _bindAttribArrays( Shader* shader );
	};
}

#endif
