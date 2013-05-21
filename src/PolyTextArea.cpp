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
	mRendering( rt )
{
	DEBUG_ASSERT( pFont, "Cannot create a PolyTextArea with a null font" );

	if( mRendering == RT_OUTLINE )
	{
		DEBUG_ASSERT( pFont->hasPolyOutline(), "Cannot create an outline PolyTextArea if the font has no outline" );
	}
	else
	{
		DEBUG_ASSERT( pFont->hasPolySurface(), "Cannot create a surface PolyTextArea if the font has no surface " );
	}

	mSpaceWidth = pFont->getCharacter(' ')->advance;

	//create a new mesh with the required parameters
	mMesh = new Mesh();
	mMesh->setTriangleMode( (mRendering == PolyTextArea::RT_OUTLINE) ? Mesh::TM_LINE_LIST : Mesh::TM_LIST );
	mMesh->setVertexFieldEnabled( (mRendering == PolyTextArea::RT_EXTRUDED) ? Mesh::VF_POSITION3D : Mesh::VF_POSITION2D );

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
			basePosition.y -= 1.f;
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
				charPosition.x += pFont->getKerning( character, lastChar ); 

			//merge this outline in the VBO
			if( mRendering == RT_OUTLINE )
			{
				int baseIdx = mMesh->getVertexCount();

				for( auto& point : t->positions )
					mMesh->vertex( charPosition + point );

				for( auto& index : t->indices )
					mMesh->index( baseIdx + index );
			}
			else
			{
				DEBUG_TODO;
			}
		}

		lastChar = character;
		basePosition.x += character->advance;
	}

	if( mCentered )
		_centerLine( rowStartIdx, basePosition.x );

	mMesh->end();

	mDirty = false;
}

