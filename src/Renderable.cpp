#include "stdafx.h"

#include "Renderable.h"

#include "Game.h"
#include "Viewport.h"
#include "Mesh.h"
#include "GameState.h"

using namespace Dojo;

Renderable::Renderable( Object* parent, const Vector& pos, Mesh* m ) :
	Object( parent, pos, Vector::ONE ),
	visible( true ),
	layer( INT_MIN ),
	renderingOrder(0),
	currentFadeTime(0)
{
	reset();

	mesh = m;
}

Renderable::Renderable( Object* parent, const Vector& pos, const String& meshName ) :
	Object( parent, pos, Vector::ONE ),
	visible( true ),
	layer(0),
	renderingOrder(0),
	currentFadeTime(0)
{
	reset();

	if( meshName.size() )
	{
		setMesh( getGameState()->getMesh( meshName ) );
		
		DEBUG_ASSERT_INFO( getMesh(), "Tried to create a Renderable but the mesh wasn't found", "name = " + meshName );
	}
}

Renderable::~Renderable()
{
	
}

void Renderable::reset() {
	Object::reset();

	visible = true;

	color.r = color.g = color.b = color.a = 1.f;
	fading = false;
	scale.x = scale.y = 1;
}

void Renderable::startFade(const Color& start, const Color& end, float duration) {
	DEBUG_ASSERT(duration > 0, "The duration of a fade must be greater than 0");

	fadeStartColor = start;
	fadeEndColor = end;

	color = start;

	currentFadeTime = 0;

	fadeEndTime = duration;

	fading = true;

	setVisible(true);
}

void Renderable::startFade(float startAlpha, float endAlpha, float duration) {
	color.a = startAlpha;

	Color end = color;
	end.a = endAlpha;

	startFade(color, end, duration);
}

void Renderable::onAction( float dt )
{
	Object::onAction( dt );

	if( mesh )
		_updateWorldAABB( mesh->getMin(), mesh->getMax() );
	
	advanceFade(dt);
}

bool Renderable::canBeRendered() const {
	return isVisible() && mesh && mesh->isLoaded() && mesh->getVertexCount() > 0;
}

void Renderable::stopFade() {
	fading = false;
}

void Renderable::advanceFade(float dt) {
	if (fading) //fade is scheduled
	{
		float fade = currentFadeTime / fadeEndTime;
		float invf = 1.f - fade;

		color.r = fadeEndColor.r*fade + invf*fadeStartColor.r;
		color.g = fadeEndColor.g*fade + invf*fadeStartColor.g;
		color.b = fadeEndColor.b*fade + invf*fadeStartColor.b;
		color.a = fadeEndColor.a*fade + invf*fadeStartColor.a;

		if (currentFadeTime > fadeEndTime)
		{
			fading = false;

			if (fadeEndColor.a == 0)
				setVisible(false);
		}

		currentFadeTime += dt;
	}
}

void Renderable::_notifyRenderInfo(Renderer* r, int layerID, int renderIdx) {
	render = r;
	layer = layerID;
	renderingOrder = renderIdx;
}
