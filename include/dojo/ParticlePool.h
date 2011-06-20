/*
 *  ParticlePool.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 5/5/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef ParticlePool_h__
#define ParticlePool_h__

#include "dojo_common_header.h"

#include "Particle.h"
#include "Viewport.h"
#include "Game.h"

namespace Dojo 
{		
	class ParticlePool : public BaseObject
	{	
	public:		
			
		typedef Array<Particle*> ParticleList;
			
		ParticlePool( GameState* l, int layer ) :
		gameState( l ),
		firstIdleIdx(0),
		destLayer(layer)
		{
			
		}
			
		virtual ~ParticlePool()
		{
			for( uint i = 0; i < pool.size(); ++i )
			{
				_unregisterParticle( pool.at(i) );
				
				delete pool.at(i);
			}
		}
		
		virtual Particle* getParticle( const Vector& pos, float timeToLive, FrameSet* set, float timePerFrame = 0 );
		
		inline void release( Particle* p )
		{
			DEBUG_ASSERT( p );
			
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
		
		inline void releaseAll()
		{
			Particle* p;
			for( uint i = 0; i < pool.size(); ++i )
			{			
				p = pool.at(i);
				
				p->setActive( false );
				p->setVisible( false );
			}
			
			firstIdleIdx = 0;
		}
							
		inline GameState* getGameState()				{	return gameState;		}
		inline Particle* getParticle( uint i )			
		{	
			DEBUG_ASSERT( pool.size() > i );
			return pool.at( i );	
		}
		inline uint getActiveParticleNumber()			{	return firstIdleIdx;	}
					
		inline void update( float dt )
		{
			//update all the active particles
			Particle* p;
			Vector size;
			
			//cleanup
			for( uint i = 0; i < firstIdleIdx; )
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
			for( uint i = 0; i < firstIdleIdx; ++i )
				pool.at(i)->move( dt );
		}
									
	protected:		
				
		ParticleList pool;		
		GameState* gameState;
		
		uint destLayer;		
		uint firstIdleIdx;
									
		inline void _registerParticle( Particle* p )
		{
			//register the particle to updates and manually to the render system
			Platform::getSingleton()->getRender()->addRenderable( p, destLayer );
		}
									
		inline void _unregisterParticle( Particle* p )
		{
			Platform::getSingleton()->getRender()->removeRenderable( p );
		}
									
		inline Particle* _getUnused()
		{			
			if( firstIdleIdx < pool.size() )
				return pool.at( firstIdleIdx );
			
			return NULL;
		}

		virtual Particle* _getParticleImpl()
		{
			return new Particle( this, gameState, pool.size() );
		}
	};
}

#endif