#include "MainWindow.h"
#include "AgentWindow.h"
#include "OutputWindow.h"
#include "SoarAgent.h"
#include "SoarDebugger.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QSplitter>
#include <QStatusBar>
#include <QTabWidget>
#include <QTimer>
#include <QToolBar>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

MainWindow::MainWindow(SoarDebugger *debugger, QWidget *parent)
    : QMainWindow(parent), m_debugger(debugger), m_centralSplitter(nullptr),
      m_agentTree(nullptr), m_tabWidget(nullptr), m_outputWindow(nullptr),
      m_statusLabel(nullptr), m_kernelStatusLabel(nullptr),
      m_agentStatusLabel(nullptr) {
  setWindowTitle("Soar Debugger");
  setWindowIcon(QIcon(":/icons/soar.png")); // We'll add an icon later

  // Set minimum size
  setMinimumSize(800, 600);

  // Create UI components
  createCentralWidget();
  createMenus();
  createToolBars();
  createStatusBar();

  // Connect to debugger signals
  connect(m_debugger, &SoarDebugger::kernelStarted, this,
          &MainWindow::onKernelStarted);
  connect(m_debugger, &SoarDebugger::kernelStopped, this,
          &MainWindow::onKernelStopped);
  connect(m_debugger, &SoarDebugger::agentCreated, this,
          &MainWindow::onAgentCreated);
  connect(m_debugger, &SoarDebugger::agentDestroyed, this,
          &MainWindow::onAgentDestroyed);

  // Initial state
  updateActions();
  updateStatusBar();

  // Restore settings
  QSettings settings;
  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("windowState").toByteArray());

  // Create default agent if started normally (not from source code)
  QTimer::singleShot(100, this, &MainWindow::createDefaultAgent);
}

MainWindow::~MainWindow() {
  // Save settings
  QSettings settings;
  settings.setValue("geometry", saveGeometry());
  settings.setValue("windowState", saveState());
}

void MainWindow::closeEvent(QCloseEvent *event) {
  // Shutdown cleanly without confirmation
  m_debugger->shutdown();
  event->accept();
}

void MainWindow::createCentralWidget() {
  // Create main splitter
  m_centralSplitter = new QSplitter(Qt::Horizontal, this);
  setCentralWidget(m_centralSplitter);

  // Create agent tree
  m_agentTree = new QTreeWidget();
  m_agentTree->setHeaderLabel("Agents");
  m_agentTree->setMaximumWidth(200);
  m_agentTree->setMinimumWidth(150);
  connect(m_agentTree, &QTreeWidget::itemSelectionChanged, this,
          &MainWindow::onAgentSelectionChanged);

  // Create tab widget for agent windows
  m_tabWidget = new QTabWidget();
  m_tabWidget->setTabsClosable(true);
  m_tabWidget->setMovable(true);
  connect(m_tabWidget, &QTabWidget::tabCloseRequested, this,
          &MainWindow::onTabCloseRequested);

  // Add to splitter
  m_centralSplitter->addWidget(m_agentTree);
  m_centralSplitter->addWidget(m_tabWidget);
  m_centralSplitter->setStretchFactor(0, 0);
  m_centralSplitter->setStretchFactor(1, 1);

  // Set initial sizes: 200px for tree, rest for tabs
  m_centralSplitter->setSizes(QList<int>() << 200 << 600);

  // Make splitter handle more subtle
  m_centralSplitter->setHandleWidth(1);

  // Create output window (hidden by default, each agent has its own)
  m_outputWindow = new OutputWindow(nullptr, this);
  m_outputWindow->hide();
}

void MainWindow::createMenus() {
  // File menu
  m_fileMenu = menuBar()->addMenu("&File");

  m_newAgentAction = new QAction("&New Agent...", this);
  m_newAgentAction->setShortcut(QKeySequence::New);
  m_newAgentAction->setStatusTip("Create a new Soar agent");
  connect(m_newAgentAction, &QAction::triggered, this, &MainWindow::newAgent);
  m_fileMenu->addAction(m_newAgentAction);

  m_fileMenu->addSeparator();

  m_connectRemoteAction = new QAction("&Connect to Remote...", this);
  m_connectRemoteAction->setStatusTip("Connect to a remote Soar kernel");
  connect(m_connectRemoteAction, &QAction::triggered, this,
          &MainWindow::connectToRemote);
  m_fileMenu->addAction(m_connectRemoteAction);

  m_disconnectAction = new QAction("&Disconnect", this);
  m_disconnectAction->setStatusTip("Disconnect from the current kernel");
  connect(m_disconnectAction, &QAction::triggered, this,
          &MainWindow::disconnectKernel);
  m_fileMenu->addAction(m_disconnectAction);

  m_fileMenu->addSeparator();

  m_exitAction = new QAction("E&xit", this);
  m_exitAction->setShortcut(QKeySequence::Quit);
  m_exitAction->setStatusTip("Exit the debugger");
  connect(m_exitAction, &QAction::triggered, this,
          &MainWindow::exitApplication);
  m_fileMenu->addAction(m_exitAction);

  // Agent menu
  m_agentMenu = menuBar()->addMenu("&Agent");

  m_runAction = new QAction("&Run", this);
  m_runAction->setShortcut(QKeySequence("F5"));
  m_runAction->setStatusTip("Run the selected agent");
  connect(m_runAction, &QAction::triggered, this, &MainWindow::runAgent);
  m_agentMenu->addAction(m_runAction);

  m_stopAction = new QAction("&Stop", this);
  m_stopAction->setShortcut(QKeySequence("Shift+F5"));
  m_stopAction->setStatusTip("Stop the selected agent");
  connect(m_stopAction, &QAction::triggered, this, &MainWindow::stopAgent);
  m_agentMenu->addAction(m_stopAction);

  m_stepAction = new QAction("Step &Decision", this);
  m_stepAction->setShortcut(QKeySequence("F10"));
  m_stepAction->setStatusTip("Run one decision cycle");
  connect(m_stepAction, &QAction::triggered, this, &MainWindow::stepAgent);
  m_agentMenu->addAction(m_stepAction);

  m_agentMenu->addSeparator();

  m_initAction = new QAction("&Init-Soar", this);
  m_initAction->setStatusTip("Reinitialize the selected agent");
  connect(m_initAction, &QAction::triggered, this, &MainWindow::initAgent);
  m_agentMenu->addAction(m_initAction);

  // View menu
  m_viewMenu = menuBar()->addMenu("&View");

  m_showOutputAction = new QAction("&Output Window", this);
  m_showOutputAction->setCheckable(true);
  m_showOutputAction->setStatusTip("Show/hide the output window");
  connect(m_showOutputAction, &QAction::triggered, this,
          &MainWindow::showOutput);
  m_viewMenu->addAction(m_showOutputAction);

  m_showAgentTreeAction = new QAction("&Agent Tree", this);
  m_showAgentTreeAction->setCheckable(true);
  m_showAgentTreeAction->setChecked(true);
  m_showAgentTreeAction->setStatusTip("Show/hide the agent tree");
  connect(m_showAgentTreeAction, &QAction::triggered, this,
          &MainWindow::showAgentTree);
  m_viewMenu->addAction(m_showAgentTreeAction);

  // Help menu
  m_helpMenu = menuBar()->addMenu("&Help");

  m_aboutSoarAction = new QAction("&About Soar Debugger", this);
  m_aboutSoarAction->setStatusTip("About this application");
  connect(m_aboutSoarAction, &QAction::triggered, this, &MainWindow::aboutSoar);
  m_helpMenu->addAction(m_aboutSoarAction);

  m_aboutQtAction = new QAction("About &Qt", this);
  m_aboutQtAction->setStatusTip("About Qt");
  connect(m_aboutQtAction, &QAction::triggered, this, &MainWindow::aboutQt);
  m_helpMenu->addAction(m_aboutQtAction);
}

void MainWindow::createToolBars() {
  // File toolbar
  m_fileToolBar = addToolBar("File");
  m_fileToolBar->addAction(m_newAgentAction);
  m_fileToolBar->addSeparator();
  m_fileToolBar->addAction(m_connectRemoteAction);
  m_fileToolBar->addAction(m_disconnectAction);

  // Agent toolbar
  m_agentToolBar = addToolBar("Agent");
  m_agentToolBar->addAction(m_runAction);
  m_agentToolBar->addAction(m_stopAction);
  m_agentToolBar->addAction(m_stepAction);
  m_agentToolBar->addSeparator();
  m_agentToolBar->addAction(m_initAction);
}

void MainWindow::createStatusBar() {
  m_statusLabel = new QLabel("Ready");
  statusBar()->addWidget(m_statusLabel, 1);

  m_kernelStatusLabel = new QLabel("No kernel");
  statusBar()->addPermanentWidget(m_kernelStatusLabel);

  m_agentStatusLabel = new QLabel("No agent");
  statusBar()->addPermanentWidget(m_agentStatusLabel);
}

void MainWindow::updateActions() {
  bool hasKernel = m_debugger->isKernelRunning();
  bool hasAgent = !m_debugger->getAllAgents().isEmpty();

  // File actions
  m_newAgentAction->setEnabled(hasKernel);
  m_disconnectAction->setEnabled(hasKernel);

  // Agent actions
  m_runAction->setEnabled(hasAgent);
  m_stopAction->setEnabled(hasAgent);
  m_stepAction->setEnabled(hasAgent);
  m_initAction->setEnabled(hasAgent);
}

void MainWindow::updateStatusBar() {
  bool hasKernel = m_debugger->isKernelRunning();
  int agentCount = m_debugger->getAllAgents().size();

  if (hasKernel) {
    m_kernelStatusLabel->setText("Kernel: Running");
  } else {
    m_kernelStatusLabel->setText("Kernel: Stopped");
  }

  if (agentCount == 0) {
    m_agentStatusLabel->setText("Agents: 0");
  } else {
    m_agentStatusLabel->setText(QString("Agents: %1").arg(agentCount));
  }
}

// Slot implementations
void MainWindow::newAgent() {
  bool ok;
  QString name = QInputDialog::getText(
      this, "New Agent", "Agent name:", QLineEdit::Normal, "soar", &ok);

  if (ok && !name.isEmpty()) {
    SoarAgent *agent = m_debugger->createAgent(name);
    if (!agent) {
      QMessageBox::warning(this, "Error", "Failed to create agent: " + name);
    }
  }
}

void MainWindow::connectToRemote() {
  // Create dialog for remote connection
  QDialog dialog(this);
  dialog.setWindowTitle("Connect to Remote Kernel");
  dialog.setModal(true);

  QVBoxLayout *layout = new QVBoxLayout(&dialog);

  // Host input
  QHBoxLayout *hostLayout = new QHBoxLayout();
  hostLayout->addWidget(new QLabel("Host:"));
  QLineEdit *hostEdit = new QLineEdit("localhost");
  hostLayout->addWidget(hostEdit);
  layout->addLayout(hostLayout);

  // Port input
  QHBoxLayout *portLayout = new QHBoxLayout();
  portLayout->addWidget(new QLabel("Port:"));
  QSpinBox *portSpinBox = new QSpinBox();
  portSpinBox->setRange(1, 65535);
  portSpinBox->setValue(12121);
  portLayout->addWidget(portSpinBox);
  layout->addLayout(portLayout);

  // Buttons
  QHBoxLayout *buttonLayout = new QHBoxLayout();
  QPushButton *connectButton = new QPushButton("Connect");
  QPushButton *cancelButton = new QPushButton("Cancel");
  buttonLayout->addStretch();
  buttonLayout->addWidget(connectButton);
  buttonLayout->addWidget(cancelButton);
  layout->addLayout(buttonLayout);

  connect(connectButton, &QPushButton::clicked, &dialog, &QDialog::accept);
  connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

  if (dialog.exec() == QDialog::Accepted) {
    QString host = hostEdit->text();
    int port = portSpinBox->value();

    if (m_debugger->connectToRemoteKernel(host, port)) {
      m_statusLabel->setText(QString("Connected to %1:%2").arg(host).arg(port));
    } else {
      QMessageBox::warning(
          this, "Connection Failed",
          QString("Failed to connect to remote kernel at %1:%2")
              .arg(host)
              .arg(port));
    }
  }
}

void MainWindow::disconnectKernel() { m_debugger->stopKernel(); }

void MainWindow::exitApplication() { close(); }

void MainWindow::runAgent() {
  SoarAgent *agent = getCurrentAgent();
  if (agent) {
    agent->runTilOutput();
    m_statusLabel->setText(QString("Running agent: %1").arg(agent->name()));
  }
}

void MainWindow::stopAgent() {
  SoarAgent *agent = getCurrentAgent();
  if (agent) {
    agent->stop();
    m_statusLabel->setText(QString("Stopped agent: %1").arg(agent->name()));
  }
}

void MainWindow::initAgent() {
  SoarAgent *agent = getCurrentAgent();
  if (agent) {
    agent->initSoar();
    m_statusLabel->setText(QString("Initialized agent: %1").arg(agent->name()));
  }
}

void MainWindow::stepAgent() {
  SoarAgent *agent = getCurrentAgent();
  if (agent) {
    agent->runTilDecision();
    m_statusLabel->setText(QString("Stepped agent: %1").arg(agent->name()));
  }
}

void MainWindow::showOutput() {
  // Global output window is deprecated - each agent window has its own
  // This could be used for kernel-level messages if needed
  if (m_showOutputAction->isChecked()) {
    m_outputWindow->show();
    m_outputWindow->raise();
  } else {
    m_outputWindow->hide();
  }
}

void MainWindow::showAgentTree() {
  m_agentTree->setVisible(m_showAgentTreeAction->isChecked());
}

void MainWindow::aboutSoar() {
  QMessageBox::about(
      this, "About Soar Debugger",
      "Soar Debugger\n\n"
      "A Qt-based debugger for the Soar cognitive architecture.\n"
      "Version: 9.6.4\n\n"
      "Copyright (c) University of Michigan");
}

void MainWindow::aboutQt() { QMessageBox::aboutQt(this); }

void MainWindow::onKernelStarted() {
  m_statusLabel->setText("Kernel started");
  updateActions();
  updateStatusBar();
}

void MainWindow::onKernelStopped() {
  m_statusLabel->setText("Kernel stopped");

  // Close all agent tabs
  m_tabWidget->clear();
  m_agentTree->clear();

  updateActions();
  updateStatusBar();
}

void MainWindow::onAgentCreated(SoarAgent *agent) {
  // Add to agent tree
  QTreeWidgetItem *item = new QTreeWidgetItem(m_agentTree);
  item->setText(0, agent->name());
  item->setData(0, Qt::UserRole, QVariant::fromValue(agent));

  // Create agent window and add to tabs
  AgentWindow *agentWindow = new AgentWindow(agent, this);
  int tabIndex = m_tabWidget->addTab(agentWindow, agent->name());
  m_tabWidget->setCurrentIndex(tabIndex);

  m_statusLabel->setText(QString("Agent created: %1").arg(agent->name()));
  updateActions();
  updateStatusBar();
}

void MainWindow::onAgentDestroyed(SoarAgent *agent) {
  // Remove from agent tree
  for (int i = 0; i < m_agentTree->topLevelItemCount(); ++i) {
    QTreeWidgetItem *item = m_agentTree->topLevelItem(i);
    SoarAgent *itemAgent = item->data(0, Qt::UserRole).value<SoarAgent *>();
    if (itemAgent == agent) {
      delete item;
      break;
    }
  }

  // Remove tab
  for (int i = 0; i < m_tabWidget->count(); ++i) {
    AgentWindow *window = qobject_cast<AgentWindow *>(m_tabWidget->widget(i));
    if (window && window->agent() == agent) {
      m_tabWidget->removeTab(i);
      break;
    }
  }

  m_statusLabel->setText(QString("Agent destroyed: %1").arg(agent->name()));
  updateActions();
  updateStatusBar();
}

void MainWindow::onAgentSelectionChanged() {
  // TODO: Handle agent selection changes
}

void MainWindow::onTabCloseRequested(int index) {
  // Get the agent window
  AgentWindow *window = qobject_cast<AgentWindow *>(m_tabWidget->widget(index));
  if (window) {
    SoarAgent *agent = window->agent();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Close Agent",
        QString("Close agent '%1'? This will destroy the agent.")
            .arg(agent->name()),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (reply == QMessageBox::Yes) {
      m_debugger->destroyAgent(agent);
    }
  }
}

SoarAgent *MainWindow::getCurrentAgent() {
  int currentIndex = m_tabWidget->currentIndex();
  if (currentIndex >= 0) {
    AgentWindow *window =
        qobject_cast<AgentWindow *>(m_tabWidget->widget(currentIndex));
    if (window) {
      return window->agent();
    }
  }
  return nullptr;
}

void MainWindow::createDefaultAgent() {
  // Only create default agent if no agents exist and no command-line args
  // (command-line args would indicate being opened from source code)
  QStringList args = QCoreApplication::arguments();

  // Check if we have any agents already
  if (!m_debugger->getAllAgents().isEmpty()) {
    return;
  }

  // If opened with specific arguments (other than just the program name),
  // assume it's being called from source code and don't auto-create
  if (args.size() > 1) {
    return;
  }

  // Create a default agent named "soar"
  SoarAgent *agent = m_debugger->createAgent("soar");
  if (agent) {
    m_statusLabel->setText("Default agent 'soar' created");
  }
}
