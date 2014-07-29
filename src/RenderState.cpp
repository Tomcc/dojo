#include "stdafx.h"

#include "RenderState.h"

#include "Texture.h"
#include "Mesh.h"
#include "FrameSet.h"
#include "Timer.h"
#include "Shader.h"

using namespace Dojo;

Dojo::RenderState::TextureUnit::TextureUnit() :
scale(1, 1),
rotation(0),
offset(0, 0),
texture(nullptr),
optTransform(nullptr) {

}

Dojo::RenderState::TextureUnit::~TextureUnit() {
	if (optTransform)
		SAFE_DELETE(optTransform);
}


void RenderState::TextureUnit::_updateTransform() {
	if (!optTransform)
		optTransform = new Matrix;

	//build the transform
	*optTransform = glm::scale(Matrix(1), scale);
	*optTransform = glm::translate(*optTransform, offset);
	*optTransform = glm::rotate(*optTransform, Math::toEuler(rotation), Vector::UNIT_Z);
}

void Dojo::RenderState::TextureUnit::applyTransform() {
	DEBUG_ASSERT(optTransform, "Tried to apply a non-existing texture transform");

	glMatrixMode(GL_TEXTURE);
	glLoadMatrixf(glm::value_ptr(*optTransform));
}

const Matrix& RenderState::TextureUnit::getTransform() {
	static const Matrix identityMatrix;
	return isTransformRequired() ? *optTransform : identityMatrix;
}

RenderState::RenderState() :
mTextureNumber(0),
cullMode(CM_BACK),
blendingEnabled(true),
srcBlend(GL_SRC_ALPHA),
destBlend(GL_ONE_MINUS_SRC_ALPHA),
blendEquation(GL_FUNC_ADD),
mesh(nullptr),
pShader(nullptr) {
	memset(textures, 0, sizeof(textures)); //zero all the textures
}

RenderState::~RenderState() {
	for (int i = 0; i < DOJO_MAX_TEXTURES; ++i)
		delete textures[i];
}

void RenderState::setTexture(Texture* tex, int ID /*= 0 */) {
	DEBUG_ASSERT(ID >= 0, "Passed a negative texture ID to setTexture()");
	DEBUG_ASSERT(ID < DOJO_MAX_TEXTURES, "An ID passed to setTexture must be smaller than DOJO_MAX_TEXTURE_UNITS");

	if (textures[ID] == NULL) //adding a new one
	{
		++mTextureNumber;
		textures[ID] = new TextureUnit();
	}
	else if (tex == NULL)
		SAFE_DELETE(textures[ID]);

	textures[ID]->texture = tex;
}

void RenderState::setBlending(BlendingMode mode) {
	static const GLenum modeToGLTable[] = {
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, //alphablend
		GL_DST_COLOR, GL_ZERO, //multiply
		GL_ONE, GL_ONE, //add
		GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR //invert
	};

	setBlending(modeToGLTable[mode * 2], modeToGLTable[mode * 2 + 1]);
}

void RenderState::setShader(Shader* shader) {
	pShader = shader;
}

Texture* RenderState::getTexture(int ID /*= 0 */) {
	DEBUG_ASSERT(ID >= 0, "Can't retrieve a negative texture ID");
	DEBUG_ASSERT(ID < DOJO_MAX_TEXTURES, "An ID passed to getTexture must be smaller than DOJO_MAX_TEXTURE_UNITS");

	if (textures[ID])
		return textures[ID]->texture;
	else
		return NULL;
}

RenderState::TextureUnit* RenderState::getTextureUnit(int ID) {
	DEBUG_ASSERT(ID >= 0, "Can't retrieve a negative textureUnit");
	DEBUG_ASSERT(ID < DOJO_MAX_TEXTURES, "An ID passed to getTextureUnit must be smaller than DOJO_MAX_TEXTURE_UNITS");

	return textures[ID];
}

int RenderState::getDistance(RenderState* s) {
	DEBUG_ASSERT(s, "getDistance: The input RenderState is null");

	int dist = 0;

	if (s->mesh != mesh)
		dist += 3;

	for (int i = 0; i < DOJO_MAX_TEXTURES; ++i)
	{
		if (textures[i] != s->textures[i])
			dist += 2;
	}

	if (s->isAlphaRequired() != isAlphaRequired())
		dist += 1;

	return dist;
}

void RenderState::useAlphaBlend() {
	srcBlend = GL_SRC_ALPHA;
	destBlend = GL_ONE_MINUS_SRC_ALPHA;
	blendEquation = GL_FUNC_ADD;
}

void RenderState::useAdditiveBlend() {
	destBlend = GL_ONE;
	srcBlend = GL_SRC_ALPHA;
	blendEquation = GL_FUNC_ADD;
}


bool RenderState::isAlphaRequired()
{
	return blendingEnabled || getTextureNumber() == 0;
}

void RenderState::applyState()
{
	for( int i = 0; i < DOJO_MAX_TEXTURES; ++i )
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

	mesh->bind( pShader );
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
