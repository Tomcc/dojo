#include "Game.h"

#include "Object.h"
#include "GameState.h"
#include "Sprite.h"
#include "dojomath.h"
#include "Platform.h"

using namespace Dojo;

Game::Game(utf::string_view gamename, int w, int h, Orientation r, float nativedt, float maximumdt, bool requireDepthBuffer) :
	name(gamename.copy()),
	nativeWidth(w),
	nativeHeight(h),
	nativeOrientation(r),
	mNativeFrameLength(nativedt),
	mMaxFrameLength(maximumdt),
	mRequireDepthBuffer(requireDepthBuffer) {
	DEBUG_ASSERT( name.not_empty(), "A Game must have a non-empty name" );
	DEBUG_ASSERT( w >= 0, "negative width" );
	DEBUG_ASSERT( h >= 0, "negative height" );
	DEBUG_ASSERT( mNativeFrameLength > 0, "the Native Frame Lenght must be greater than 0 seconds" );
	DEBUG_ASSERT( mMaxFrameLength >= mNativeFrameLength, "the native frame lenght must be less or equal than the max frame length" );

	auto& p = Platform::singleton();

	if (w == 0) {
		nativeWidth = p.getScreenWidth();
	}

	if (h == 0) {
		nativeHeight = p.getScreenHeight();
	}

	p.addApplicationListener(self); //always listen to the app
}

Game::~Game() {

}
