//
//  Render.h
//  NinjaTraining
//
//  Created by Tommaso Checchi on 4/23/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//
#ifndef Render_h__
#define Render_h__

#include "dojo_common_header.h"

#include "Array.h"
#include "Color.h"
#include "Vector.h"
#include "RenderState.h"
#include "BaseObject.h"
#include "Timer.h"

namespace Dojo {
	
	class Renderable;
	class Texture;
	class Viewport;
	class Mesh;
	class Platform;
	class Game;
	
	class Render : public BaseObject
	{	
	public:		

		class Layer : public Dojo::Array< Renderable* >
		{
		public:
			bool depthCheck, lightingOn, projectionOff;

			Layer() :
			projectionOff( true ),
			depthCheck( false ),
			lightingOn( false )
			{

			}
		};
		
		enum RenderOrientation
		{
			RO_PORTRAIT,
			RO_PORTRAIT_REVERSE,
			RO_LANDSCAPE_LEFT,
			RO_LANDSCAPE_RIGHT
		};
						
		typedef Array< Layer* > LayerList;
		
		Render( uint width, uint height, uint devicePixelScale, RenderOrientation);		
		
		~Render();		
						
		void addRenderable( Renderable* s, int layer );
				
		void removeRenderable( Renderable* s );
		
		void removeAllRenderables()
		{
			for( uint i = 0; i < negativeLayers.size(); ++i )
				negativeLayers.at(i)->clear();
			
			for( uint i = 0; i < positiveLayers.size(); ++i )
				positiveLayers.at(i)->clear();
		}
		
		void setViewport( Viewport* v );
						
		void setInterfaceOrientation( RenderOrientation o );
		
		inline RenderOrientation getInterfaceOrientation()
		{
			return renderOrientation;
		}
		
		Layer* getLayer( int layerID );

		uint getLayerNumber()
		{
			return positiveLayers.size() + negativeLayers.size();
		}
		
		inline int getScreenWidth()						{	return width;		}
		inline int getScreenHeight()					{	return height;		}
		
		inline int getWidth()
		{
			return viewportWidth;
		}
		
		inline int getHeight()
		{
			return viewportHeight;
		}
		
		inline float getContentScale()				{	return devicePixelScale;	}
		inline float getNativeToScreenRatio()		{	return nativeToScreenRatio;	}
		inline Viewport* getViewport()				{	return viewport;	}
		
		inline bool isValid()						{	return valid;		}
						
		void startFrame();
		
		void renderElement( Renderable* r );
		
		void renderLayer( Layer* list );
		
		void endFrame();
		
		//renders all the layers and their contained Renderables in the given order
		void render()
		{		
			startFrame();
			
			//first render from the most negative to -1
			if( negativeLayers.size() > 0 )
			{				
				for( int i = negativeLayers.size()-1; i >= 0; --i )
					renderLayer( negativeLayers.at(i) );
			}
			
			//then from 0 to the most positive
			for( uint i = 0; i < positiveLayers.size(); ++i )
				renderLayer( positiveLayers.at(i) );

			endFrame();
		}
				
	protected:	

		Platform* platform;
		
		bool valid;
						
		// The pixel dimensions of the CAEAGLLayer
		int width, height, viewportWidth, viewportHeight;
		float devicePixelScale;
		
		float renderRotation;
		RenderOrientation renderOrientation, deviceOrientation;
		
		float nativeToScreenRatio;
						
		Viewport* viewport;	
		Vector viewportPixelRatio, textureScreenPixelRatio, spriteScreenPixelSize;
		
		RenderState* currentRenderState, *firstRenderState;
		Layer* currentLayer;
				
		bool frameStarted;
		
		LayerList negativeLayers, positiveLayers;
	};		
}

#endif