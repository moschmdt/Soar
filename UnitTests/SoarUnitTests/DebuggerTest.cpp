//
//  DebuggerTest.cpp
//  Soar-UnitTests
//
//  Tests for SpawnDebugger and KillDebugger functionality
//

#include "DebuggerTest.hpp"

#include <thread>
#include <chrono>

// Helper function to sleep cross-platform
void sleep_ms(int milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void DebuggerTest::testSpawnAndKillDebugger()
{
    // Note: This test requires the SoarDebugger executable to be available
    // in one of the standard search locations

    // Try to spawn the debugger
    bool spawnResult = agent->SpawnDebugger();

    // Test FAILS if debugger is not found
    assertTrue_msg("SpawnDebugger should return true - SoarDebugger executable must be available in "
                   "current directory, $SOAR_HOME, library path, build/*/QtDebugger/, or working directory.",
                   spawnResult);

    // Give the debugger a moment to start up
    sleep_ms(500);

    // Verify we can interact with the agent while debugger is running
    agent->ExecuteCommandLine("run 5");

    // Give debugger a moment to process any events
    sleep_ms(500);

    // Now kill the debugger
    bool killResult = agent->KillDebugger();
    assertTrue_msg("KillDebugger should return true", killResult);

    // Give the process time to terminate
    sleep_ms(500);
}

void DebuggerTest::testSpawnDebuggerTwice()
{
    // Try to spawn the debugger
    bool firstSpawn = agent->SpawnDebugger();

    // Test FAILS if debugger is not found
    assertTrue_msg("First SpawnDebugger should return true - SoarDebugger executable must be available.",
                   firstSpawn);

    // Give the debugger a moment to start
    sleep_ms(500);

    // Try to spawn again - should fail because one is already running
    bool secondSpawn = agent->SpawnDebugger();
    assertFalse_msg("Second SpawnDebugger call should fail when debugger is already running", secondSpawn);

    // Clean up
    agent->KillDebugger();
    sleep_ms(500);
}

void DebuggerTest::testSpawnDebuggerInvalidPath()
{
    // Try to spawn with an invalid path
    bool result = agent->SpawnDebugger(-1, "/this/path/does/not/exist/debugger");

    assertFalse_msg("SpawnDebugger should return false with invalid path", result);
}

void DebuggerTest::testKillNonExistentDebugger()
{
    // Try to kill debugger when none is running (with ignoreNonExistent=false)
    bool result = agent->KillDebugger(false);

    assertFalse_msg("KillDebugger should return false when no debugger is running", result);

    // Try again with ignoreNonExistent=true (should still return false but not print error)
    result = agent->KillDebugger(true);
    assertFalse_msg("KillDebugger should return false even with ignoreNonExistent=true", result);
}
