/*
 *  Sprite.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/23/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef Sprite_h__
#define Sprite_h__

#include "dojo_common_header.h"

#include "Array.h"

#include "Texture.h"
#include "Vector.h"
#include "Renderable.h"
#include "FrameSet.h"
#include "Color.h"
#include "Render.h"
#include "AnimatedQuad.h"

namespace Dojo
{	
	class Sprite : public AnimatedQuad
	{
		
		
	public:		
				
		typedef Array<Animation*> AnimationList;
														
		Sprite( GameState* level, const Vector& pos, const std::string& defaultAnimName = "", float timePerFrame = 1 );
				
		virtual ~Sprite()
		{
			for( uint i = 0; i < animations.size(); ++i )
				delete animations.at(i);
			
			animation = NULL;
			
			//frames have to be relased manually from the group!
		}		

		///registers the given animation and returns its handle
		inline uint registerAnimation( FrameSet* set, float timePerFrame )
		{
			DOJO_ASSERT( set );
			
			Animation* a = new Animation( set, timePerFrame );
		
			animations.addElement( a );		
			
			return animations.size()-1;
		}
		
		uint registerAnimation( const std::string& base, float timePerFrame );
				
		///sets the animation at the given index
		inline void setAnimation( uint i ) 	
		{				
			DOJO_ASSERT( animations.size() > i );
			
			animation = animations.at(i);
			
			_setTexture( animation->getCurrentFrame() );
			
			_updateScreenSize();
		}
		
	protected:		
		AnimationList animations;
	};
}

#endif