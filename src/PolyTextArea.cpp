#include "stdafx.h"

#include "PolyTextArea.h"

#include "Mesh.h"

using namespace Dojo;

PolyTextArea::PolyTextArea( Object* parent, const Vector& position, Font* font, bool centered, RenderingType rt, float extrudeDepth ) :
	Renderable( parent, position ),
	mCentered( centered ),
	mDirty( true ), //be sure to init anyways even if the user doesn't write anything
	pFont( font ),
	mDepth( extrudeDepth ),
	mRendering( rt ),
	mInterline( 0 )
{
	DEBUG_ASSERT( pFont, "Cannot create a PolyTextArea with a null font" );

	mSpaceWidth = pFont->getCharacter(' ')->advance;

	//create a new mesh with the required parameters
	mMesh = new Mesh();

	if( mRendering == RT_OUTLINE )
	{
		DEBUG_ASSERT( pFont->hasPolyOutline(), "Cannot create an outline PolyTextArea if the font has no outline" );
		mMesh->setTriangleMode( Mesh::TM_LINE_LIST );
		mMesh->setVertexFieldEnabled( Mesh::VF_POSITION2D );
	}
	else
	{
		DEBUG_ASSERT( pFont->hasPolySurface(), "Cannot create a surface PolyTextArea if the font has no surface " );
		mMesh->setTriangleMode( Mesh::TM_LIST );

		if( mRendering == RT_SURFACE )
			mMesh->setVertexFieldEnabled( Mesh::VF_POSITION2D );
		else
			mMesh->setVertexFieldEnabled( Mesh::VF_POSITION3D );
	}

	setMesh( mMesh );
}

PolyTextArea::~PolyTextArea()
{
	mMesh->onUnload(); //yes, creating and destroying polytextareas is heavy
	SAFE_DELETE( mMesh );
}

void PolyTextArea::_centerLine( int rowStartIdx, float rowWidth )
{
	DEBUG_ASSERT( mCentered, "Cannot center an uncentered PolyTextArea" );
	DEBUG_ASSERT( rowStartIdx >= 0, "invalid rowStartIdx" );
	DEBUG_ASSERT( mesh->isEditing(), "Cannot center a row if mesh is locked" );

	float halfRow = rowWidth * 0.5f;
	for( int i = rowStartIdx; i < mMesh->getVertexCount(); ++i )
	{
		//change back each
		float* v = mMesh->_getVertex( i );
		v[0] -= halfRow;
	}
}

void PolyTextArea::_prepare()
{
	Vector basePosition;
	int rowStartIdx = 0;
	Font::Character* lastChar = nullptr;

	mMesh->begin();

	for( auto c : mContent )
	{
		//get the tesselation for each character and stuff it into the mesh
		auto character = pFont->getCharacter( c );
		Tessellation* t = character->getTesselation();

		DEBUG_ASSERT( t, "The character has no tesselation, have you forgotten to add the flag to the font definition file?" );
		
		if( c == '\n')
		{
			if( mCentered )  //move all the added vertices back
			{
				_centerLine( rowStartIdx, basePosition.x );
				rowStartIdx = mMesh->getVertexCount();
				lastChar = nullptr;
			}

			basePosition.x = 0;
			basePosition.y -= 1.f + mInterline;
		}
		else if( c == ' ' )
		{
			basePosition.x += mSpaceWidth;
		}
		else if( c == '\t' )
		{
			basePosition.x += mSpaceWidth * 4;
		}
		else
		{
			Vector charPosition = basePosition;
			charPosition.x += character->bearingU;
			charPosition.y -= character->bearingV;

			if( pFont->isKerningEnabled() && lastChar )
				charPosition.x -= pFont->getKerning( character, lastChar ); 

			//merge this outline in the VBO
			int baseIdx = mMesh->getVertexCount();

			//choose the right data sources for outline/surface
			auto& vertices = (mRendering == RT_OUTLINE) ? t->positions : t->outPositions;
			auto& indices = (mRendering == RT_OUTLINE) ? t->indices : t->outIndices;

			for( auto& point : vertices )
				mMesh->vertex( charPosition + point );

			for( auto& index : indices )
				mMesh->index( baseIdx + index );
		}

		lastChar = character;
		basePosition.x += character->advance + pFont->getSpacing();
	}

	if( mCentered )
		_centerLine( rowStartIdx, basePosition.x );

	mMesh->end();

	mDirty = false;
}

