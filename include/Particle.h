/*
 *  Particle.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 5/5/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef Particle_h__
#define Particle_h__

#include "dojo_common_header.h"

#include "Sprite.h"
#include "GameState.h"

#include "Renderable.h"

namespace Dojo {
	
	class ParticlePool;
	
	//class tightly coupled to ParticlePool needed to create fast appearing/disappearing effects
	class Particle : public AnimatedQuad 
	{
	public:	
		
		class EventListener
		{
		public:
			
			virtual void onTimedEvent( Particle* p )=0;
		};
				
		float lifeTime;		
		float spriteSizeScaleSpeed;
		
		Particle( ParticlePool* p, GameState* level, uint i ) :
		AnimatedQuad( level, Vector::ZERO ),
		pool( p ),
		index( i ),
		lifeTime( 1 )
		{
			reset();
			
			setVisible( false );
		}		
		
		virtual void reset()
		{
			AnimatedQuad::reset();
			
			spriteSizeScaleSpeed = 0;
			listener = NULL;
		}
		
		inline void setTimedEvent( EventListener* l, float lifeTime )
		{
			DEBUG_ASSERT( l );
			
			eventTime = lifeTime;
			listener = l;
		}
		
		inline void removeTimedEvent()
		{
			listener = NULL;
			eventTime = 0;
		}
		
		inline EventListener* getListener()		{	return listener;	}
		
		inline bool launchTimedEvent()
		{
			return listener && lifeTime < eventTime;
		}
		
		bool onMove( float dt )				{	return false;		}
		
		inline void _setPoolIdx( uint i )	{	index = i;			}
		inline uint _getPoolIdx()			{	return index;		}
		
	protected:
		
		uint index;
		
		ParticlePool* pool;
		
		EventListener* listener;
		
		float eventTime;
		
	};
}

#endif