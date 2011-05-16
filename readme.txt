===LICENSE===

MIT License. But tell me if you do something cool with Dojo ;)

===BUILDING===

so, here's the part for which you opened this file in the first place:

*** IOS ***

Drop the whole Dojo folder to your project, and select "Create group for any added folder", delete (as in "Remove Reference") the other Platform's folders (Apple/OSX, Linux, Win32), any non-source file (.hg, .sln, etc) and you're good to go.

Code drop-in is indeed ugly, but Apple's policy about dylibs is not crystal clear (and OpenFeint does this, so.)

Warning: On IOS you will need a custom main.mm calling UIApplicationMain, and .xib interface file that links the UIView to Apple/IOS/Application class.
You will need two .xibs in the case of iPhone+iPad non universal App.

Tested on IOS 4.3, XCode4

*** WINDOWS ***

Requirements
-FreeImage					http://freeimage.sourceforge.net/
-Poco C++					http://pocoproject.org/
-OpenAL1.1 + freeAlut				http://connect.creativelabs.com/openal/Downloads/Forms/AllItems.aspx
-glew						http://glew.sourceforge.net/index.html
-OIS Input Library				http://sourceforge.net/projects/wgois/
	-Directx SDK for DirectInput for OIS

The VC++ project looks for the needed files under "dojo/dependencies/include" and "dojo/dependencies/lib". 

The easiest way to build Dojo for Windows is to download a precompiled dependency pack from the downloads section on bitbucket:
https://bitbucket.org/tommo89/dojo/downloads
Then place it under dojo/dependencies, open your .sln version and hit rebuild.

Yes, many of those dependencies are more of an utility to maintain a sane codebase, but if needed some of them (like alit, OIS or Poco could be replaced with native code).

Tested on Windows 7 x64, VC2008

*** MAC OS X ***

Open dojo.xcodeproj, hit build. Done. It will get installed as a Framework into /Library/Frameworks for future use.

Tested on Snow Leopard x64, XCode4

*More systems to come*

===USING DOJO===

*Runtime*

The best way to develop your game using Dojo, is taking advantage of its state/event framework.
Many classes are meant to be inherited, allowing your game code to be called by the engine's events.

You can recognize at a glance what events a class listens to, looking for methods like "onBegin", "onStateLoop", "onButtonClicked".

The interface that rules the game's runtime is StateInterface, an interface that represents a State.
A State can begin (onBegin) can repeat itself (onLoop) can end (onEnd).
Game, GameState and some objects are in fact states.

Every State is also a State Machine by itself, meaning that it will manage other Substates. 
For example, a GameState is a substate owned by a Game.

A State can have "Implicit Substates" when implementing the callbacks "onStateBegin()/Loop()/End()".
This allows to have plenty of substates without creating a swarm of StateInterface classes.
The current substate of a State can be queried using "getCurrentState()" if implicit, and "getCurrentStatePtr()" if explicit.

Also, any Object's subclass that is registered either to a GameState or to a parent Object receives an "onAction" event each frame.

*Rendering*

Dojo's rendering is organized by Layers: objects in layers with a lower index are always drawn behind objects in higher layers, and this also applies to 3D layers.
This is really useful in games where you need a structure like "skybox < level < weapon < 2D hud". 

Each layer has some flags that tell how the Renderables it contains are rendered together.
-projectionOff: disables projection (farther things smaller)
-depthCheck: enables depth check. Depth Check is needed for a correct perspective rendering, but can make things faster in 2D
	it is also useful when managing 3D alpha blending.
-depthClear: enables a layer to clear the ZBuffer before rendering. It is on by default.
	can be useful to "merge" two layers together, for example to ensure that glass is always drawn after the walls.
-lightingOn: enables lighting. Heavy on performance.

Dojo offers a single rendering primitive, Renderable.
Renderable owns a RenderState and can have 1 to 8 textures, and needs to have one Mesh, along with other parameters like alpha blending and visibility.

Sprite is an useful class that extends Renderable for your regular sprite-based animation.

TextArea extends Renderable to display (currently ASCII only) texts using bitmap Fonts (.font + .png)

Particle extends Renderable to offer faster execution and pooling using the ParticlePool class.

Viewport is the "eye" that sees your world; the same object shares the 2D and 3D settings, being able to see all the layers from the same position.
If you have 3D and 2D layers, remember to attach all the 2D objects to your Viewport ;)

*Resources*

Resources are loaded using loadResources() method on ResourceGroup (and incidentally, GameState is a ResourceGroup).
Resources can be FrameSets, Textures, Meshes, Fonts, Tables, SoundSets and SoundBuffers.

The most useful are the "composite resources", that just map to a group of "real resources" such as:
-FrameSet: a frameset is a collection of Textures either created procedurally adding them to the collection, 
	or reading sequentially named .pngs from disk.
	For example, "frame_1.png frame_2.png frame_3.png" are auto-loaded into a FrameSet.
	FrameSets are best used for sprite animations.

-Font: a font is defined in a .font file, that defines character width, height, and ASCII offset.
	It requires a Texture as its bitmap.

-SoundSet: sounds can be easily made random using SoundSets, think for example at collisions sounds for a material. 
	SoundSet maps any filename sequence just like FrameSet.

"Real Resources" are:
-Texture: an image, or a tile of a Texture Atlas.
-Mesh: well, a mesh. A collection of points and triangles. Currently all meshes have
	to be constructed procedurally as there isn't any importer to .dms format 
	(and it WILL get scrapped for something better, sometime).

-SoundBuffer is a sound. Prefer using SoundSet instead, or better SoundSource.

-Table is a "property list tree" that does a whole lot of serialization heavy duties, see the Serializable interface.
	It can store named ints, bools, Vectors, Colors, child Tables and even raw Data.
	Use this to implement your loading/saving needs across platforms.
	See also Platform::load()/save()

Phew. This covers a great deal of Dojo, but there's obviously much more. Feel free to peek in the code to get more insights ;)

										Tommaso Checchi, 2011


