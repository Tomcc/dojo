#include "stdafx.h"

#include "ParticlePool.h"

#include "Viewport.h"
#include "Platform.h"
#include "Render.h"
#include "GameState.h"

using namespace Dojo;

ParticlePool::~ParticlePool()
{
	for( uint i = 0; i < pool.size(); ++i )
	{
		
		Platform::getSingleton()->getRender()->removeRenderable( pool.at(i) );
		
		SAFE_DELETE( pool.at(i) );
	}
}

Particle* ParticlePool::getParticle( const Vector& pos, float timeToLive, FrameSet* set, float timePerFrame )
{
	Particle* p = _getUnused();
	
	if( !p ) //no idle
	{
		p = _getParticleImpl();

		pool.add( p );
		
		Platform::getSingleton()->getRender()->addRenderable( p, destLayer );
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
