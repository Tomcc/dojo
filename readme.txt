===LICENSE===
incoming...

===BUILDING===

so, here's the part for which you opened this file in the first place:

*IOS*

Just add to your project any file from 
-include
-src
-IOS 

folders, as IOS won't support dynamic linking nor custom frameworks.

*Windows*

Requirements
-FreeImage
-POCO C++
-OpenAL1.1 + freeAlut
-glew
-OIS Input Library
	-Directx SDK

The VC++ project looks for the needed files under "dojo/dependencies/include" and "dojo/dependencies/lib". 

The easiest way to build Dojo is to download a precompiled dependency pack from the downloads section on bitbucket:
https://bitbucket.org/tommo89/dojo/downloads

Then place it under dojo/dependencies, open your .sln version and hit rebuild.

*More systems to come*

===USING DOJO===

The code first:

	Platform* platform = Platform::createNativePlatform();

	platform->setGame( new NinjaGame() );

	platform->initialise();

	platform->loop( 1.f/60.f );

The first thing to create to startup Dojo is a Platform instance:

	Platform::createNativePlatform();

it will take care of selecting the correct implementation for your system, and is usually done in the main function (yeah, even on IOS).

Then, you create some Dojo::Game subclass and pass it to the platform to be executed.

	initialise() 
then performs window creation, and whatever the system needs.

	loop() 
starts your main loop, with a specified minimum frame time; 1/60 means that the framerate is capped at 60 fps.

Note: loop() internally calls a step() method, that *could* as well be called in your own main loop, while ideally everything should fit well enough in your Game::onLoop() method.

Other general directions:

Game, GameState, and Sprite are meant to be subclasses.
Your gameplay should fit in the Event Methods, (pure) virtual methods that are called by dojo when something happens, for example:

	onBegin()
	onLoop()
	onButtonClicked()
	onKeyPressed()

The typical flow of a game could be:

-game's onBegin() 
setups globals and starts up a level
Example:

	MyGame::onBegin()
	{
		setState( new MyLevel( this ) );
	}

-level's onBegin()
every setState triggers onBegin on the state being activated;
now it's your first level.

You could want to load resources
	loadResources( "data/something" );

Create a viewport with a background and a nice fade

	camera = new Viewport( 
this, 
Vector::ZERO, 
size,
 Color( 0, 0, 0, 1 ), 
480,
 320 );

	camera->setBackgroundSprite( "background" );

	

	camera->getFader()->color = Color( 0,0,0,1 );
	
	camera->getFader()->startFade( 1, 0, 2 );

	addObject( camera );
	
	

	Platform::getSingleton()->getRender()->setViewport( camera );

And then add any Renderable  (such as Sprite, AnimatedQuad, Font and Model ) to be rendered in a given layer, where lesser is farthest

	addObject( renderable, layer, clickable );

Runtime:
check Game's onLoop(), GameState onLoop and GameState::updateObjects()  ;)
