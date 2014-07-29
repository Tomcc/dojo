#include "stdafx.h"

#include "Sprite.h"

#include "Viewport.h"
#include "FrameSet.h"
#include "GameState.h"

using namespace Dojo;

Sprite::Sprite( Object* parent, const Vector& pos, const String& defaultAnimName, float tpf, bool pp ) :
AnimatedQuad( parent, pos ),
mAnimationIdx( -1 )
{	
	pixelPerfect = pp;

	//get rid of default animation
	SAFE_DELETE( animation );
	
	if( defaultAnimName.size() )
	{		
		setAnimation( registerAnimation( defaultAnimName, tpf ) );
		
		//set convenient size to fit the first frame			
		_updateScreenSize();

		setSize( screenSize );	
	}
}

Sprite::~Sprite() {
	for (int i = 0; i < animations.size(); ++i)
		SAFE_DELETE(animations.at(i));

	//frames have to be released manually from the group!
}

void Sprite::reset() {
	AnimatedQuad::reset();

	if (animations.size())
		setAnimation(0);
}

int Sprite::registerAnimation(FrameSet* set, float timePerFrame /*= -1 */) {
	DEBUG_ASSERT(set != nullptr, "registering a null frameset");

	if (timePerFrame < 0)
		timePerFrame = set->getPreferredAnimationTime();

	DEBUG_ASSERT(timePerFrame >= 0, "the time per frame of an animation can't be negative");

	Animation* a = new Animation(set, timePerFrame);

	animations.add(a);

	//if no current animation, set this as default
	if (mAnimationIdx == -1)
		setAnimation(0);

	return animations.size() - 1;
}

int Sprite::registerAnimation( const String& base, float timePerFrame )
{
	FrameSet* set = gameState->getFrameSet( base );

	DEBUG_ASSERT_INFO( set != nullptr, "The FrameSet to be registered could not be found", "name = " + base );

	return registerAnimation( set, timePerFrame );	
}

void Sprite::setAnimation(int i) {
	mAnimationIdx = i;

	DEBUG_ASSERT(mAnimationIdx >= 0, "negative animation index");
	DEBUG_ASSERT(animations.size() > mAnimationIdx, "OOB animation index");

	if (animation)
		animation->_unset();

	animation = animations.at(mAnimationIdx);

	_setTexture(animation->getCurrentFrame());

	_updateScreenSize();
}
