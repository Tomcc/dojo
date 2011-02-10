/*
 *  Viewport.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/25/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef Viewport_h__
#define Viewport_h__

#include "dojo_common_header.h"

#include "Vector.h"
#include "Color.h"
#include "Object.h"
#include "Model.h"
#include "Sprite.h"
#include "Texture.h"
#include "Platform.h"

namespace Dojo {
	
	class Render;
	
	class Viewport : public Object
	{
	public:
						
		Viewport( GameState* level, const Vector& pos, const Vector& size, const Color& clear, uint targetX, uint targetY ) :
		Object( level, pos, size ),
		clearColor( clear ),
		targetSize( (float)targetX, (float)targetY ),
		background( NULL )
		{
			DEBUG_ASSERT( targetX > 0 && targetY > 0 );

			nativeToScreenRatio = Platform::getSingleton()->getRender()->getNativeToScreenRatio();
			
			//create the fader object			
			fadeObject = new Model( level, position, "texturedQuad", "" );
			fadeObject->color = Color( 0, 0, 0, 0 );
			
			fadeObject->scale.x = size.x;
			fadeObject->scale.y = size.y;
			
			fadeObject->setVisible( false );
			
			level->addObject( fadeObject, 8 );		
		}		
						
		virtual ~Viewport()
		{
			if( background )
			{
				gameState->removeSprite( background );
				delete background;
			}			
			
			gameState->removeSprite( fadeObject );
			delete fadeObject;
		}		
		
		inline void setBackgroundSprite( const std::string& name )
		{			
			DEBUG_ASSERT( name.size() );
			
			if( background )
			{
				gameState->removeSprite( background );
				delete background;
			}
			
			background = new Sprite( gameState, position, name );
			background->setRequiresAlpha( false );
									
			//force the proportions to fill screen
			background->_updateScreenSize();
			
			//the background image must not be stretched on different aspect ratios
			//so we just pick the pixel size for the horizontal			
			Vector ss = background->getScreenSize();
			
			background->pixelScale.x = size.x / ss.x;
			background->pixelScale.y = background->pixelScale.x;	
		}
				
		inline void setClearColor( const Color& color)	{	clearColor = color;	}	
				
		inline const Color& getClearColor()				{	return clearColor;	}
		inline AnimatedQuad* getBackgroundSprite()		{	return background;	}
		inline Model* getFader()						{	return fadeObject;	}
		
		inline Vector makeWorldCoordinates( const Vector& screenPoint )
		{
			return makeWorldCoordinates( (int)screenPoint.x, (int)screenPoint.y );
		}
		
		inline Vector makeWorldCoordinates( int x, int y )
		{
			return Vector(
						  getMinX() + (x / targetSize.x) * size.x,
						  getMaxY() - (y / targetSize.y) * size.y );
		}
						
		void makeScreenSize( Vector& dest, int w, int h );
		
		inline void makeScreenSize( Vector& dest, Texture* tex )
		{
			makeScreenSize( dest, tex->getWidth(), tex->getHeight() );
		}
		
		virtual void action( float dt )
		{
			Object::action( dt );
			
			if( fadeObject->isVisible() )
			{
				fadeObject->position.x = position.x;
				fadeObject->position.y = position.y;
			}
			
			if( background )
			{				
				background->position.x = position.x;
				background->position.y = position.y;
			}		
		}	
				
	protected:
		
		Vector targetSize;
		
		AnimatedQuad* background;
		Model* fadeObject;
				
		Color clearColor;

		float nativeToScreenRatio;
	};
}

#endif

