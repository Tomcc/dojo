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
				setMesh( meshName );
			
			if( !setName.empty() )
				setTexture( setName );
		}
		
		inline void setMesh( Mesh* m )
		{
			DOJO_ASSERT( m );
			
			mesh = m;
		}
		
		inline void setTexture( FrameSet* set )
		{			
			if( set )			
				texture = set->getRandomFrame();
			else 
				texture = NULL;
		}
		
		inline void setMesh( const std::string& meshName )
		{
			setMesh( gameState->getMesh( meshName ) );
		}
		
		inline void setTexture( const std::string& setName )
		{
			setTexture( gameState->getFrameSet( setName ) );
		}
				
	protected:
		
	};
}

#endif