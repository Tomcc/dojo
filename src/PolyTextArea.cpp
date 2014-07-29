#include "stdafx.h"

#include "PolyTextArea.h"

#include "Mesh.h"
#include "Tessellation.h"
#include "Font.h"

using namespace Dojo;

PolyTextArea::PolyTextArea( Object* parent, const Vector& position, Font* font, bool centered, RenderingType rt ) :
	Renderable( parent, position ),
	mCentered( centered ),
	mDirty( true ), //be sure to init anyways even if the user doesn't write anything
	pFont( font ),
	mRendering( rt ),
	mInterline( 0 ),
	mInflateRadius( 0.01f ),
	mDepth( 0.15f ),
	mBevelDepth( 0.015f )
{
	DEBUG_ASSERT( pFont, "Cannot create a PolyTextArea with a null font" );

	mSpaceWidth = pFont->getCharacter(' ')->advance;

	//create a new mesh with the required parameters
	mMesh = new Mesh();

	if( mRendering == RT_OUTLINE )
	{
		DEBUG_ASSERT( pFont->hasPolyOutline(), "Cannot create an outline PolyTextArea if the font has no outline" );
		mMesh->setTriangleMode( TriangleMode::LineList );
		mMesh->setVertexFieldEnabled( VertexField::Position2D );
	}
	else
	{
		DEBUG_ASSERT( pFont->hasPolySurface(), "Cannot create a surface PolyTextArea if the font has no surface " );
		mMesh->setTriangleMode( TriangleMode::TriangleList );

		if( mRendering == RT_SURFACE )
		{
			mMesh->setVertexFieldEnabled( VertexField::Position2D );
			//the normal here is unbound and defaults to 0,0,1 apparently, which is correct
			//and allows for a substantial bandwidth saving
			//TODO check if this works on every driver (tested on Intel HD4000, NV 630)
		}
		else
		{
			mMesh->setVertexFieldEnabled( VertexField::Position3D );
			mMesh->setVertexFieldEnabled( VertexField::Normal );
		}
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
	for (Mesh::IndexType i = rowStartIdx; i < mMesh->getVertexCount(); ++i)
		mMesh->getVertex(i).x -= halfRow; //change back each
}

void PolyTextArea::_tesselateExtrusionStrip( Tessellation* t, int baseIdx, int layerBbaseIdx )
{
	int backFaceOffset = layerBbaseIdx - baseIdx;

	//create a strip to bind the two faces together
	for( auto& segment : t->extrusionContourIndices )
		{
			mMesh->triangle( 
				baseIdx + segment.i2, 
				baseIdx + segment.i1 + backFaceOffset,
				baseIdx + segment.i1 );
			mMesh->triangle( 
				baseIdx + segment.i2, 
				baseIdx + segment.i2 + backFaceOffset,
				baseIdx + segment.i1 + backFaceOffset );
		}
}

void PolyTextArea::_addExtrusionLayer( Tessellation* t, const Vector& origin, float inflate, const Vector* forcedNormal )
{
	int layerIdx = mMesh->getVertexCount();

	for( auto& vertex : t->extrusionContourVertices )
	{
		mMesh->vertex( origin + vertex.position + vertex.normal * inflate );
		mMesh->normal( forcedNormal ? *forcedNormal : vertex.normal );
	}

	if( mPrevLayerIdx >= 0 )
		_tesselateExtrusionStrip( t, mPrevLayerIdx, layerIdx );

	mPrevLayerIdx = layerIdx;
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

			//merge this mesh in the VBO
			int baseIdx = mMesh->getVertexCount();

			if( mRendering == RT_SURFACE )
			{
				for( auto& pos : t->positions )
					mMesh->vertex( charPosition + pos.toVec() );
			
				for( auto& index : t->outIndices )
					mMesh->index( baseIdx + index );
			}
			else if( mRendering == RT_EXTRUDED )
			{
				//tesselate front face
				for( auto& index : t->outIndices )
					mMesh->index( baseIdx + index );

				//extrude the character
				mPrevLayerIdx = -1;

				_addExtrusionLayer( t, charPosition, 0, &Vector::UNIT_Z );

				if( mInflateRadius && mBevelDepth > 0 ) //use a rounded extrusion only if the inflation and the bevel are valid
				{
					_addExtrusionLayer( t, charPosition - Vector( 0,0, mBevelDepth ), mInflateRadius, nullptr );
					_addExtrusionLayer( t, charPosition - Vector( 0,0, mDepth - mBevelDepth ), mInflateRadius, nullptr );
				}

				_addExtrusionLayer( t, charPosition - Vector(0,0,mDepth), 0, &Vector::NEGATIVE_UNIT_Z );

				//add backface - flip index winding to flip the face
				for( size_t i = 0; i < t->outIndices.size(); i += 3 )
					mMesh->triangle( 
						mPrevLayerIdx + t->outIndices[i+2], 
						mPrevLayerIdx + t->outIndices[i+1], 
						mPrevLayerIdx + t->outIndices[i] );
			}
			else //HACK do not actually use contours here
			{
				for( size_t i = 0; i < t->positions.size(); ++i )
				{
					mMesh->vertex( charPosition + t->positions[i].toVec() );
				}

				for( auto& contour : t->contours )
					for( auto& index : contour.indices )
						mMesh->index( baseIdx + index );
			}
		}

		lastChar = character;
		basePosition.x += character->advance + pFont->getSpacing();
	}

	if( mCentered )
		_centerLine( rowStartIdx, basePosition.x );

	mMesh->end();

	mDirty = false;
}

