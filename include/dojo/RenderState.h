/*
 *  RenderState.h
 *  Ninja Training
 *
 *  Created by Tommaso Checchi on 8/12/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */

#ifndef RenderState_h__
#define RenderState_h__

#include "dojo_common_header.h"

#include "Color.h"
#include "Vector.h"
#include "Array.h"

namespace Dojo
{
	class Texture;
	class Mesh;
	
	///A render state is responsibile of managing the state of the underlying OGL render minimising the changes to be done when it is activated
	class RenderState 
	{
	public:

		enum CullMode
		{
			CM_FRONT,
			CM_BACK,
			CM_DISABLED
		};
				
		Color color;

		Vector textureOffset, textureScale;
		float textureRotation;

		GLenum srcBlend, destBlend;

		CullMode cullMode;

		RenderState() :
		textureScale( 1,1 ),
		textureRotation( 0 ),
		mTextureNumber( 0 ),
		cullMode( CM_BACK ),
		blendingEnabled( true ),
		textureTransform( false ),
		mesh( NULL ),
		srcBlend( GL_SRC_ALPHA ),
		destBlend( GL_ONE_MINUS_SRC_ALPHA )
		{
			memset( textures, 0, sizeof( textures ) ); //zero all the textures
		}
		
		virtual ~RenderState()
		{
			
		}
		
		inline void setMesh( Mesh* m )
		{			
			DEBUG_ASSERT( m );

			mesh = m;
		}

		///Sets a texture in the required slot.
		/**
		It can be NULL, which means that the slot is disabled.
		*/
		inline void setTexture( Texture* tex, int ID = 0 )
		{
			DEBUG_ASSERT( ID >= 0 );
			DEBUG_ASSERT( ID < 8 );

			if( textures[ID] == NULL ) //adding a new one
				++mTextureNumber;

			textures[ID] = tex;
		}

		inline void setBlendingEnabled( bool enabled )	{	blendingEnabled = enabled;	}

		inline void setRequiresTextureTransform( bool req )
		{
			textureTransform = req;
		}
				
		inline Texture* getTexture( int ID = 0 )
		{
			DEBUG_ASSERT( ID >= 0 );
			DEBUG_ASSERT( ID < 8 );
	
			return textures[ID];
		}

		inline Mesh* getMesh()								{	return mesh;			}
		
		inline int getTextureNumber()
		{
			return mTextureNumber;
		}

		bool isAlphaRequired();
		
		///returns the "weight" of the changes needed to pass from "this" to "s"
		inline uint getDistance( RenderState* s )
		{
			DEBUG_ASSERT( s );
			
			uint dist = 0;
			
			if( s->mesh != mesh )
				dist += 3;
			
			for( int i = 0; i < 8; ++i )
			{
				if( textures[i] != s->textures[i] )
					dist += 2;
			}
			
			if( s->isAlphaRequired() != isAlphaRequired() )
				dist += 1;
			
			return dist;
		}
		
		void commitChanges( RenderState* nextState );
				
	protected:
			
		bool blendingEnabled;
		bool textureTransform;
		
		Texture* textures[8];
		int mTextureNumber;

		Mesh* mesh;

		void _bindTextureSlot( int i );
	};
}

#endif