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
#include "BaseObject.h"
#include "Vector.h"
#include "Array.h"

namespace Dojo
{
	class Texture;
	class Mesh;
	
	///A render state is responsibile of managing the state of the underlying OGL render minimising the changes to be done when it is activated
	class RenderState : public BaseObject
	{
	public:

		typedef Array<Texture*> TextureList;
				
		Color color;

		Vector textureOffset, textureScale;
		float textureRotation;

		RenderState() :
		alphaRequired( true ),
		mesh( NULL ),
		textureRotation( 0 ),
		textureTransform( false ),
		textureScale( 1,1 )
		{
			
		}
		
		inline void setMesh( Mesh* m )
		{
			DEBUG_ASSERT( m );
			
			mesh = m;
		}

		void setTexture( Texture* tex, uint ID = 0 )
		{
			DEBUG_ASSERT( ID <= getTextureNumber() );

			if( ID == getTextureNumber() ) //push new
				textures.add( tex );
			else
				textures.at( ID ) = tex;
		}

		inline void setRequiresAlpha( bool required )	{	alphaRequired = required;	}
		inline void setRequiresTextureTransform( bool req )
		{
			textureTransform = req;
		}
				
		inline Texture* getTexture( uint ID = 0 )			
		{	
			return (getTextureNumber()) ? textures.at(ID) : NULL;	
		}
		inline Mesh* getMesh()								{	return mesh;			}
		inline uint getTextureNumber()
		{
			return textures.size();
		}
		
		bool isAlphaRequired();
		
		///returns the "weight" of the changes needed to pass from "this" to "s"
		inline uint getDistance( RenderState* s )
		{
			DEBUG_ASSERT( s );
			
			uint dist = 0;
			
			if( s->mesh != mesh )
				dist += 3;
			
			if( s->getTextureNumber() != s->getTextureNumber() )
				dist += 2;

			else if( s->getTexture() != getTexture() )
				dist += 2;
			
			if( s->isAlphaRequired() != isAlphaRequired() )
				dist += 1;
			
			return dist;
		}
		
		void commitChanges( RenderState* nextState );
				
	protected:
			
		bool alphaRequired;
		bool textureTransform;
		
		TextureList textures;

		Mesh* mesh;
	};
}

#endif