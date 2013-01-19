//android MAIN
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
	    setSize( Vector( 15,10 ) ); //set a level size, in "Units"... what an Unit means is up to you!

	    camera = new Viewport(this, 
			Vector::UNIT_Z,
			getSize(),
			Color::BLUE, 
			70, 0.01, 1000 );

	    addChild( camera ); //add the camera to the scene
	    setViewport( camera ); //tell Dojo we will render from this camera
	} 

	virtual void onEnd()
	{

	}
};

class MyGame : public Game
{
public:
	MyGame() :
	Game( "MyGame", 1024, 768, DO_LANDSCAPE_RIGHT, 1.f/60.f  )
	{

	}

protected:

	virtual void onBegin()
	{
                //load a level as soon as Dojo starts up
		Level* level = new Level( this );

		setState( level );
	}

	virtual void onLoop( float dt )  //frame update event
	{

	}

	virtual void onEnd()      //termination event
	{

	}
};


extern "C" int main( int argc, char** argv )
{
	
    	DEBUG_MESSAGE("Platform::create()");

	Platform* platform = Platform::create();

    	DEBUG_MESSAGE("new MyGame()");

	platform->initialise( new MyGame() );

    	DEBUG_MESSAGE("Platform::run()");

	platform->loop(1.0f/60.0f);

	Platform::shutdownPlatform();

	return 0;
}
