#include "stdafx.h"

#include "Mesh.h"

#include "Utils.h"
#include "Platform.h"

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

inline void checkGLError()
{
	String err;
	GLenum g = glGetError();

	switch(g)
	{
	case GL_NO_ERROR:
		return;
	case GL_INVALID_ENUM:           err = "GL_INVALID_ENUM";        break;
	case GL_INVALID_VALUE:          err = "GL_INVALID_VALUE";       break;
	case GL_INVALID_OPERATION:		err = "GL_INVALID_OPERATION";   break;
	case GL_STACK_OVERFLOW:         err = "GL_STACK_OVERFLOW";      break;
	case GL_STACK_UNDERFLOW:		err = "GL_STACK_UNDERFLOW";     break;
	case GL_OUT_OF_MEMORY:          err = "GL_OUT_OF_MEMORY";       break;
	};

	DEBUG_ASSERT_MSG( g == GL_NO_ERROR, ( "OpenGL encountered an error: " + err ).ASCII().c_str() );
}

#ifndef _DEBUG
	#define CHECK_GL_ERROR	checkGLError()
#else
	#define CHECK_GL_ERROR
#endif

///Tells the buffer to allocate at least "vertices" vertices
void Mesh::setVertexCap( uint count )
{
	DEBUG_ASSERT( vertexCount <= indexMaxValue ); //check indexability

	if( count < vertexMaxCount ) //no need to grow the buffer
		return;

	vertexMaxCount = (count/VERTEX_PAGE_SIZE + 1 ) * VERTEX_PAGE_SIZE;

	if( !vertices )
	{					
		vertices = (byte*)malloc( vertexSize * vertexMaxCount );

		_buildFieldOffsets();	//build the offsets for each field	
	}			
	else
	{	
		//TODO MAKE THIS ACTUALLY WORK
		vertices = (byte*)realloc( vertices, vertexSize * vertexMaxCount );
	}
}	

void Mesh::setIndexCap( uint count )
{
	if( count < indexMaxCount ) //no need to grow the buffer
		return;

	indexMaxCount = (count/INDEX_PAGE_SIZE + 1 ) * INDEX_PAGE_SIZE;

	if( !indices )
	{					
		indices = (byte*)malloc( indexByteSize * indexMaxCount );
	}
	else
	{	
		//TODO MAKE THIS ACTUALLY WORK
		indices = (byte*)realloc( indices, indexByteSize * indexMaxCount );

		/*GLint* temp = (GLint*)malloc( sizeof(GLint) * indexMaxCount );
		memcpy( temp, indices, sizeof( GLint ) * indexCount );
		free( indices );

		indices = temp;*/
	}
}


void Mesh::_prepareVertex( float x, float y, float z )
{
	DEBUG_ASSERT( isEditing() );

	//grow the buffer to the needed size			
	if( vertexCount >= vertexMaxCount )
		setVertexCap( vertexCount+1 );

	currentVertex = vertices + vertexCount * vertexSize;

	if( x > max.x )	max.x = x;
	else if( x < min.x ) min.x = x;

	if( y > max.y ) max.y = y;
	else if( y < min.y ) min.y = y;

	if( z > max.z ) max.z = z;
	else if( z < min.z ) min.z = z;

	++vertexCount;
}

void Mesh::vertex( float x, float y )
{				
	_prepareVertex(x,y,0);

	float* ptr = (float*)currentVertex;

	ptr[0] = x;
	ptr[1] = y;
}

void Mesh::vertex( float x, float y, float z )
{				
	_prepareVertex(x,y,z);

	float* ptr = (float*)currentVertex;

	ptr[0] = x;
	ptr[1] = y;
	ptr[2] = z;
}

void Mesh::_bindAttribArrays()
{
	glBindBuffer( GL_ARRAY_BUFFER, vertexHandle );

	//construct attributes
	for( int i = 0; i < _VF_COUNT; ++i )
	{
		GLenum state = glFeatureStateMap[i];
		VertexField ft = (VertexField)i;

		if( ft == VF_UV )			glClientActiveTexture( GL_TEXTURE0 );
		else if( ft == VF_UV_1 )	glClientActiveTexture( GL_TEXTURE1 );

		if( isVertexFieldEnabled( ft ) )	//bind data and client states
		{
			glEnableClientState( state );
			CHECK_GL_ERROR;

			void* fieldOffset = (void*)vertexFieldOffset[ ft ];

			switch( ft )
			{
			case VF_POSITION3D:			glVertexPointer(3, GL_FLOAT, vertexSize, fieldOffset );	break;
			case VF_POSITION2D:			glVertexPointer(2, GL_FLOAT, vertexSize, fieldOffset ); break;
			case VF_NORMAL:				glNormalPointer( GL_FLOAT, vertexSize, fieldOffset );	break;
				//TODO switch to BGRA to reduce bandwidth use
			case VF_COLOR:				glColorPointer( 4, GL_UNSIGNED_BYTE, vertexSize, fieldOffset );	break;

			case VF_UV:	
			case VF_UV_1:	
				glTexCoordPointer(2, GL_FLOAT, vertexSize, fieldOffset );	break;
			};

			CHECK_GL_ERROR;
		}
		else if( state != GL_VERTEX_ARRAY ) //do not disable the position
			glDisableClientState( state );
	}

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, isIndexed() ? indexHandle : 0 ); //only bind the index buffer if existing (duh)

	CHECK_GL_ERROR;
}

bool Mesh::end()
{			
	DEBUG_ASSERT( isEditing() );

	editing = false;
	
	if( !dynamic && isLoaded() ) //already loaded and not dynamic?
		return false;

	//create the VBO
	if( !vertexHandle )
		glGenBuffers(1, &vertexHandle );

	uint usage = (dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
	glBindBuffer(GL_ARRAY_BUFFER, vertexHandle);
	glBufferData(GL_ARRAY_BUFFER, vertexSize * vertexCount, vertices, usage);

	CHECK_GL_ERROR;

	//create the IBO
	if( isIndexed() ) //we support unindexed meshes
	{				
		if( !indexHandle )
			glGenBuffers(1, &indexHandle );

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexHandle );
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexByteSize * indexCount, indices, usage);

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
	
	if( mDestroyBuffersOnEnd ) //won't be updated ever again
		_destroyBuffers();
	
	loaded = glGetError() == GL_NO_ERROR;
	
	currentVertex = NULL;

	//guess triangle count
	uint elemCount = isIndexed() ? getIndexCount() : getVertexCount();
	
	switch ( triangleMode ) {
		case TM_LIST:       triangleCount = elemCount / 3;  break;
		case TM_STRIP:      triangleCount = elemCount-2;    break;
		case TM_LINE_STRIP: triangleCount = 0;
	}
	
	//geometric hints
	center.x = (max.x + min.x)*0.5f;
	center.y = (max.y + min.y)*0.5f;
	center.z = (max.z + min.z)*0.5f;
	
	dimensions = max - min;

	return loaded;
}

void Mesh::bind()
{		
	DEBUG_ASSERT( vertexHandle );

#ifndef DOJO_DISABLE_VAOS
	DEBUG_ASSERT( vertexArrayDesc );
	glBindVertexArray( vertexArrayDesc );
#else

	_bindAttribArrays(); //bind attribs each frame! (costly)
#endif

	CHECK_GL_ERROR;
}

const uint Mesh::VERTEX_FIELD_SIZES[] = { 
	2 * sizeof( GLfloat ),
	3 * sizeof( GLfloat ),
	2 * sizeof( GLfloat ),  //uv0
	2 * sizeof( GLfloat ),
	2 * sizeof( GLfloat ),
	2 * sizeof( GLfloat ),
	2 * sizeof( GLfloat ),
	2 * sizeof( GLfloat ),
	2 * sizeof( GLfloat ),
	2 * sizeof( GLfloat ),
	4 * sizeof( GLubyte ),
	3 * sizeof( GLfloat )
};

bool Mesh::onLoad()
{
	DEBUG_ASSERT( !isLoaded() );

	if( !isReloadable() )
		return false;

	//load binary mesh
	char* data;
	Platform::getSingleton()->loadFileContent( data, filePath );
		
	DEBUG_ASSERT( data );
	
	char* ptr = data;
	
	//index size
	setIndexByteSize( *ptr++ );
	
	//triangle mode
	setTriangleMode( (TriangleMode)*ptr++ );
	
	//fields
	for( int i = 0; i < _VF_COUNT; ++i )
	{
		if( *ptr++ )
			setVertexFieldEnabled( (VertexField)i );
	}
	
	//max and min
	memcpy( &max, ptr, sizeof( Vector ) );
	ptr += sizeof( Vector );
	
	memcpy( &min, ptr, sizeof( Vector ) );
	ptr += sizeof( Vector );
		
	//center and dimensions
	center = (max+min)*0.5f;
	dimensions = (max-min);
	
	//vertex count
	uint vc = *((int*)ptr);
	ptr += sizeof( int );
	
	//index count
	uint ic = *((int*)ptr);
	ptr += sizeof( int );
		
	setDynamic( false );
	
	begin( vc );
	
	//grab vertex data
	memcpy( vertices, ptr, vc * vertexSize );
	ptr += vc * vertexSize;
	vertexCount = vc;
	
	//grab index data
	if( ic )
	{
		setIndexCap( ic );
		memcpy( indices, ptr, ic * indexByteSize );
		indexCount = ic;
	}
	
	//push over to GPU
	return end();
}
