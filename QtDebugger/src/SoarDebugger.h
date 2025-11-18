#ifndef SOAR_DEBUGGER_H
#define SOAR_DEBUGGER_H

#include <QObject>
#include <QTimer>
#include <memory>

// Forward declare Soar SML classes to avoid including everything
namespace sml {
class Kernel;
class Agent;
} // namespace sml

class SoarAgent;
class DocumentThread;

/**
 * @brief Main debugger controller that manages Soar kernel and agents
 */
class SoarDebugger : public QObject {
  Q_OBJECT

public:
  explicit SoarDebugger(QObject *parent = nullptr);
  ~SoarDebugger();

  bool initialize();
  void shutdown();

  // Kernel management
  bool startLocalKernel(int port = -1);
  bool connectToRemoteKernel(const QString &host, int port);
  void stopKernel();
  bool isKernelRunning() const { return m_kernel != nullptr; }

  // Agent management
  SoarAgent *createAgent(const QString &name);
  void destroyAgent(SoarAgent *agent);
  SoarAgent *getAgent(const QString &name);
  QList<SoarAgent *> getAllAgents();

  // Kernel access
  sml::Kernel *kernel() const { return m_kernel; }

signals:
  void kernelStarted();
  void kernelStopped();
  void agentCreated(SoarAgent *agent);
  void agentDestroyed(SoarAgent *agent);

private slots:
  void pumpMessages();

private:
  void setupKernel();

  sml::Kernel *m_kernel;
  QList<SoarAgent *> m_agents;
  QTimer *m_messageTimer;
  DocumentThread *m_documentThread;
  bool m_isLocalKernel; // True if we created/own the kernel, false if remote
                        // connection
};

#endif // SOAR_DEBUGGER_H
