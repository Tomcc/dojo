//
//  Tests.h
//  Dice
//
//  Created by Tommaso Checchi on 6/25/11.
//  Copyright 2011 none. All rights reserved.
//

#ifndef Dice_Tests_h
#define Dice_Tests_h

#include "dojo_common_header.h"

namespace Dojo {
	
	///Frameworktests is Dojo's test suite, and can't be extended
	/**
	To enable testing, pass a table with "runTests = 1" to the platform.
	*/
	class FrameworkTests
	{
	public:
		
		FrameworkTests( std::ostream& outStream = std::cout ) :
		out( outStream )
		{
			
		}
		
		///tests static objects such as Strings, Vectors, Colors for correctness
		void staticTest();
		
		///various initialization tests, such as Table parsing, resource loading and scene setup
		void initTests();
		
		///tests that can only be executed at runtime, such as state machines and gamestates.
		/**
		 Also performs teardown tests on the created objects.
		*/
		void runtimeTests();
		
		
	protected:
		
		std::ostream& out;
	};
}

#endif
