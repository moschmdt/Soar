/**
 * @file SpawnDebugger.cpp
 * @brief Simple test program to spawn the Soar Qt debugger
 *
 * This program creates a Soar kernel and agent, then spawns the debugger.
 * Useful for testing the SpawnDebugger functionality.
 *
 * Usage:
 *   ./SpawnDebugger [debugger_path]
 *
 * Examples:
 *   ./SpawnDebugger                                    # Use system-installed debugger
 *   ./SpawnDebugger /path/to/SoarDebugger              # Use explicit path
 *   ./SpawnDebugger ../../build/Debug/QtDebugger/SoarDebugger  # Use build directory
 */

#include "sml_Client.h"
#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char *argv[])
{
    std::cout << "=== Soar Debugger Spawn Test ===" << std::endl;

    // Parse command line arguments
    const char *debuggerPath = nullptr;
    if (argc > 1)
    {
        debuggerPath = argv[1];
        std::cout << "Using explicit debugger path: " << debuggerPath << std::endl;
    }
    else
    {
        std::cout << "No path provided - will use system-installed debugger from PATH" << std::endl;
    }

    // Create a kernel in a new thread
    std::cout << "\nCreating Soar kernel..." << std::endl;
    sml::Kernel *kernel = sml::Kernel::CreateKernelInNewThread();

    if (!kernel)
    {
        std::cerr << "ERROR: Failed to create kernel" << std::endl;
        return 1;
    }

    std::cout << "Kernel created successfully" << std::endl;
    std::cout << "Kernel is listening on port: " << kernel->GetListenerPort() << std::endl;

    // Create an agent
    std::cout << "\nCreating agent 'test-agent'..." << std::endl;
    sml::Agent *agent = kernel->CreateAgent("test-agent");

    if (!agent)
    {
        std::cerr << "ERROR: Failed to create agent" << std::endl;
        kernel->Shutdown();
        delete kernel;
        return 1;
    }

    std::cout << "Agent created successfully" << std::endl;

    // Spawn the debugger
    std::cout << "\n=== Spawning Debugger ===" << std::endl;
    bool success = agent->SpawnDebugger(-1, debuggerPath);

    if (success)
    {
        std::cout << "\n✓ SUCCESS: Debugger spawned successfully!" << std::endl;
        std::cout << "\nThe debugger should now be running and connected to the agent." << std::endl;
        std::cout << "Press Ctrl+C to exit this program (the debugger will be terminated)." << std::endl;

        // Keep the program running so the debugger stays connected
        std::cout << "\nWaiting..." << std::endl;

        // Wait for user input or run for a while
        for (int i = 0; i < 60; i++)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "." << std::flush;

            // Check every 10 seconds
            if (i % 10 == 9)
            {
                std::cout << " (" << (i + 1) << "s)" << std::endl;
            }
        }

        std::cout << "\n\nTimeout reached (60s)." << std::endl;
    }
    else
    {
        std::cerr << "\n✗ FAILED: Could not spawn debugger" << std::endl;
        std::cerr << "\nPossible reasons:" << std::endl;
        std::cerr << "  1. Debugger not found in PATH (try providing explicit path)" << std::endl;
        std::cerr << "  2. Invalid debugger path provided" << std::endl;
        std::cerr << "  3. Debugger already running" << std::endl;
        std::cerr << "\nTry running with an explicit path:" << std::endl;
        std::cerr << "  " << argv[0] << " /path/to/SoarDebugger" << std::endl;
    }

    // Clean up
    std::cout << "\n=== Cleaning Up ===" << std::endl;
    std::cout << "Killing debugger (if running)..." << std::endl;
    agent->KillDebugger(true);

    std::cout << "Shutting down kernel..." << std::endl;
    kernel->Shutdown();
    delete kernel;

    std::cout << "Done." << std::endl;

    return success ? 0 : 1;
}
