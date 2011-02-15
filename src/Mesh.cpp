#include "stdafx.h"

#include "Mesh.h"

#include "Utils.h"
#include "Platform.h"

using namespace Dojo;

bool Mesh::end()
{			
	DEBUG_ASSERT( isEditing() );
	
	if( !dynamic && isLoaded() ) //already loaded and not dynamic?
		return false;
	
	if( !vertexHandle )
		glGenBuffers(1, &vertexHandle );	
	
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

void Mesh::bind()
{		
	DEBUG_ASSERT( !isEditing() );

	glBindBuffer(GL_ARRAY_BUFFER, vertexHandle);

	if( vertexFields[ VF_POSITION2D ] )
		glVertexPointer(2, GL_FLOAT, vertexSize, 0 );

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

	if( vertexFields[ VF_UV ] )
	{
		glTexCoordPointer(
			2, 
			GL_FLOAT, 
			vertexSize, 
			(void*)vertexFieldOffset[ VF_UV ] );

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);	
	}
	else
		glDisableClientState( GL_TEXTURE_COORD_ARRAY );

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
}

const uint Mesh::VERTEX_FIELD_SIZES[] = { 
	2 * sizeof( GLfloat ),
	3 * sizeof( GLfloat ),
	2 * sizeof( GLfloat ),
	4 * sizeof( GLubyte ),
	3 * sizeof( GLfloat ) };

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
			
	float x,y,z,w;
	while( buf < end )
	{
		if( state == PS_INIT )
		{
			//fields
			if( Utils::tokenEquals( buf, "position_2D" ) )				setVertexFieldEnabled( VF_POSITION2D, true );			
			else if( Utils::tokenEquals( buf, "position_3D" ) )			setVertexFieldEnabled( VF_POSITION3D, true );			
			else if( Utils::tokenEquals( buf, "color" ) )				setVertexFieldEnabled( VF_COLOR, true );			
			else if( Utils::tokenEquals( buf, "uv" ) )					setVertexFieldEnabled( VF_UV, true );			
			else if( Utils::tokenEquals( buf, "normal" ) )				setVertexFieldEnabled( VF_NORMAL, true );
			
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
