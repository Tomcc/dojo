/*
 *  Model.h
 *  Ninja Training
 *
 *  Created by Tommaso Checchi on 8/13/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */

#ifndef Model_h__
#define Model_h__

#include "dojo_common_header.h"

#include "Renderable.h"
#include "GameState.h"

namespace Dojo 
{
	class Model : public Renderable
	{
	public:
				
		Model( GameState* gamestate, const Vector& pos, const std::string& meshName = "", const std::string& setName = "" ) :
		Renderable( gamestate, pos )
		{			
			if( !meshName.empty() )
				setMeshName( meshName );
			
			if( !setName.empty() )
				setTextureName( setName );
		}
		
		void setMeshName( const std::string& meshName )
		{
			RenderState::setMesh( gameState->getMesh( meshName ) );
		}

		void setTexture( FrameSet* set )
		{			
			if( set )			
				texture = set->getRandomFrame();
			else 
				texture = NULL;
		}
		
		inline void setTextureName( const std::string& name )
		{
			setTexture( gameState->getFrameSet( name ) );
		}
				
	protected:
		
	};
}

#endif