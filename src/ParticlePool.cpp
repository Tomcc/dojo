#include "stdafx.h"

#include "ParticlePool.h"

#include "Viewport.h"
#include "Platform.h"
#include "Renderer.h"
#include "GameState.h"
#include "FrameSet.h"
#include "Texture.h"

using namespace Dojo;

ParticlePool::~ParticlePool()
{
	for( int i = 0; i < pool.size(); ++i )
	{
		
		Platform::singleton().getRenderer().removeRenderable( pool.at(i) );
		
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
		
		Platform::singleton().getRenderer().addRenderable( p, destLayer );
	}
	else
	{				
		//wake up
		p->onReset();
	}
	
	++firstIdleIdx;		
	
	p->setActive( true );
	p->setVisible( true );
	
    DEBUG_TODO;
	//p->worldPosition = pos;			
    
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

Particle* ParticlePool::_getParticleImpl()
{
	return new Particle( this, gameState, pool.size() );
}