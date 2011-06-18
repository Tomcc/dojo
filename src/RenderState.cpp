#include "stdafx.h"

#include "dojo/RenderState.h"

#include "dojo/Texture.h"
#include "dojo/Mesh.h"
#include "dojo/FrameSet.h"
#include "dojo/Timer.h"

using namespace Dojo;

bool RenderState::isAlphaRequired()
{
	return alphaRequired || getTextureNumber() == 0;
}

void RenderState::_bindTextureSlot( int i )
{
	if( getTexture(i) )
		getTexture(i)->bind(i);
	else
	{
		//bind nothing
		glActiveTexture( GL_TEXTURE0 + i );
		glBindTexture( GL_TEXTURE_2D, NULL );
		glDisable( GL_TEXTURE_2D );
	}
}

void RenderState::commitChanges( RenderState* pastState )
{
	DEBUG_ASSERT( pastState );
	DEBUG_ASSERT( mesh );
		
	//bind the new textures
	uint i = 0;
	for( ; i < getTextureNumber() && i < pastState->getTextureNumber(); ++i )
	{
		if( getTexture(i) != pastState->getTexture(i) )
			_bindTextureSlot( i );
	}
	//bind remaining slots
	for( ; i < getTextureNumber(); ++i )
		_bindTextureSlot(i);

	if( !getTextureNumber() )
		_bindTextureSlot(0);

	//apply transform?
	//past state had texture transform, but we dont' need it
	if( textureTransform != pastState->textureTransform )
	{
		glMatrixMode( GL_TEXTURE );
		glLoadIdentity();

		if( textureTransform )
		{
			glScalef( textureScale.x, textureScale.y, 1 );
			glRotatef( textureRotation, 0, 0, 1.f );
			glTranslatef( textureOffset.x, textureOffset.y, 0 );
		}
	}
	
	//bind the new mesh
	if( mesh != pastState->mesh )
		mesh->bind();
	
	//enable alpha
	if( isAlphaRequired() && !pastState->isAlphaRequired() )
	{
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); //normal func				
		glEnable(GL_BLEND);
	}
	//disable alpha
	else if( !isAlphaRequired() && pastState->isAlphaRequired() )
	{
		glBlendFunc(GL_ONE, GL_ZERO);
		glDisable(GL_BLEND);
	}

	if( cullMode != pastState->cullMode )
	{
		switch( cullMode )
		{
		case CM_DISABLED:
			glDisable( GL_CULL_FACE );
			break;

		case CM_BACK:
			glEnable( GL_CULL_FACE );
			glCullFace( GL_BACK );
			break;

		case CM_FRONT:
			glEnable( GL_CULL_FACE );
			glCullFace( GL_FRONT );
		}
	}

	//always bind color as it is just not expensive
	glColor4f( color.r, color.g, color.b, color.a );
}