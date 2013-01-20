
#include <dojo.h>

#include "Level.h"

using namespace Dojo;

class MyGame : public Game
{
public:
	MyGame() :
	Game( "MyGame", 1024, 768, DO_LANDSCAPE_RIGHT, 1.f/60.f  )
	{

	}

protected:
	virtual void onBegin()   //initialization event
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