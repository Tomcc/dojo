
#include <dojo.h>

using namespace Dojo;

class Level : public GameState 
{
public:
	enum Layers //this enum is used to set the objects' rendering order
	{
		LL_ACTORS,
		LL_OVERLAY
	};

	Level( Game* game ) : 
	GameState( game )
	{

	}
protected:
	virtual void onBegin()
	{
		addPrefabMeshes(); //this loads prefab stuff, always useful
		addFolder( "data" ); //this assumes that "data" is "(resource dir)/data"
		loadResources(); //this loads all the resources in the group, both the prefabs and data files!

	    setSize( Vector( 15,10 ) ); //set a level size, in "Units"... what an Unit means is up to you!

	    camera = new Viewport(this, 
			Vector::UNIT_Z,
			getSize(),
			Color::BLUE, 
			70, 0.01, 1000 );

	    addChild( camera ); //add the camera to the scene
	    setViewport( camera ); //tell Dojo we will render from this camera

	    Sprite* ninja = new Sprite( this, Vector::ZERO, "ninjaRun", 0.1 );
		addChild( ninja, LL_ACTORS );
	}

	virtual void onEnd()
	{

	}
};