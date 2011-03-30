#ifndef Trail_h__
#define Trail_h__

#include "dojo_common_header.h"

#include <queue>

#include "Renderable.h"

namespace Dojo
{
	///A Trail is an object that leaves a polygon trail behind itself
	/**
		it updates when a given amount of time has passed, and the points are added until the buffer is full.
		each point is added with the current color of the trail.
	*/
	class Trail : public Renderable
	{
	public:

		Trail( GameState* l, const Vector& pos, const std::string& texName = "" );

		inline void setMaxSegments( uint segs )
		{
			DEBUG_ASSERT( segs > 0 );

			maxSegments = segs;
		}

		inline void setUpdateInterval(float dt)
		{
			DEBUG_ASSERT( dt >= 0 );

			updateInterval = dt;
		}

		///set the offset of the couple of emitted point in LOCAL SPACE
		inline void setOffset( const Vector& trailOffset )
		{
			offset = trailOffset;
		}

		virtual void action( float dt );

	protected:

		float updateInterval;
		Vector offset;
		uint maxSegments;

		std::queue<Vector> points;

		float elapsedTime;

		void _retesselate();

	private:
	};
}
#endif // Trail_h__