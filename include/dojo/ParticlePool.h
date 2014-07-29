/*
 *  ParticlePool.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 5/5/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "dojo_common_header.h"

#include "Array.h"
#include "Particle.h"

namespace Dojo 
{		
	class GameState;
	
	class ParticlePool 
	{	
	public:		
			
		typedef Array<Particle*> ParticleList;
			
		ParticlePool( GameState* l, int layer ) :
		gameState( l ),
		firstIdleIdx(0),
		destLayer(layer)
		{
			
		}
			
		virtual ~ParticlePool();
		
		virtual Particle* getParticle( const Vector& pos, float timeToLive, FrameSet* set, float timePerFrame = 0 );
		
		void release( Particle* p )
		{
			DEBUG_ASSERT( p, "particle is null" );
			
			if( firstIdleIdx == 0 || p->isActive() == false )
				return;
			
			p->setActive( false );
			p->setVisible( false );
							
			//swap with the first busy
			firstIdleIdx--;
			
			Particle* firstBusy = pool.at( firstIdleIdx );
			
			pool[ firstIdleIdx ] = p;				
			pool[ p->_getPoolIdx() ] = firstBusy;
			
			firstBusy->_setPoolIdx( p->_getPoolIdx() );
			p->_setPoolIdx( firstIdleIdx );
		}
		
		void releaseAll()
		{
			Particle* p;
			for( int i = 0; i < pool.size(); ++i )
			{			
				p = pool.at(i);
				
				p->setActive( false );
				p->setVisible( false );
			}
			
			firstIdleIdx = 0;
		}
							
		GameState* getGameState()				{	return gameState;		}
		Particle* getParticle( int i )			
		{
			return pool.at( i );
		}

		int getActiveParticleNumber()			{	return firstIdleIdx;	}
					
		void update( float dt )
		{
			//update all the active particles
			Particle* p;
			Vector size;
			
			//cleanup
			for( int i = 0; i < firstIdleIdx; )
			{
				for( i = 0; i < firstIdleIdx; ++i )
				{
					p = (Particle*)pool.at(i);
					p->lifeTime -= dt;
					if( p->lifeTime <= 0 )
					{
						release( p );
						break;
					}
				}
			}
			
			//update active particles			
			for( int i = 0; i < firstIdleIdx; ++i )
				pool.at(i)->move( dt );
		}
									
	protected:		
				
		ParticleList pool;		
		GameState* gameState;
		
		int destLayer;		
		int firstIdleIdx;
											
		Particle* _getUnused()
		{			
			if( firstIdleIdx < pool.size() )
				return pool.at( firstIdleIdx );
			
			return NULL;
		}

		virtual Particle* _getParticleImpl();
	};
}

