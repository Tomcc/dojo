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
				
		Model( GameState* gamestate, const Vector& pos, const String& meshName = String::EMPTY, const String& setName = String::EMPTY ) :
		Renderable( gamestate, pos )
		{			
			if( !meshName.empty() )
				setMeshName( meshName );
			
			if( !setName.empty() )
				setTextureName( setName );
		}
		
		void setMeshName( const String& meshName )
		{
			RenderState::setMesh( gameState->getMesh( meshName ) );
		}

		inline void setTextureName( const String& name )
		{
			FrameSet* t = gameState->getFrameSet(name);
			
			DEBUG_ASSERT( t );
			
			setTexture( t->getRandomFrame() );
		}
				
	protected:
		
	};
}

#endif