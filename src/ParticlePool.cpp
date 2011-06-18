#include "stdafx.h"

#include "dojo/ParticlePool.h"

#include "dojo/Viewport.h"

using namespace Dojo;

Particle* ParticlePool::getParticle( const Vector& pos, float timeToLive, FrameSet* set, float timePerFrame )
{
	Particle* p = _getUnused();
	
	if( !p ) //no idle
	{
		p = _getParticleImpl();

		pool.add( p );
		
		_registerParticle( p );
	}
	else
	{				
		//wake up
		p->onReset();
	}
	
	++firstIdleIdx;		
	
	p->setActive( true );
	p->setVisible( true );
	
	p->worldPosition = pos;			
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
