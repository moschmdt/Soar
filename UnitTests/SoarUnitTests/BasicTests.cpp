//
//  BasicTests.cpp
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/17/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "BasicTests.hpp"
#include <sstream>

void BasicTests::testBasicElaborationAndMatch()
{
	runTest("testBasicElaborationAndMatch", 0);
}

void BasicTests::testInitialState()
{
	runTest("testInitialState", 0);
}

void BasicTests::testDollarSyntax()
{
	// Set up the test
	runTestSetup("testDollarSyntax");
	
	// Capture output from print commands
	std::string output;
	
	// Get the printed rules and check their format
	std::string shorthandRule = agent->ExecuteCommandLine("print test-shorthand");
	std::string longhandRule = agent->ExecuteCommandLine("print test-longhand"); 
	std::string complexRule = agent->ExecuteCommandLine("print test-complex-conjunctive");
	
	// Verify the shorthand rule contains "$ <" (shorthand syntax)
	assertTrue_msg("Shorthand rule should contain '$ <'", 
		shorthandRule.find("$ <") != std::string::npos);
	
	// Verify the longhand rule ALSO gets shortened to "$ <" (smart printing)
	assertTrue_msg("Longhand rule should be shortened to contain '$ <'",
		longhandRule.find("$ <") != std::string::npos);
		
	// Verify the complex rule contains both "$$ <" and "< 5" (can't be shortened)
	assertTrue_msg("Complex rule should contain '$$ <'",
		complexRule.find("$$ <") != std::string::npos);
	assertTrue_msg("Complex rule should contain '< 5'",
		complexRule.find("< 5") != std::string::npos);
	
	// Verify the longhand rule does NOT contain the longhand syntax (it got shortened)
	assertTrue_msg("Longhand rule should NOT contain '{<y> $$ <y>}' (should be shortened)",
		longhandRule.find("{<y> $$ <y>}") == std::string::npos);
}
