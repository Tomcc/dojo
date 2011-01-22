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
#include "BaseObject.h"

namespace Dojo
{
	class Texture;
	class Mesh;
	
	///A render state is responsibile of managing the state of the underlying OGL render minimising the changes to be done when it is activated
	class RenderState : public BaseObject
	{
	public:
				
		Color color;
		
		RenderState() :
		alphaRequired( false ),
		mesh( NULL ),
		texture( NULL )
		{
			
		}
		
		inline void setRequiresAlpha( bool required )	{	alphaRequired = required;	}
				
		inline Texture* getTexture()			{	return texture;			}
		inline Mesh* getMesh()					{	return mesh;			}		
		
		bool isAlphaRequired();
		
		///returns the "weight" of the changes needed to pass from "this" to "s"
		inline uint getDistance( RenderState* s )
		{
			DOJO_ASSERT( s );
			
			uint dist = 0;
			
			if( s->mesh != mesh )
				dist += 3;
			
			if( s->texture != texture )
				dist += 2;
			
			if( s->isAlphaRequired() != isAlphaRequired() )
				dist += 1;
			
			return dist;
		}
		
		void commitChanges( RenderState* nextState );
				
	protected:
			
		bool alphaRequired;
		
		Texture* texture;
		Mesh* mesh;
	};
}

#endif