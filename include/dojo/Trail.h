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

		bool autoUpdate;

		Trail( GameState* l, const Vector& pos, const String& texName = String::EMPTY );

		virtual ~Trail();

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

		///adds the two ends of a segment to the trail, IN WORLD SPACE!
		void addSegment( const Vector& end1, const Vector& end2 );

		void retesselate( const Vector& startpoint );

		virtual void onAction( float dt );

	protected:

		float updateInterval;
		Vector offset;
		uint maxSegments;

		Dojo::Array<Vector> points;

		float elapsedTime;

		void _retesselate();

	private:
	};
}
#endif // Trail_h__