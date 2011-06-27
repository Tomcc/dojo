/*
 *  Renderable.h
 *  Dojo Training
 *
 *  Created by Tommaso Checchi on 7/14/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */

#ifndef Renderable_h__
#define Renderable_h__

#include "dojo_common_header.h"

#include "Object.h"
#include "Color.h"

#include "FrameSet.h"
#include "Render.h"
#include "RenderState.h"
#include "InputSystem.h"

namespace Dojo 
{
	class Renderable : public RenderState, public Object
	{				
	public:	

		class Listener
		{
		public:

			virtual void onButtonPressed( Renderable* r, const Vector& point )=0;
			virtual void onButtonReleased( Renderable* r, const Vector& point )=0;

		protected:
		private:
		};	
		
		Listener* clickListener;		
			
		Vector uvOffset;
				
		Renderable( GameState* level, const Vector& pos ) :
		Object( level, pos, Vector::ONE ),
		layer(0),
		renderingOrder(0),
		visible( true ),
		currentFadeTime(0),
		clickListener( NULL )
		{
			reset();
		}
		
		virtual ~Renderable()
		{
			if( render )
				render->removeRenderable( this );
		}
				
		virtual void reset()
		{
			Object::reset();
			
			visible = true;
			
			color.r = color.g = color.b = color.a = 1.f;
			fading = false;
			scale.x = scale.y = 1;
		}	
				
		inline void setVisible( bool v )				{	visible = v;		}
		
		inline void startFade( const Color& start, const Color& end, float duration )
		{			
			DEBUG_ASSERT( duration > 0 );
			
			fadeStartColor = start;
			fadeEndColor = end;			
			
			color = start;
			
			currentFadeTime = 0;
			
			fadeEndTime = duration;
			
			fading = true;
			
			setVisible( true );
		}
		
		inline void startFade( float startAlpha, float endAlpha, float duration )
		{
			DEBUG_ASSERT( startAlpha <= 1.0f && startAlpha >= 0.f );
			DEBUG_ASSERT( endAlpha <= 1.0f && endAlpha >= 0.f );
			
			color.a = startAlpha;
			
			Color end = color;
			end.a = endAlpha;
			
			startFade(color, end, duration);
		}
		
		inline int getLayer()				{	return layer;			}
		inline uint getRenderingOrder()		{	return renderingOrder;	}

		inline const Vector& getAABBMax()	{	return worldUpperBound;	}
		inline const Vector& getAABBMin()	{	return worldLowerBound;	}
		
		///tells if the object is either visible or has a mesh
		inline bool isVisible()				{	return visible && mesh;	}
		inline bool isFading()				{	return fading;			}


		inline bool contains( const Vector& p )
		{
			return 
				p.x < worldUpperBound.x && 
				p.x > worldLowerBound.x && 
				p.y < worldUpperBound.y && 
				p.y > worldLowerBound.y;
		}

		virtual bool prepare( const Vector& viewportPixelSize )
		{
			return true;
		}
			
		inline void advanceFade( float dt )		
		{			
			if( fading ) //fade is scheduled
			{
				float fade = currentFadeTime/fadeEndTime;
				float invf = 1.f-fade;
				
				color.r = fadeEndColor.r*fade + invf*fadeStartColor.r;
				color.g = fadeEndColor.g*fade + invf*fadeStartColor.g;
				color.b = fadeEndColor.b*fade + invf*fadeStartColor.b;
				color.a = fadeEndColor.a*fade + invf*fadeStartColor.a;
								
				if( currentFadeTime > fadeEndTime )
				{
					fading = false;
					
					if( color.a <= 0.f )
						setVisible( false );
				}				
				
				currentFadeTime += dt;
			}
		}
		
		virtual void onAction( float dt );		
		
		inline void _notifyRenderInfo( Render* r, int layerID, uint renderIdx )
		{			
			render = r;
			layer = layerID;
			renderingOrder = renderIdx;
		}
				
	protected:
		
		bool visible;

		Vector worldUpperBound, worldLowerBound;
		
		Render* render;
		int layer;
		uint renderingOrder;
		
		bool fading;
		float currentFadeTime;
		float fadeEndTime;
		Color fadeStartColor;
		Color fadeEndColor;		
	};
}
#endif
