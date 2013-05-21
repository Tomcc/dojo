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

void PolyTextArea::_prepare()
{
	Vector basePosition;

	mMesh->begin();

	for( auto c : mContent )
	{
		//get the tesselation for each character and stuff it into the mesh
		Tessellation* t = pFont->getCharacter( c )->getTesselation();

		DEBUG_ASSERT( t, "The character has no tesselation, have you forgotten to add the flag to the font definition file?" );

		if( mRendering == RT_OUTLINE )
		{
			if( c == '\n')
			{
				DEBUG_TODO; //newline
			}
			else if( c == ' ' )
			{
				//todo
				basePosition.x += 1;
			}

			//merge this outline in the VBO
			int baseIdx = mMesh->getVertexCount();

			for( auto& point : t->positions )
				mMesh->vertex( basePosition + point );

			for( auto& index : t->indices )
				mMesh->index( baseIdx + index );

			basePosition.x = mMesh->getMax().x;
		}
		else
		{
			DEBUG_TODO;
		}
	}

	mMesh->end();

	mDirty = false;
}

