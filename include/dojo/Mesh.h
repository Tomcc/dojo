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

#define MESH_MAX_TEXTURES 8

namespace Dojo 
{
	class Mesh : public Resource 
	{
	public:
		
		enum VertexField
		{
			VF_POSITION2D = 0,
			VF_POSITION3D = 1,
			VF_UV = 2,
			VF_UV_1 = 3,
			VF_UV_2 = 4,
			VF_UV_3 = 5,
			VF_UV_4 = 6,
			VF_UV_5 = 7,
			VF_UV_6 = 8,
			VF_UV_7 = 9,
			VF_COLOR = 10,
			VF_NORMAL = 11
		};
				
		static const uint FIELDS_NUMBER = VF_NORMAL+1;	
		static const uint VERTEX_PAGE_SIZE = 256;
		static const uint INDEX_PAGE_SIZE = 256;
		
		static const uint VERTEX_FIELD_SIZES[];
		
		enum TriangleMode
		{
			TM_STRIP,
			TM_LIST,
			TM_LINE_STRIP
		};
		
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
			mDestroyBuffersOnEnd( true )
		{
			//set all fields to zero
			memset( vertexFields, 0, sizeof( bool ) * FIELDS_NUMBER );
			memset( vertexFieldOffset, 0, sizeof(uint) * FIELDS_NUMBER );

			//default index size is 16
			setIndexByteSize( sizeof(GLushort) );
		}

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
			memset( vertexFields, 0, sizeof( bool ) * FIELDS_NUMBER );
			memset( vertexFieldOffset, 0, sizeof(uint) * FIELDS_NUMBER );

			//default index size is 16
			setIndexByteSize( sizeof(GLushort) );
		}
		
		virtual ~Mesh()
		{

		}

		///sets the dimension of a single index in this mesh
		/**
			MUST be called only before the first begin ever!
		*/
		inline void setIndexByteSize( byte bytenumber )
		{
			DEBUG_ASSERT( !indices );
			DEBUG_ASSERT( 
				bytenumber == 1 || 
				bytenumber == 2 || 
				bytenumber == 4 );

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
#ifndef PLATFORM_IOS
				indexGLType = GL_UNSIGNED_INT;
				indexMaxValue = 0xffffffff;
#else
				DEBUG_ASSERT( !"32 BIT INDICES NOT SUPPORTED ON OES" );
#endif
			}
		}		
		
		void setVertexFieldEnabled( VertexField f )
		{			
			if( !vertices ) //if the memory has not been created
			{				
				vertexFields[f] = true;
				
				vertexSize += VERTEX_FIELD_SIZES[ f ];
			}
		}
		
		///Tells the buffer to allocate at least "vertices" vertices
		void setVertexCap( uint count );

		void setIndexCap( uint count );
		
		///A dynamic mesh won't clear its CPU cache when loaded, and allows to call load() more than one time!
		inline void setDynamic( bool d, bool destroyBuffersOnEnd = true )
		{
			dynamic = d;
			mDestroyBuffersOnEnd = destroyBuffersOnEnd;
		}		
		
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
					
		inline void begin( uint extimatedVerts = 1 )
		{			
			//be sure that we aren't already building
			DEBUG_ASSERT( extimatedVerts > 0 );
			DEBUG_ASSERT( !isEditing() );

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
			DEBUG_ASSERT( !isEditing() );

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
			DEBUG_ASSERT( isEditing() );
			
			float* ptr = (float*)( currentVertex + vertexFieldOffset[ VF_UV + set ] );
			ptr[0] = u;
			ptr[1] = v;
		}

		inline void setAllUVs( float u, float v ) 
		{
			for( int i = 0; i < MESH_MAX_TEXTURES; ++i )
			{
				if( isVertexFieldEnabled( (VertexField)(VF_UV + i) ) )
					uv( u,v,i );
			}
		}
		
		//sets the color of the last set vertex		
		inline void color( float r, float g, float b, float a  )
		{		
			DEBUG_ASSERT( isEditing() );	
			
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
			DEBUG_ASSERT( isEditing() );
			
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
			DEBUG_ASSERT( isEditing() );
			DEBUG_ASSERT( idx <= indexMaxValue );
			
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
		
		inline void triangle( uint i1, uint i2, uint i3 )
		{
			index(i1);
			index(i2);
			index(i3);
		}

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
			DEBUG_ASSERT( isLoaded() );

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
				
		///binds all the pointers for the needed client states
		virtual void bind();
				
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
		
		inline uint getVertexCount()
		{
			return vertexCount;
		}
		
		inline uint getIndexCount()
		{
			return indexCount;
		}

		inline uint getTriangleCount()
		{
			return triangleCount;
		}
				
		inline float* _getVertex( uint i )
		{			
			DEBUG_ASSERT( i < vertexCount );
			
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

		uint triangleCount;
		
		bool vertexFields[ FIELDS_NUMBER ];		
		uint vertexFieldOffset[ FIELDS_NUMBER ];
		
		TriangleMode triangleMode;
		
		uint vertexHandle, indexHandle;
		
		bool dynamic;
		bool mDestroyBuffersOnEnd;
		bool editing;
				
		void _buildFieldOffsets()
		{
			uint offset = 0;
			for( uint i = 0; i < FIELDS_NUMBER; ++i )
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
	};
}

#endif
