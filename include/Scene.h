/*
 *  Scene.h
 *  Ninja Training
 *
 *  Created by Tommaso Checchi on 9/8/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */

#ifndef Scene_h__
#define Scene_h__

#include "dojo_config.h"

namespace Dojo
{
	
	class Renderable;
	
	///A scene is a collection of layers of Objects
	/*
	Each GameState is linked to one Scene, and the same goes for RenderTargets.
	However, a Scene can be linked to multiple RenderTargets, allowing to render it more than once per frame.
	*/
	class Scene
	{
	public:
		
		const static uint layerNumber = 10;
		
		typedef Array<Renderable*> RenderableList;
		typedef Array<RenderableList*> LayerList;
		
		Scene()
		{
			
		}		
		
		void addRenderable( Renderable* s, int layer );
		
		void removeRenderable( Renderable* s );
		
		void removeAllRenderables()
		{
			for( uint i = 0; i < negativeLayers.size(); ++i )
				negativeLayers.at(i)->clear();
			
			for( uint i = 0; i < positiveLayers.size(); ++i )
				positiveLayers.at(i)->clear();
		}
				
		inline void setCullingEnabled( bool state )	
		{	
			cullingEnabled = state;	
		}
		
		RenderableList* getLayer( int layerID );		
				
		void renderLayer( RenderableList* list );
		
		virtual void render()
		{			
			//first render from the most negative to -1
			if( negativeLayers.size() > 0 )
			{				
				for( uint i = negativeLayers.size(); i >= 0; --i )
					renderLayer( negativeLayers.at(i) );
			}
			
			//then from 0 to the most positive
			for( uint i = 0; i < positiveLayers.size(); ++i )
				renderLayer( positiveLayers.at(i) );
		}
		
	protected:		
		
		bool cullingEnabled;
		
		LayerList negativeLayers, positiveLayers;
		
	};
}

#endif