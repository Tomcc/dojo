/*
 *  Renderable.h
 *  Dojo Training
 *
 *  Created by Tommaso Checchi on 7/14/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */

#pragma once

#include "dojo_common_header.h"

#include "RenderState.h"
#include "Object.h"
#include "Color.h"

namespace Dojo 
{
	class Renderer;
	
	class Renderable : public RenderState, public Object
	{				
	public:	

		Vector uvOffset;

		Renderable( Object* parent, const Vector& pos, Mesh* m = nullptr );
				
		Renderable( Object* parent, const Vector& pos, const String& meshName );
		
		virtual ~Renderable();
		
		virtual void reset();
				
		void setVisible( bool v )				{	visible = v;		}
		
		///starts a linear fade on the color of this Renderable, from start to end and "duration" seconds long
		void startFade( const Color& start, const Color& end, float duration );
		
		///starts a linear fade on the alpha of this Renderable, from start to end and "duration" seconds long
		void startFade( float startAlpha, float endAlpha, float duration );

		///stops a current fade leaving the Renderable in the current state
		void stopFade();
		
		///returns the ID of the Render::Layer this object is assigned to
		int getLayer()	const			{	return layer;			}
		int getRenderingOrder()		{	return renderingOrder;	}

		///true if this object has been assigned to a Render::Layer
		bool hasLayer()						{	return layer != INT_MIN;	}
		
		///tells if the object is either visible or has a mesh
		bool isVisible() const		{ return visible; }
		bool canBeRendered() const;
		bool isFading()	const			{	return fading;			}
        
        virtual bool isRenderable()         {   return true;            }
		
		void advanceFade( float dt );
		
		virtual void onAction( float dt );		
		
		void _notifyRenderInfo( Renderer* r, int layerID, int renderIdx );
	protected:
		
		bool visible;
		
		Renderer* render;
		int layer;
		int renderingOrder;
		
		bool fading;
		float currentFadeTime;
		float fadeEndTime;
		Color fadeStartColor;
		Color fadeEndColor;	
	};
}
