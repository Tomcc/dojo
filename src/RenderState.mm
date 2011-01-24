/*
 *  RenderState.mm
 *  Ninja Training
 *
 *  Created by Tommaso Checchi on 8/14/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */

#include "RenderState.h"

#include "Texture.h"
#include "Mesh.h"

using namespace Dojo;

bool RenderState::isAlphaRequired()
{
	return alphaRequired || !texture;
}

void RenderState::commitChanges( RenderState* pastState )
{			
	DEBUG_ASSERT( pastState );
	
	//bind the new texture or nothing
	if( texture && texture != pastState->texture )
		texture->bind();
	else if( !texture )
		glBindTexture(GL_TEXTURE_2D, 0);
	
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