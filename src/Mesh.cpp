#include "stdafx.h"

#include "Mesh.h"

#include "Utils.h"
#include "Platform.h"

using namespace Dojo;

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

	center.x = (max.x + min.x)*0.5f;
	center.y = (max.y + min.y)*0.5f;
	center.z = (max.z + min.z)*0.5f;

	dimensions = max - min;

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

bool Mesh::end()
{			
	DEBUG_ASSERT( isEditing() );
	
	if( !dynamic && isLoaded() ) //already loaded and not dynamic?
		return false;
	
	glGetError();
	if( !vertexHandle )
		glGenBuffers(1, &vertexHandle );	
		
	int error = glGetError();
	DEBUG_ASSERT( vertexHandle );
	
	uint usage = (dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
	
	glBindBuffer(GL_ARRAY_BUFFER, vertexHandle);
	glBufferData(GL_ARRAY_BUFFER, vertexSize * vertexCount, vertices, usage);
							
	if( isIndexed() ) //we support unindexed meshes
	{				
		if( !indexHandle )
			glGenBuffers(1, &indexHandle );
		
		DEBUG_ASSERT( indexHandle );
						
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexHandle );
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexByteSize * indexCount, indices, usage);							
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0 );
	
	if( !dynamic ) //won't be updated ever again
		_destroyBuffers();
	
	loaded = glGetError() == GL_NO_ERROR;
	
	currentVertex = NULL;
	editing = false;

	//guess triangle count
	uint elemCount = isIndexed() ? getIndexCount() : getVertexCount();
	triangleCount = (triangleMode == TM_LIST) ? elemCount / 3 : elemCount-2;
				
	return loaded;
}

void Mesh::bind()
{		
	DEBUG_ASSERT( vertexHandle );

	glBindBuffer(GL_ARRAY_BUFFER, vertexHandle);

	if( vertexFields[ VF_POSITION2D ] )
		glVertexPointer( 2, GL_FLOAT, vertexSize, 0 );

	else if( vertexFields[ VF_POSITION3D ] )
		glVertexPointer( 3, GL_FLOAT, vertexSize, 0 );

	if( vertexFields[ VF_COLOR ] )
	{
		glColorPointer(
			4, 
			GL_UNSIGNED_BYTE, 
			vertexSize, 
			(void*)vertexFieldOffset[ VF_COLOR ] );

		glEnableClientState(GL_COLOR_ARRAY);
	}
	else
		glDisableClientState( GL_COLOR_ARRAY );

	for( uint i = 0; i < 8; ++i )
	{
		if( vertexFields[ VF_UV + i ] )
		{
			glClientActiveTexture( GL_TEXTURE0 + i );

			glEnableClientState(GL_TEXTURE_COORD_ARRAY);	
			glTexCoordPointer(
				2, 
				GL_FLOAT, 
				vertexSize, 
				(void*)vertexFieldOffset[ VF_UV + i ] );

		}
		else
		{
			glClientActiveTexture( GL_TEXTURE0 + i );
			glDisableClientState( GL_TEXTURE_COORD_ARRAY );
		}
	}

	if( vertexFields[ VF_NORMAL ] )
	{
		glNormalPointer(
			GL_FLOAT, 
			vertexSize, 
			(void*)vertexFieldOffset[ VF_NORMAL ] );

		glEnableClientState(GL_NORMAL_ARRAY);
	}
	else
		glDisableClientState( GL_NORMAL_ARRAY );

	if( isIndexed() )			
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexHandle);
	else
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL );
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

bool Mesh::load()
{
	char* file;
	uint size = Platform::getSingleton()->loadFileContent( file, filePath );
	
	if( !file )
		return false;
	
	char* buf = file;
	char* end = file + size;

	//begin load
	begin();
	
	enum ParseState
	{
		PS_INIT,
		PS_VERTEXDATA,
		PS_INDEXDATA
	} state = PS_INIT;
			
	//TODO why this sometimes hangs?
	
	float x,y,z,w;
	while( buf < end )
	{
		if( state == PS_INIT )
		{
			//fields
			if( Utils::tokenEquals( buf, "position_2D" ) )				setVertexFieldEnabled( VF_POSITION2D );			
			else if( Utils::tokenEquals( buf, "position_3D" ) )			setVertexFieldEnabled( VF_POSITION3D );			
			else if( Utils::tokenEquals( buf, "color" ) )				setVertexFieldEnabled( VF_COLOR );			
			else if( Utils::tokenEquals( buf, "uv" ) )					setVertexFieldEnabled( VF_UV );			
			else if( Utils::tokenEquals( buf, "normal" ) )				setVertexFieldEnabled( VF_NORMAL );
			
			//attributes
			else if( Utils::tokenEquals( buf, "dynamic" ) ) 			dynamic = true;
			else if( Utils::tokenEquals( buf, "triangle_list" ) )		triangleMode = TM_LIST;
			else if( Utils::tokenEquals( buf, "triangle_strip" ) )		triangleMode = TM_STRIP;
			
			//data
			else if( Utils::tokenEquals( buf, "vertex_data" ) )			state = PS_VERTEXDATA;			
			else if( Utils::tokenEquals( buf, "index_data" ) )			state = PS_INDEXDATA;
					
			//get next word
			Utils::nextToken( buf, end );			
		}
		else if( state == PS_VERTEXDATA )
		{			
			if( Utils::tokenEquals( buf, "index_data" ) )
			{				
				state = PS_INDEXDATA;
				Utils::nextToken( buf, end );
			}
			else {
				//try to read serially all the needed attributes
				//remember that something like vertex( Utils::toFloat(), Utils::toFloat() ) WONT WORK!
				if( isVertexFieldEnabled( VF_POSITION2D ) )
				{
					x = Utils::toFloat( buf, end );
					y = Utils::toFloat( buf, end );
					vertex( x,y );
				}				
				if( isVertexFieldEnabled( VF_POSITION3D ) )
				{
					x = Utils::toFloat( buf, end );
					y = Utils::toFloat( buf, end );
					z = Utils::toFloat( buf, end );
					vertex( x,y,z );
				}

				if( isVertexFieldEnabled( VF_UV ) )
				{
					x = Utils::toFloat( buf, end );
					y = Utils::toFloat( buf, end );
					uv( x,y );
				}		
				if( isVertexFieldEnabled( VF_COLOR ) )
				{
					x = Utils::toFloat( buf, end );
					y = Utils::toFloat( buf, end );
					z = Utils::toFloat( buf, end );
					w = Utils::toFloat( buf, end );
					color( x,y,z,w );
				}			
				if( isVertexFieldEnabled( VF_NORMAL ) )
				{
					x = Utils::toFloat( buf, end );
					y = Utils::toFloat( buf, end );
					z = Utils::toFloat( buf, end );
					normal( x,y,z );
				}
			}
		}
		else if( state == PS_INDEXDATA )
		{			
			if( Utils::tokenEquals( buf, "vertex_data" ) )
			{				
				state = PS_VERTEXDATA;
				Utils::nextToken( buf, end );
			}
			else
				index( Utils::toInt( buf, end ) );
		}
	}
	
	//push to device
	this->end();
	
	free( file );
	
	return true;
}
