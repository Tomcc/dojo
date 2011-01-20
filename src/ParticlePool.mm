/*
 *  ParticlePool.mm
 *  Ninja Training
 *
 *  Created by Tommaso Checchi on 8/4/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */

#include "ParticlePool.h"

#include "Viewport.h"

using namespace Dojo;

Particle* ParticlePool::getParticle( const Vector& pos, float timeToLive, FrameSet* set, float timePerFrame )
{
	Particle* p = _getUnused();
	
	if( !p ) //no idle
	{
		p = new Particle( this, gameState, pool.size() );
		pool.addElement( p );
		
		_registerParticle( p );
	}
	else
	{				
		//wake up
		p->reset();
	}
	
	++firstIdleIdx;		
	
	p->setActive( true );
	p->setVisible( true );
	
	p->position = pos;			
	p->lifeTime = timeToLive;
	
	//default time per frame
	if( timePerFrame == 0 )
		timePerFrame = timeToLive / (float)set->getFrameNumber();
	
	p->immediateAnimation( set, timePerFrame );
	
	//make screensize
	Vector size;
	gameState->getViewport()->makeScreenSize(size, 
											 set->getFrame(0)->getWidth(), 
											 set->getFrame(0)->getHeight() );				
	p->setSize( size );					
	
	p->removeTimedEvent();
	
	return p;			
}
