# Soar Qt Debugger Architecture

## Overview

The Soar Qt Debugger is a graphical debugging environment for the Soar cognitive architecture, built using Qt6. It provides a comprehensive interface for managing Soar kernels, agents, and their execution.

## Architecture Hierarchy

```
QApplication (main.cpp)
└── MainWindow (main debugger window)
    ├── SoarDebugger (kernel/agent manager)
    │   ├── sml::Kernel (Soar kernel)
    │   └── QList<SoarAgent*> (managed agents)
    │
    ├── QSplitter (m_centralSplitter - horizontal)
    │   ├── QTreeWidget (m_agentTree - left panel)
    │   │   └── QTreeWidgetItem per agent
    │   │
    │   └── QTabWidget (m_tabWidget - right panel)
    │       └── AgentWindow (one per agent)
    │           ├── QSplitter (m_mainSplitter - horizontal)
    │           │   ├── QSplitter (m_leftSplitter - vertical)
    │           │   │   ├── WorkingMemoryTree
    │           │   │   └── ProductionsWindow
    │           │   │
    │           │   └── QSplitter (m_rightSplitter - vertical)
    │           │       ├── OutputWindow
    │           │       └── CommandWindow
    │
    ├── Menu Bar
    │   ├── File Menu
    │   ├── Agent Menu
    │   ├── View Menu
    │   └── Help Menu
    │
    ├── Toolbars
    │   ├── File Toolbar
    │   └── Agent Toolbar
    │
    └── Status Bar
        ├── Status Label
        ├── Kernel Status
        └── Agent Status
```

## Core Components

### 1. **main.cpp**

Entry point for the application.

-   Creates `QApplication`
-   Sets application metadata (name, version, organization)
-   Initializes `SoarDebugger` and `MainWindow`
-   Starts Qt event loop

### 2. **SoarDebugger** (`SoarDebugger.h/cpp`)

Central manager for Soar kernel and agents.

**Responsibilities:**

-   Manages the Soar SML kernel lifecycle
-   Creates and destroys agents
-   Connects to local or remote kernels
-   Emits signals when kernel/agent state changes

**Key Methods:**

-   `initialize()` - Starts local kernel
-   `shutdown()` - Stops kernel and cleans up agents
-   `createAgent(name)` - Creates new Soar agent
-   `destroyAgent(agent)` - Destroys specific agent
-   `connectToRemoteKernel(host, port)` - Connects to remote kernel
-   `getAllAgents()` - Returns list of all agents

**Signals:**

-   `kernelStarted()` - Emitted when kernel initializes
-   `kernelStopped()` - Emitted when kernel shuts down
-   `agentCreated(SoarAgent*)` - Emitted when agent is created
-   `agentDestroyed(SoarAgent*)` - Emitted when agent is destroyed

### 3. **MainWindow** (`MainWindow.h/cpp`)

Main application window and UI coordinator.

**Components:**

-   **m_agentTree** (`QTreeWidget`) - Left panel showing list of agents
-   **m_tabWidget** (`QTabWidget`) - Right panel with tabs for each agent
-   **m_centralSplitter** (`QSplitter`) - Horizontal splitter between tree and tabs
-   **m_outputWindow** (`OutputWindow`) - Global output window (hidden by default)

**Menus:**

-   **File Menu**: New Agent, Connect to Remote, Disconnect, Exit
-   **Agent Menu**: Run, Stop, Step, Init Soar
-   **View Menu**: Show/hide panels
-   **Help Menu**: About dialogs

**Toolbars:**

-   **File Toolbar**: Quick access to file operations
-   **Agent Toolbar**: Agent control buttons (run, stop, step, init)

**Key Slots:**

-   `newAgent()` - Creates new agent with user-provided name
-   `connectToRemote()` - Shows dialog to connect to remote kernel
-   `runAgent()` - Executes current agent
-   `stopAgent()` - Stops current agent execution
-   `stepAgent()` - Steps current agent by N decisions
-   `initAgent()` - Initializes current agent (init-soar)
-   `onAgentCreated(SoarAgent*)` - Adds agent to tree and creates tab
-   `onAgentDestroyed(SoarAgent*)` - Removes agent from UI
-   `onAgentSelectionChanged()` - Handles agent tree selection
-   `onTabCloseRequested(int)` - Handles tab close events
-   `createDefaultAgent()` - Auto-creates default "soar" agent on startup

**Helper Methods:**

-   `getCurrentAgent()` - Returns currently selected agent from active tab
-   `updateActions()` - Enables/disables actions based on state
-   `updateStatusBar()` - Updates status information

### 4. **SoarAgent** (`SoarAgent.h/cpp`)

Qt wrapper around SML Agent with signals/slots.

**Responsibilities:**

-   Wraps `sml::Agent` with Qt-friendly interface
-   Manages agent callbacks (print, update events)
-   Provides command execution interface
-   Emits signals for UI updates

**Key Methods:**

-   `executeCommand(command)` - Executes Soar CLI command
-   `runStep(count)` - Runs N decision cycles
-   `runTilDecision()` - Runs until decision phase
-   `runTilOutput()` - Runs until output phase
-   `runTilHalt()` - Runs until halt
-   `stop()` - Stops agent execution
-   `initSoar()` - Reinitializes agent
-   `getWorkingMemory()` - Retrieves WM contents
-   `getProductions()` - Retrieves production list
-   `removeCallbacks()` - Cleans up SML callbacks

**Signals:**

-   `outputReceived(QString)` - Emitted when agent produces output
-   `runStarted()` - Emitted when agent starts running
-   `runStopped()` - Emitted when agent stops
-   `workingMemoryChanged()` - Emitted after decision cycle (for auto-refresh)

**Callbacks:**

-   `printHandler()` - Static callback for print events
-   `updateHandler()` - Static callback for update events (after output phase)

### 5. **AgentWindow** (`AgentWindow.h/cpp`)

Per-agent view containing all debugging panels.

**Layout Structure:**

```
m_mainSplitter (Horizontal)
├── m_leftSplitter (Vertical)
│   ├── WorkingMemoryTree (top)
│   └── ProductionsWindow (bottom)
│
└── m_rightSplitter (Vertical)
    ├── OutputWindow (top, 75%)
    └── CommandWindow (bottom, 25%)
```

**Components:**

-   **m_wmTree** (`WorkingMemoryTree`) - Displays working memory
-   **m_productionsWindow** (`ProductionsWindow`) - Shows productions
-   **m_outputWindow** (`OutputWindow`) - Agent output display
-   **m_commandWindow** (`CommandWindow`) - Command line interface

**Key Method:**

-   `agent()` - Returns associated SoarAgent

## Widget Details

### 6. **WorkingMemoryTree** (`WorkingMemoryTree.h/cpp`)

Displays working memory in tree structure.

**Components:**

-   **m_tree** (`QTreeWidget`) - Tree view of WM elements
-   **m_commandEdit** (`QLineEdit`) - Editable print command
-   **m_refreshButton** (`QPushButton`) - Manual refresh trigger
-   **m_expandButton** (`QPushButton`) - Expand all nodes
-   **m_collapseButton** (`QPushButton`) - Collapse all nodes
-   **m_printCommand** (`QString`) - Current print command (default: "print --depth 3 <s>")

**Functionality:**

-   Executes configurable print command to populate tree
-   Auto-refreshes on `workingMemoryChanged()` signal
-   Allows customization of print depth and target
-   Displays WME hierarchy with identifiers, attributes, and values

**Key Slots:**

-   `refresh()` - Refreshes WM display
-   `expandAll()` - Expands all tree nodes
-   `collapseAll()` - Collapses all tree nodes
-   `updateCommand()` - Updates m_printCommand from edit field

**Constructor Connection:**

```cpp
connect(m_agent, &SoarAgent::workingMemoryChanged, this, &WorkingMemoryTree::refresh);
```

### 7. **ProductionsWindow** (`ProductionsWindow.h/cpp`)

Displays and manages Soar productions.

**Components:**

-   **m_productionList** (`QListWidget`) - List of production names
-   **m_productionText** (`QTextEdit`) - Detailed production source
-   **m_filterEdit** (`QLineEdit`) - Filter productions by name

**Functionality:**

-   Lists all loaded productions
-   Shows detailed production text when selected
-   Filters production list based on user input
-   Displays production source code with syntax

**Key Slots:**

-   `refresh()` - Reloads production list
-   `filterProductions(text)` - Filters list by name
-   `onProductionSelected()` - Shows selected production details

### 8. **CommandWindow** (`CommandWindow.h/cpp`)

Command-line interface for Soar commands.

**Components:**

-   **m_commandLine** (`QLineEdit`) - Command input field
-   **m_outputDisplay** (`QTextEdit`) - Command output display
-   **m_clearButton** (`QPushButton`) - Clear output button
-   **m_infoButton** (`QPushButton`) - Opens CLI documentation ("ℹ" button)
-   **m_commandHistory** (`QStringList`) - Command history
-   **m_historyIndex** (`int`) - Current position in history

**Functionality:**

-   Executes arbitrary Soar CLI commands
-   Maintains command history (navigate with Up/Down arrows)
-   Displays command results
-   Links to Soar CLI documentation (https://soar.eecs.umich.edu/reference/cli/)

**Key Slots:**

-   `executeCommand()` - Executes current command (triggered by Enter key)
-   `clearOutput()` - Clears output display
-   `openCliDocumentation()` - Opens browser to CLI docs

**Event Handling:**

-   `eventFilter()` - Intercepts Up/Down keys for history navigation

### 9. **OutputWindow** (`OutputWindow.h/cpp`)

Displays agent output and trace messages.

**Components:**

-   **m_textEdit** (`QTextEdit`) - Output text display
-   **m_clearButton** (`QPushButton`) - Clear output
-   **m_autoScrollCheckbox** (`QCheckBox`) - Enable/disable auto-scroll
-   **m_autoScroll** (`bool`) - Auto-scroll state

**Functionality:**

-   Receives output via `outputReceived()` signal from SoarAgent
-   Auto-scrolls to bottom when new output arrives (if enabled)
-   Displays all agent print events and trace output

**Key Slots:**

-   `appendOutput(text)` - Adds text to output display
-   `clearOutput()` - Clears all output
-   `toggleAutoScroll(enabled)` - Enables/disables auto-scroll

**Constructor Connection:**

```cpp
connect(m_agent, &SoarAgent::outputReceived, this, &OutputWindow::appendOutput);
```

## Signal/Slot Connections

### SoarDebugger → MainWindow

```cpp
kernelStarted()      → onKernelStarted()
kernelStopped()      → onKernelStopped()
agentCreated()       → onAgentCreated()
agentDestroyed()     → onAgentDestroyed()
```

### SoarAgent → WorkingMemoryTree

```cpp
workingMemoryChanged() → refresh()
```

### SoarAgent → OutputWindow

```cpp
outputReceived(QString) → appendOutput(QString)
```

### MainWindow Internal

```cpp
m_agentTree::itemSelectionChanged → onAgentSelectionChanged()
m_tabWidget::tabCloseRequested    → onTabCloseRequested(int)
```

## Data Flow

### Agent Creation Flow

1. User clicks "New Agent" or default agent is created
2. `MainWindow::newAgent()` or `createDefaultAgent()` called
3. `SoarDebugger::createAgent(name)` creates SML agent
4. `SoarDebugger` emits `agentCreated(SoarAgent*)`
5. `MainWindow::onAgentCreated()` receives signal
6. Creates `QTreeWidgetItem` in agent tree
7. Creates `AgentWindow` with all sub-widgets
8. Adds `AgentWindow` as tab in `m_tabWidget`
9. All sub-widgets connect to `SoarAgent` signals

### Command Execution Flow

1. User types command in `CommandWindow::m_commandLine`
2. Presses Enter → `executeCommand()` slot called
3. `SoarAgent::executeCommand(command)` called
4. `sml::Agent::ExecuteCommandLine()` executed
5. If command modifies WM (run, step, etc.), `workingMemoryChanged()` emitted
6. `WorkingMemoryTree::refresh()` receives signal
7. Executes print command via `executeCommand()`
8. Parses output and updates tree display

### Output Display Flow

1. Soar agent produces output (print, trace, etc.)
2. SML calls `SoarAgent::printHandler()` callback
3. Handler emits `outputReceived(QString)` signal
4. `OutputWindow::appendOutput()` receives signal
5. Appends text to `m_textEdit`
6. Auto-scrolls if `m_autoScroll` is true

## Key Design Patterns

### 1. **Model-View Separation**

-   **Model**: `SoarDebugger`, `SoarAgent` (wraps SML)
-   **View**: All Qt widgets (`MainWindow`, `AgentWindow`, etc.)
-   **Controller**: Signal/slot connections coordinate model-view updates

### 2. **Composite Pattern**

-   `AgentWindow` contains multiple sub-widgets
-   Each widget is self-contained and independent
-   All widgets share reference to same `SoarAgent`

### 3. **Observer Pattern**

-   Qt signals/slots implement observer pattern
-   `SoarAgent` notifies observers of state changes
-   Widgets subscribe to relevant signals

### 4. **Wrapper Pattern**

-   `SoarAgent` wraps `sml::Agent` with Qt interface
-   Converts C++ SML callbacks to Qt signals
-   Provides Qt-friendly API over SML

## Initialization Sequence

```
1. main() creates QApplication
2. main() creates SoarDebugger
3. SoarDebugger::initialize()
   - Creates sml::Kernel
   - Emits kernelStarted()
4. main() creates MainWindow(debugger)
5. MainWindow constructor:
   - createCentralWidget() (tree + tabs)
   - createMenus()
   - createToolBars()
   - createStatusBar()
   - Connect to SoarDebugger signals
   - Schedule createDefaultAgent() via QTimer
6. MainWindow::createDefaultAgent() (after 100ms)
   - SoarDebugger::createAgent("soar")
   - Emits agentCreated(SoarAgent*)
7. MainWindow::onAgentCreated()
   - Create QTreeWidgetItem
   - Create AgentWindow(agent)
   - AgentWindow creates all sub-widgets
   - Each widget connects to agent signals
8. Application ready for interaction
```

## Shutdown Sequence

```
1. User closes MainWindow or selects Exit
2. MainWindow::closeEvent()
3. SoarDebugger::shutdown()
4. For each agent:
   - SoarAgent::removeCallbacks()
     - UnregisterForPrintEvent()
     - UnregisterForUpdateEvent()
   - sml::Kernel::DestroyAgent()
   - Emit agentDestroyed()
5. sml::Kernel::Shutdown()
6. Emit kernelStopped()
7. QApplication exits
```

## File Organization

```
QtDebugger/
├── CMakeLists.txt                 # Build configuration
├── src/
│   ├── main.cpp                   # Application entry point
│   ├── SoarDebugger.h/cpp        # Kernel/agent manager
│   ├── SoarAgent.h/cpp           # Agent wrapper
│   ├── MainWindow.h/cpp          # Main window
│   ├── AgentWindow.h/cpp         # Per-agent view
│   ├── WorkingMemoryTree.h/cpp   # WM tree widget
│   ├── ProductionsWindow.h/cpp   # Productions widget
│   ├── CommandWindow.h/cpp       # CLI widget
│   └── OutputWindow.h/cpp        # Output display widget
└── debugger.instructions.md       # This file
```

## Extension Points

### Adding New Widgets

1. Create widget class inheriting from `QWidget`
2. Accept `SoarAgent*` in constructor
3. Connect to relevant `SoarAgent` signals
4. Add to `AgentWindow::createLayout()`

### Adding New Agent Commands

1. Add method to `SoarAgent` (e.g., `loadFile()`)
2. Call `sml::Agent` method internally
3. Emit relevant signals if state changes
4. Add UI action/button in `MainWindow` or `AgentWindow`

### Adding New Callbacks

1. Add callback ID member to `SoarAgent`
2. Register callback in `setupCallbacks()`
3. Create static handler method
4. Emit Qt signal from handler
5. Unregister in `removeCallbacks()`

## Critical Implementation Details

### Memory Management

-   All widgets use Qt parent-child ownership
-   Parent widgets automatically delete children
-   `SoarAgent` objects managed by `SoarDebugger`
-   Raw pointers used with Qt ownership system

### Thread Safety

-   All UI operations run on main Qt thread
-   SML callbacks execute on Soar kernel thread
-   Qt signals/slots ensure thread-safe communication
-   No explicit mutex locking required

### Auto-Refresh Mechanism

-   `SoarAgent` registers for `smlEVENT_AFTER_ALL_OUTPUT_PHASES`
-   Emits `workingMemoryChanged()` after each decision cycle
-   `WorkingMemoryTree` refreshes automatically
-   Also triggered after `executeCommand()` for run/step commands

### Customizable WM Display

-   `WorkingMemoryTree::m_printCommand` holds current print command
-   Default: `"print --depth 3 <s>"` (current state)
-   User can edit via `QLineEdit` at top of widget
-   Pressing Enter triggers `refresh()` with new command

## Common Workflows

### Creating and Running an Agent

1. Click "New Agent" or use default "soar" agent
2. Switch to agent's tab (auto-selected on creation)
3. Load productions via command: `source file.soar`
4. Click "Run" or use command: `run 5`
5. View output in OutputWindow
6. Inspect WM in WorkingMemoryTree (auto-refreshes)

### Debugging Productions

1. Select agent in tree
2. View productions in ProductionsWindow
3. Filter by name using search box
4. Click production to see full source
5. Use command window: `matches production-name`
6. Check WM tree for current state

### Remote Debugging

1. File → Connect to Remote
2. Enter host and port
3. Connect to running kernel
4. Create or attach to agents
5. Debug as normal
6. File → Disconnect when done
