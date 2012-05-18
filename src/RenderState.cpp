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

void RenderState::commitChanges( RenderState* pastState )
{
	DEBUG_ASSERT( pastState );
	DEBUG_ASSERT( mesh );
	    
#ifdef DOJO_FORCE_WHOLE_RENDERSTATE_COMMIT
    
	for( int i = 0; i < 8; ++i )
	{
		//select current slot
		glActiveTexture( GL_TEXTURE0 + i );
            
		if( textures[i] )
		{
			textures[i]->texture->bind(i);
                
			if( textures[i]->isTransformRequired() )
				textures[i]->applyTransform();
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
    
	mesh->bind();
    
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
    
#else
    	
	//bind the new textures
	for( int i = 0; i < 8; ++i )
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
    
	//always bind color as it is just not expensive
	glColor4f( color.r, color.g, color.b, color.a );
}