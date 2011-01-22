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
#include "color.h"

#include "FrameSet.h"
#include "Render.h"
#include "RenderState.h"

namespace Dojo 
{
	class Renderable : public Object, public RenderState
	{				
	public:		
				
		class ClickListener
		{
		public:
			
			ClickListener()
			{
				
			}
			
			virtual void onButtonPressed( Renderable* s ) = 0;			
			virtual void onButtonReleased( Renderable* s ) = 0;
			
		protected:
		};		
		
		ClickListener* clickListener;
		
		float spriteRotation, rotationSpeed;		
		
		Vector scale;		
		Vector uvOffset;
				
		Renderable( GameState* level, const Vector& pos ) :
		Object( level, pos, Vector::ONE ),
		layer(0),
		renderingOrder(0),
		visible( true ),
		rendered( false ),
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
			spriteRotation = 0;
			
			rotationSpeed = 0;
			color.r = color.g = color.b = color.a = 1.f;
			fading = false;
			scale.x = scale.y = 1;
		}	
				
		inline void setVisible( bool v )				{	visible = v;		}
		
		inline void startFade( const Color& start, const Color& end, float duration )
		{			
			DOJO_ASSERT( duration > 0 );
			
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
			DOJO_ASSERT( startAlpha <= 1.0f && startAlpha >= 0.f );
			DOJO_ASSERT( endAlpha <= 1.0f && endAlpha >= 0.f );
			
			color.a = startAlpha;
			
			Color end = color;
			end.a = endAlpha;
			
			startFade(color, end, duration);
		}
		
		inline int getLayer()								{	return layer;			}
		inline uint getRenderingOrder()						{	return renderingOrder;	}
		
		inline bool isVisible()								{	return visible;			}
		inline bool isFading()								{	return fading;			}
		inline bool isRendered()							{	return rendered;		}
		
		virtual void prepare( const Vector& viewportPixelSize )
		{
			
		}
			
		inline void advanceFade( float dt )		
		{			
			if( fading ) //fade is scheduled
			{
				float fade = currentFadeTime/fadeEndTime;
				
				color.r = fadeEndColor.r*fade + (1.f-fade)*fadeStartColor.r;
				color.g = fadeEndColor.g*fade + (1.f-fade)*fadeStartColor.g;
				color.b = fadeEndColor.b*fade + (1.f-fade)*fadeStartColor.b;
				color.a = fadeEndColor.a*fade + (1.f-fade)*fadeStartColor.a;
								
				if( currentFadeTime > fadeEndTime )
				{
					fading = false;
					
					if( color.a <= 0.f )
						setVisible( false );
				}				
				
				currentFadeTime += dt;
			}
		}
		
		virtual void action( float dt );		
		
		inline void _notifyRenderInfo( Render* r, int layerID, uint renderIdx )
		{			
			render = r;
			layer = layerID;
			renderingOrder = renderIdx;
		}
		
		///this tells if the given viewport contains any pixel at all of this renderable
		/** by default this checks against collision with Object's size */
		virtual bool _canBeRenderedBy( Viewport* v );
		
	protected:
		
		bool visible;
		bool rendered;
		
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