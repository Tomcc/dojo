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

#include "RenderState.h"
#include "Object.h"
#include "Color.h"

namespace Dojo 
{
	class Render;
	
	class Renderable : public RenderState, public Object
	{				
	public:	

		Vector uvOffset;

		Renderable( Object* parent, const Vector& pos, Mesh* m = nullptr );
				
		Renderable( Object* parent, const Vector& pos, const String& meshName );

		
		virtual ~Renderable();
		
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
			DEBUG_ASSERT( duration > 0, "The duration of a fade must be greater than 0" );
			
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
			color.a = startAlpha;
			
			Color end = color;
			end.a = endAlpha;
			
			startFade(color, end, duration);
		}
		
		inline int getLayer()				{	return layer;			}
		inline uint getRenderingOrder()		{	return renderingOrder;	}
		
		///tells if the object is either visible or has a mesh
		inline bool isVisible()				{	return visible && mesh;	}
		inline bool isFading()				{	return fading;			}
					
		inline bool isInView()
		{
			return !mCulled;
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
					
					if( fadeEndColor.a == 0 )
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
		
		inline void _notifyCulled( bool culled )
		{
			mCulled = culled;
		}
		
		
		
	protected:
		
		bool visible;
		
		Render* render;
		int layer;
		uint renderingOrder;
		
		bool fading;
		float currentFadeTime;
		float fadeEndTime;
		Color fadeStartColor;
		Color fadeEndColor;	
		
	private:
		
		bool mCulled;
	};
}
#endif
