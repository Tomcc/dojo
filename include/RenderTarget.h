/*
 *  RenderTarget.h
 *  Ninja Training
 *
 *  Created by Tommaso Checchi on 9/8/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */

#ifndef RenderTarget_h__
#define RenderTarget_h__

#include "dojo_config.h"

#include "Buffer.h"
#include "Scene.h"

namespace Dojo 
{
	class RenderTarget : public Buffer 
	{
	public:
		
		RenderTarget( Scene* s = NULL, Viewport* v = NULL ) :
		scene( s ),
		viewport( v )
		{
			
		}
		
		inline Scene* getScene()
		{
			return scene;
		}
		
		inline Viewport* getViewport()
		{
			return viewport;
		}
		
		inline void setScene( Scene* s )
		{
			DOJO_ASSERT( s );
			
			scene = s;
		}
		
		inline void setViewport( Viewport* v )
		{
			DOJO_ASSERT( v );
			
			viewport = v;
		}
		
		///renders the scene from the given point of view
		virtual void render()
		{
			//bind framebuffer
			bind();
			
			//setup viewport
			
			//render scene
			scene->render();
		}
		
	protected:
		
		Viewport* viewport;
		
		Scene* scene;
	};
}

#endif