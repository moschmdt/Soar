//
//  DebuggerTest.hpp
//  Soar-UnitTests
//
//  Tests for SpawnDebugger and KillDebugger functionality
//

#ifndef DebuggerTest_hpp
#define DebuggerTest_hpp

#include "FunctionalTestHarness.hpp"

class DebuggerTest : public FunctionalTestHarness
{
public:
    TEST_CATEGORY(DebuggerTest);

    // Test spawning and killing the Qt debugger
    TEST(testSpawnAndKillDebugger, -1)
    void testSpawnAndKillDebugger();

    // Test that spawning twice fails
    TEST(testSpawnDebuggerTwice, -1)
    void testSpawnDebuggerTwice();

    // Test spawning with explicit path (should fail with invalid path)
    TEST(testSpawnDebuggerInvalidPath, -1)
    void testSpawnDebuggerInvalidPath();

    // Test killing non-existent debugger
    TEST(testKillNonExistentDebugger, -1)
    void testKillNonExistentDebugger();
};

#endif /* DebuggerTest_hpp */
