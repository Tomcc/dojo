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

#include "dojo_config.h"

#include <OpenGLES/ES1/gl.h>

#include "Buffer.h"
#include "Vector.h"
#include "Color.h"

namespace Dojo 
{
	class Mesh : public Buffer 
	{
	public:
				
		static const uint FIELDS_NUMBER = 5;	
		static const uint VERTEX_PAGE_SIZE = 32;
		static const uint INDEX_PAGE_SIZE = 32;
		
		static const uint VERTEX_FIELD_SIZES[];
		
		enum VertexField
		{
			VF_POSITION2D,
			VF_POSITION3D,
			VF_UV,
			VF_COLOR,
			VF_NORMAL
		};
		
		enum TriangleMode
		{
			TM_STRIP,
			TM_LIST
		};
				
		Mesh( ResourceGroup* creator = NULL, const std::string& filePath = "" ) :
		Buffer( creator, filePath ),
		vertexSize(0),
		vertexCount( 0 ),		
		vertexMaxCount( 0 ),
		currentVertex( NULL ),
		vertices( NULL ),
		indexCount( 0 ),
		indexMaxCount( 0 ),
		indices( NULL ),
		dynamic( false ),
		editing( false ),
		triangleMode( TM_STRIP ),
		vertexHandle(0),
		indexHandle(0)
		{
			//set all fields to zero
			memset( vertexFields, 0, sizeof( bool ) * FIELDS_NUMBER );
			memset( vertexFieldOffset, 0, sizeof(uint) * FIELDS_NUMBER );
		}		
		
		virtual ~Mesh()
		{
			//destroy CPU mem
			_destroyBuffers();
			
			//and GPU mem
			unload();
		}
		
		void setVertexFieldEnabled( VertexField f, bool enable )
		{			
			if( !vertices ) //if the memory has not been created
			{				
				vertexFields[f] = enable;
				
				//adjust vertex size
				if( enable )
					vertexSize += VERTEX_FIELD_SIZES[ f ];
				else
					vertexSize -= VERTEX_FIELD_SIZES[ f ];
			}
		}
		
		///Tells the buffer to allocate at least "vertices" vertices
		inline void setVertexCap( uint count )
		{		
			if( count < vertexMaxCount ) //no need to grow the buffer
				return;
			
			vertexMaxCount = (count/VERTEX_PAGE_SIZE + 1 ) * VERTEX_PAGE_SIZE;
			
			if( !vertices )
			{					
				vertices = (byte*)malloc( vertexSize * vertexMaxCount );
				
				_buildFieldOffsets();	//build the offsets for each field		
			}			
			else if( count >= vertexMaxCount )
			{				
				vertices = (byte*)realloc( vertices, vertexSize * vertexMaxCount );
			}
		}	
		
		///A dynamic mesh won't clear its CPU cache when loaded, and allows to call load() more than one time!
		inline void setDynamic( bool d )
		{
			dynamic = d;
		}		
		
		inline void setTriangleMode( TriangleMode m )	{	triangleMode = m;	}
		
		inline TriangleMode getTriangleMode()	{	return triangleMode;	}
		
		///tells if begin() has been called not followed by an end()
		inline bool isEditing()
		{
			return editing;
		}
					
		inline void begin( int extimatedVerts = 0 )
		{			
			//be sure that we aren't already building
			DOJO_ASSERT( !isEditing() );
			
			currentVertex = NULL;
			vertexCount = 0;
			indexCount = 0;
			
			editing = true;
			
			//the buffer is too small for extimated vertex count?
			if( extimatedVerts > vertexMaxCount )
				setVertexCap( extimatedVerts );
		}
				
		///just skips the current vertex leaving it's position unchanged
		/**
		DO NOT CALL if the vertex has still to be created! Use only when updating.
		*/
		inline void vertex()
		{		
			DOJO_ASSERT( isEditing() );
			
			DOJO_ASSERT( currentVertex < vertices + vertexMaxCount );
			
			if( !currentVertex )
				currentVertex = vertices;
			else
				currentVertex += vertexSize; //get to the next vertex
			
			++vertexCount;
		}
					
		///adds a vertex at the given position
		inline void vertex( float x, float y )
		{				
			DOJO_ASSERT( isEditing() );
			
			//grow the buffer to the needed size			
			if( !vertices || vertexCount >= vertexMaxCount )
				setVertexCap( vertexCount );
			
			if( !currentVertex )
				currentVertex = vertices;
			else
				currentVertex += vertexSize; //get to the next vertex
			
			float* ptr = (float*)currentVertex;
			
			*ptr++ = x;
			*ptr = y;
			
			++vertexCount;
		}
		
		///adds a vertex at the given position
		inline void vertex( float x, float y, float z )
		{			
			DOJO_ASSERT( isEditing() );
			
			if( !vertices || vertexCount >= vertexMaxCount )
				setVertexCap( vertexCount );
			
			float* ptr = (float*)currentVertex;
			
			*ptr++ = x;
			*ptr++ = y;
			*ptr = z;
			
			currentVertex += vertexSize;
			++vertexCount;
		}
				
		///sets the uv of the last set vertex				
		inline void uv( float u, float v )
		{			
			DOJO_ASSERT( isEditing() );
			
			float* ptr = (float*)( currentVertex + vertexFieldOffset[ VF_UV ] );
			*ptr++ = u;
			*ptr = v;
		}
		
		//sets the color of the last set vertex		
		inline void color( float r, float g, float b, float a  )
		{		
			DOJO_ASSERT( isEditing() );	
			
			GLubyte* ptr = (GLubyte*)( currentVertex + vertexFieldOffset[ VF_COLOR ] );
			*ptr++ = (GLubyte)r*255;
			*ptr++ = (GLubyte)g*255;
			*ptr++ = (GLubyte)b*255;
			*ptr = (GLubyte)a*255;
		}
		
		///adds a vertex at the given position
		inline void normal( float x, float y, float z )
		{		
			DOJO_ASSERT( isEditing() );
			
			float* ptr = (float*)( currentVertex + vertexFieldOffset[ VF_NORMAL ] );
			
			*ptr++ = x;
			*ptr++ = y;
			*ptr = z;
		}
		
		///adds one index
		inline void index( uint idx )
		{		
			DOJO_ASSERT( isEditing() );
			
			_growIndices();
			
			indices[ indexCount++ ] = idx;			
		}
		
		///loads the data on the device
		/**
		-will discard all the data if the buffer is static
		-if the buffer is dynamic, this can be called again to update device data
		*/
		bool end()
		{			
			DOJO_ASSERT( isEditing() );
			
			if( !dynamic && isLoaded() ) //already loaded and not dynamic?
				return false;
			
			if( !vertexHandle )
				glGenBuffers(1, &vertexHandle );		
			
			DOJO_ASSERT( vertexHandle );
			
			uint usage = (dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
			
			glBindBuffer(GL_ARRAY_BUFFER, vertexHandle);
			glBufferData(GL_ARRAY_BUFFER, vertexSize * vertexCount, vertices, usage);
									
			if( isIndexed() ) //we support unindexed meshes
			{				
				if( !indexHandle )
					glGenBuffers(1, &indexHandle );
				
				DOJO_ASSERT( indexHandle );
								
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexHandle );
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof( uint ) * indexCount, indices, usage);							
			}
			
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0 );
			
			if( !dynamic ) //won't be updated ever again
				_destroyBuffers();
			
			loaded = glGetError() == GL_NO_ERROR;
			
			currentVertex = NULL;
			editing = false;
						
			return loaded;
		}
		
		///loads the whole file passed in the constructor
		virtual bool load();
		
		
		virtual void unload()
		{
			if( loaded )
			{
				glDeleteBuffers( 1, &vertexHandle );
				glDeleteBuffers( 1, &indexHandle );
				
				vertexHandle = indexHandle = 0;
				
				loaded = false;
			}
		}
				
		///binds all the pointers for the needed client states
		virtual void bind()
		{		
			DOJO_ASSERT( !isEditing() );
			
			glBindBuffer(GL_ARRAY_BUFFER, vertexHandle);
						
			if( vertexFields[ VF_POSITION2D ] )
				glVertexPointer(2, GL_FLOAT, vertexSize, 0 );
				
			else if( vertexFields[ VF_POSITION3D ] )
				glVertexPointer( 3, GL_FLOAT, vertexSize, 0 );
			
			if( vertexFields[ VF_COLOR ] )
				glColorPointer(
							   4, 
							   GL_UNSIGNED_BYTE, 
							   vertexSize, 
							   (void*)vertexFieldOffset[ VF_COLOR ] );
			
			if( vertexFields[ VF_UV ] )
				glTexCoordPointer(
								  2, 
							   GL_FLOAT, 
							   vertexSize, 
							   (void*)vertexFieldOffset[ VF_UV ] );
					
			if( vertexFields[ VF_NORMAL ] )
				glNormalPointer(
							   GL_FLOAT, 
							   vertexSize, 
								(void*)vertexFieldOffset[ VF_NORMAL ] );
						
			if( isIndexed() )			
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexHandle);
		}
				
		inline bool isIndexed()
		{
			return indices || indexHandle;
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
		
		///obtains a copy of this mesh
		/**
		It has to be loaded by hand!
		*/
		inline Mesh* getCopy()
		{
			Mesh* copy = new Mesh( creator, filePath );
			
			copy->dynamic = dynamic;
			copy->vertexSize = vertexSize;			
			memcpy( copy->vertexFields, vertexFields, sizeof( bool ) * FIELDS_NUMBER );
			
			copy->vertexCount = vertexCount;						
			copy->indexCount = indexCount;			
			
			copy->setVertexCap( copy->vertexCount );
			copy->_growIndices();
			
			memcpy( copy->vertices, vertices, vertexSize * vertexMaxCount );
			memcpy( copy->indices, indices, sizeof( uint ) * indexCount );
			
			return copy;
		}
		
		inline float* _getVertex( uint i )
		{			
			DOJO_ASSERT( i < vertexCount );
			
			return (float*)(vertices + vertexSize * i);
		}
				
	protected:
				
		uint vertexSize, vertexCount, vertexMaxCount;
		byte* currentVertex;
		byte* vertices;
		
		uint indexCount, indexMaxCount;
		uint* indices;		
		
		bool vertexFields[ FIELDS_NUMBER ];		
		uint vertexFieldOffset[ FIELDS_NUMBER ];
		
		TriangleMode triangleMode;
		
		uint vertexHandle, indexHandle;
		
		bool dynamic;
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
				
		void _growIndices()
		{			
			indexMaxCount = (indexCount/INDEX_PAGE_SIZE + 1 ) * INDEX_PAGE_SIZE;
			
			if( !indices )
				indices = (uint*)malloc( sizeof( uint ) * indexMaxCount );
			
			else if( indexCount >= indexMaxCount )
				indices = (uint*)realloc( indices, sizeof( uint ) * indexMaxCount );
		}
		
		void _destroyBuffers()
		{
			if( vertices )
			{
				free( vertices );
				vertices = NULL;
			}
			
			if( indices )
			{
				free( indices );
				indices = NULL;
			}
		}
	};
}

#endif