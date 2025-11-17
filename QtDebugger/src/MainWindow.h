#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QAction>
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QSplitter>
#include <QStatusBar>
#include <QTabWidget>
#include <QToolBar>
#include <QTreeWidget>

class SoarDebugger;
class SoarAgent;
class AgentWindow;
class OutputWindow;

/**
 * @brief Main window for the Soar debugger
 */
class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(SoarDebugger *debugger, QWidget *parent = nullptr);
  ~MainWindow();

protected:
  void closeEvent(QCloseEvent *event) override;

private slots:
  // File menu
  void newAgent();
  void connectToRemote();
  void disconnectKernel();
  void exitApplication();

  // Agent menu
  void runAgent();
  void stopAgent();
  void initAgent();
  void stepAgent();

  // View menu
  void showOutput();
  void showAgentTree();

  // Help menu
  void aboutSoar();
  void showLicense();
  void showDiagnostics();
  void aboutQt();

  // Debugger events
  void onKernelStarted();
  void onKernelStopped();
  void onAgentCreated(SoarAgent *agent);
  void onAgentDestroyed(SoarAgent *agent);

  // UI events
  void onAgentSelectionChanged();
  void onTabCloseRequested(int index);

private:
  void createMenus();
  void createToolBars();
  void createStatusBar();
  void createCentralWidget();
  void updateActions();
  void updateStatusBar();
  void createDefaultAgent();
  SoarAgent *getCurrentAgent();

  SoarDebugger *m_debugger;

  // Central widget
  QSplitter *m_centralSplitter;
  QTreeWidget *m_agentTree;
  QTabWidget *m_tabWidget;

  // Windows
  OutputWindow *m_outputWindow;

  // Status bar widgets
  QLabel *m_statusLabel;
  QLabel *m_kernelStatusLabel;
  QLabel *m_agentStatusLabel;

  // Menus
  QMenu *m_fileMenu;
  QMenu *m_agentMenu;
  QMenu *m_viewMenu;
  QMenu *m_helpMenu;

  // Actions
  QAction *m_newAgentAction;
  QAction *m_connectRemoteAction;
  QAction *m_disconnectAction;
  QAction *m_exitAction;

  QAction *m_runAction;
  QAction *m_stopAction;
  QAction *m_initAction;
  QAction *m_stepAction;

  QAction *m_showOutputAction;
  QAction *m_showAgentTreeAction;

  QAction *m_aboutSoarAction;
  QAction *m_licenseAction;
  QAction *m_diagnosticsAction;
  QAction *m_aboutQtAction;

  // Toolbars
  QToolBar *m_fileToolBar;
  QToolBar *m_agentToolBar;
};

#endif // MAIN_WINDOW_H
