#include "stdafx.h"

#include "RenderState.h"

#include "Texture.h"
#include "Mesh.h"
#include "FrameSet.h"

using namespace Dojo;

bool RenderState::isAlphaRequired()
{
	return alphaRequired || !texture;
}

void RenderState::commitChanges( RenderState* pastState )
{			
	DEBUG_ASSERT( pastState );
		
	//bind the new texture or nothing
	if( texture )
	{
		if( texture != pastState->texture )
			texture->bind();

		//apply transform?
		//past state had texture transform, but we dont' need it

	}
	else
		glBindTexture(GL_TEXTURE_2D, 0);

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

	//always bind color as it is just not expensive
	glColor4f( color.r, color.g, color.b, color.a );	
}