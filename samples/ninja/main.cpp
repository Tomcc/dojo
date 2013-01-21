#include <dojo.h>

#include <MyGame.h>

using namespace Dojo;

int main( int argc, char** argv )
{
	Platform* platform = Platform::create();

	platform->run( new MyGame() );
	return 0;
}