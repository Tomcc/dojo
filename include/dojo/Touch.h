#ifndef Touch_h__
#define Touch_h__

#include "dojo_common_header.h"

#include "Vector.h"

namespace Dojo
{
	///The Touch class represents a single tap/touch/click on the screen, and allows to trace the event from a frame to the other
	class Touch
	{
	public:

		int ID;
		Vector point, speed;

		///first frame is only set in the first frame where the touch has appeared
		int firstFrame;

		Touch( int _ID, const Vector& _point ) :
			ID( _ID ),
			point( _point ),
			speed( Vector::ZERO ),
			firstFrame( 2 ) //this is 2 because InputSystem::poll is called just after Touch creation - so it has to actually skip a frame
		{

		}

		///internal - called to notice the Touch that it has existed for another frame
		void _update()
		{
			if( firstFrame )
				firstFrame -= 1;
		}
	};
}
#endif // Touch_h__
