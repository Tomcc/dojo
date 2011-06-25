//
//  Tests.h
//
//  Created by Tommaso Checchi on 6/24/11.
//  Copyright 2011 none. All rights reserved.
//

#ifndef TestUnit_h__
#define TestUnit_h__

#include "dojo/dojo_common_header.h"

#include <dojo/Timer.h>

namespace Dojo
{		
	class TestUnit
	{
	public:
		
		TestUnit( const std::string& testName, std::ostream& out = std::cout, bool displayPassed = true ) :
		total( 0 ),
		failed( 0 ),
		timeTotal( 0 ),
		name( testName ),
		outStream( out ),
		outputPassed( displayPassed )
		{
			DEBUG_ASSERT( name.size() );
			
			outStream << "RUNNING " << name << "... ";
		}
		
		~TestUnit()
		{
			report();
		}
		
		inline std::ostream& test( bool condition )
		{
			double timeSinceLastTest = testTimer.getElapsedTime();
			timeTotal += timeSinceLastTest;
			
			++total;
						
			if( !condition )
			{
				++failed;				
				outStream << std::endl << "[" << total << "] " << " FAIL: ";				
			}
			else if( outputPassed )
				outStream << std::endl << "[" << total << "] " << " OK\t\t" << timeSinceLastTest*1000.0 << " ms";
			
			testTimer.reset();
			
			return condition ? successStream : outStream;
		}
		
		inline std::ostream& testFalse( bool condition )
		{
			return test( !condition );
		}
		
		inline void ensure( bool condition )
		{
			test( condition );
			
			assert( condition );
		}
		
		void report()
		{			
			if( failed )
				outStream << std::endl << "FAILED " << name << ": " << failed << " fail on " << total << " tests";
			else
			{
				if( outputPassed )			//same line if 
					outStream << std::endl;
				
				outStream << "OK " << total << " tests\t" << timeTotal*1000.0 << " ms";
			}
			outStream << std::endl << std::endl;
		}
		
	protected:
				
		uint total, failed;
		double timeTotal;
		
		bool outputPassed;
		
		Timer testTimer;
		
		std::string name;
		std::stringstream successStream;
		std::ostream& outStream;
	};
}

#endif
