//
//  BasicTests.hpp
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/17/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef BasicTests_cpp
#define BasicTests_cpp

#include "FunctionalTestHarness.hpp"

class BasicTests : public FunctionalTestHarness
{
public:
	TEST_CATEGORY(BasicTests);
	
	TEST(testBasicElaborationAndMatch, -1)
	void testBasicElaborationAndMatch();
	
	TEST(testInitialState, -1)
	void testInitialState();
	
	TEST(testDollarSyntax, -1)
	void testDollarSyntax();
};

#endif /* BasicTests_cpp */
