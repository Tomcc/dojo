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

#include "dojo/dojo_common_header.h"

#include "dojo/Renderable.h"
#include "dojo/GameState.h"

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
			setTexture( gameState->getFrameSet( name )->getRandomFrame() );
		}
				
	protected:
		
	};
}

#endif