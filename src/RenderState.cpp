#include "stdafx.h"

#include "RenderState.h"

#include "Texture.h"
#include "Mesh.h"
#include "FrameSet.h"
#include "Timer.h"

using namespace Dojo;

bool RenderState::isAlphaRequired()
{
	return blendingEnabled || getTextureNumber() == 0;
}

void RenderState::applyState()
{
	for( int i = 0; i < DOJO_MAX_TEXTURE_UNITS; ++i )
	{
		//select current slot
		glActiveTexture( GL_TEXTURE0 + i );
		
		if( textures[i] )
		{
			textures[i]->texture->bind(i);
			
			if( textures[i]->isTransformRequired() )
				textures[i]->applyTransform();
			else
			{
				glMatrixMode( GL_TEXTURE );
				glLoadIdentity();
			}
		}
		else
		{
			//override the previous bound texture with nothing
			glBindTexture( GL_TEXTURE_2D, 0 );
			glDisable( GL_TEXTURE_2D );
			
			glMatrixMode( GL_TEXTURE );
			glLoadIdentity();
		}
	}
	
	if( blendingEnabled )	glEnable( GL_BLEND );
	else                    glDisable( GL_BLEND );
	
	glBlendFunc( srcBlend, destBlend );
	glBlendEquation( blendEquation );
	
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
			break;
	}

	mesh->bind();
}

void RenderState::commitChanges( RenderState* pastState )
{
	DEBUG_ASSERT( pastState, "the past RenderState is null" );
	DEBUG_ASSERT( mesh, "A mesh is required to setup a new renderstate" );

	//always bind color as it is just not expensive
	glColor4f( color.r, color.g, color.b, color.a );
		
#ifdef DOJO_FORCE_WHOLE_RENDERSTATE_COMMIT
	
	applyState();
	
#else
		
	//bind the new textures
	for( int i = 0; i < DOJO_MAX_TEXTURE_UNITS; ++i )
	{
		//different from previous?
		if( (pastState->textures[i] == NULL && textures[i] != NULL) ||
			(pastState->textures[i] != NULL && textures[i] == NULL) ||
			((pastState->textures[i] && textures[i]) && pastState->textures[i]->texture != textures[i]->texture ) )
		{
			//select current slot
			glActiveTexture( GL_TEXTURE0 + i );

			if( textures[i] )
			{
				textures[i]->texture->bind(i);

				if( textures[i]->isTransformRequired() )
				{
					textures[i]->applyTransform();
				}
				else if( pastState->textures[i] && pastState->textures[i]->isTransformRequired() ) //override with null trans
				{
					glMatrixMode( GL_TEXTURE );
					glLoadIdentity();
				}
			}
			else
			{
				//override the previous bound texture with nothing
				glBindTexture( GL_TEXTURE_2D, NULL );
				glDisable( GL_TEXTURE_2D );
			}
		}
	}

	//bind the new mesh
	if( mesh != pastState->mesh )
		mesh->bind();

	//enable or disable blending
	if( blendingEnabled != pastState->blendingEnabled )
	{
		if( blendingEnabled )
			glEnable( GL_BLEND );
		else
			glDisable( GL_BLEND ); 
	}
	
	//change blending mode
	if( srcBlend != pastState->srcBlend || destBlend != pastState->destBlend )
		glBlendFunc( srcBlend, destBlend );

	//change blending equation
	if( blendEquation != pastState->blendEquation )
		glBlendEquation( blendEquation );

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
#endif
	
}