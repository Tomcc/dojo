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

namespace Dojo 
{
	class ParticlePool;


	///class tightly coupled to ParticlePool needed to create fast appearing/disappearing effects
	class Particle : public AnimatedQuad 
	{
	public:	

		//HACK
		friend class ParticlePool;

		class EventListener
		{
		public:
			
			virtual void onTimedEvent( Particle* p )=0;
		};
				
		float lifeTime;		
		float spriteSizeScaleSpeed;

		Vector acceleration;
		
		Particle( ParticlePool* p, GameState* level, uint i ) :
		AnimatedQuad( level, Vector::ZERO ),
		pool( p ),
		index( i ),
		lifeTime( 1 )
		{
			onReset();
			
			setVisible( false );
		}		
		
		virtual void onReset()
		{
			AnimatedQuad::reset();

			acceleration.x = 0;
			acceleration.y = 0;
			
			spriteSizeScaleSpeed = 0;
			listener = NULL;

			worldPosition = Vector::ZERO;
			worldRotation = Vector::ZERO;
		}
		
		inline void setTimedEvent( EventListener* l, float lifeTime )
		{			
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
		
		void move( float dt )
		{
			advanceAnim( dt );
			advanceFade( dt );

			worldPosition.x += speed.x * dt;
			worldPosition.y += speed.y * dt;	

			speed.x += acceleration.x * dt;
			speed.y += acceleration.y * dt; 

			worldRotation += rotationSpeed * dt;

			pixelScale.x += spriteSizeScaleSpeed * dt;
			pixelScale.y += spriteSizeScaleSpeed * dt;

			if( launchTimedEvent() )
			{
				getListener()->onTimedEvent( this );
				removeTimedEvent();
			}
		}
		
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