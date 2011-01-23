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

namespace Dojo {
	
	class Renderable;
	class Texture;
	class Viewport;
	class Mesh;
	
	class Render : public BaseObject
	{	
	public:				
		
		enum InterfaceOrientation
		{
			IO_PORTRAIT,
			IO_PORTRAIT_REVERSE,
			IO_LANDSCAPE_LEFT,
			IO_LANDSCAPE_RIGHT
		};
				
		const static uint layerNumber = 10;
		
		typedef Array<Renderable*> RenderableList;
		typedef Array<RenderableList*> LayerList;
		
		Render();		
		
		~Render();		
				
		bool resizeFromLayer( CAEAGLLayer* layer );
		
		void addRenderable( Renderable* s, int layer );
				
		void removeRenderable( Renderable* s );
		
		void removeAllRenderables()
		{
			for( uint i = 0; i < negativeLayers.size(); ++i )
				negativeLayers.at(i)->clear();
			
			for( uint i = 0; i < positiveLayers.size(); ++i )
				positiveLayers.at(i)->clear();
		}
	
		inline void setWindowSize(int w, int h) 
		{
			width = (float)w;
			height = (float)h;
		}
		
		inline void setViewport( Viewport* v )		{	viewport = v;		}		
		inline void setCullingEnabled( bool state )	{	cullingEnabled = state;	}
				
		inline void setInterfaceOrientation( InterfaceOrientation o )		
		{	
			interfaceOrientation = o;
			
			static float orientations[] = 	{ 0, 180, -90, 90 };
			
			interfaceRotation = orientations[ (uint)interfaceOrientation ];
		}
		
		RenderableList* getLayer( int layerID );
		
		inline float getWidth()						{	return width;		}
		inline float getHeight()					{	return height;		}
		
		inline Viewport* getViewport()				{	return viewport;	}
				
		inline InterfaceOrientation getInterfaceOrientation()					
		{	
			return interfaceOrientation;	
		}
		
		inline float getInterfaceRotation()				
		{	
			return interfaceRotation;		
		}
		
		
		inline bool isValid()						{	return valid;		}
						
		void startFrame();
		
		void renderElement( Renderable* r );
		
		void renderLayer( RenderableList* list );
		
		void endFrame();
		
		//renders all the layers and their contained Renderables in the given order
		void render()
		{			
			startFrame();
			
			//first render from the most negative to -1
			if( negativeLayers.size() > 0 )
			{				
				for( uint i = negativeLayers.size(); i >= 0; --i )
					renderLayer( negativeLayers.at(i) );
			}
			
			//then from 0 to the most positive
			for( uint i = 0; i < positiveLayers.size(); ++i )
				renderLayer( positiveLayers.at(i) );
			
			endFrame();
		}
				
	protected:	
		
		EAGLContext *context;	
		bool valid;
						
		// The pixel dimensions of the CAEAGLLayer
		float width;
		float height;
		float devicePixelScale;
		
		float interfaceRotation;		
		InterfaceOrientation interfaceOrientation;
		
		// The OpenGL names for the framebuffer and renderbuffer used to render to this view
		GLuint defaultFramebuffer, colorRenderbuffer;
				
		Viewport* viewport;	
		Vector viewportPixelRatio, textureScreenPixelRatio, spriteScreenPixelSize;
		
		RenderState* currentRenderState, *firstRenderState;
		
		bool cullingEnabled;		
		
		bool frameStarted;
		
		LayerList negativeLayers, positiveLayers;
	};		
}

#endif