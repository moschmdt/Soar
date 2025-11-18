#ifndef DOCUMENT_THREAD_H
#define DOCUMENT_THREAD_H

#include <QMutex>
#include <QString>
#include <QThread>
#include <QWaitCondition>
#include <queue>

// Forward declarations
namespace sml {
class Kernel;
class Agent;
} // namespace sml

/**
 * @brief Thread for executing Soar commands asynchronously
 *
 * Similar to Java debugger's DocumentThread - handles all Soar kernel
 * operations in a separate thread to keep UI responsive.
 * Uses SML Kernel run API for non-blocking execution with event callbacks.
 */
class DocumentThread : public QThread {
  Q_OBJECT

public:
  explicit DocumentThread(QObject *parent = nullptr);
  ~DocumentThread();

  void setConnected(bool connected);
  bool isConnected() const { return m_connected; }

  void setKernel(sml::Kernel *kernel);

  // Callback for Soar events to check for interrupts
  static void interruptCheckHandler(smlSystemEventId eventId, void *userData,
                                    sml::Kernel *kernel);

  // Run commands using SML Kernel API (non-blocking)
  void runAllAgents(int numberSteps);
  void runAllTilOutput();
  void runAllForever();
  void stopAllAgents();

  // Execute other commands synchronously
  void executeCommand(sml::Agent *agent, const QString &command);

  // Stop the thread
  void stopThread();

signals:
  void commandCompleted(const QString &result);
  void runCompleted(const QString &result);
  void updateUI();

protected:
  void run() override;

private:
  enum CommandType {
    CMD_EXECUTE,        // ExecuteCommandLine
    CMD_RUN_N_STEPS,    // RunAllAgents(n)
    CMD_RUN_TIL_OUTPUT, // RunAllTilOutput()
    CMD_RUN_FOREVER     // RunAllAgentsForever()
  };
  struct Command {
    CommandType type;
    sml::Agent *agent;
    QString command;
    int steps;
  };

  bool m_connected;
  bool m_stopping;
  bool m_stopRequested;
  sml::Kernel *m_kernel;
  int m_interruptCallbackId;
  QMutex m_mutex;
  QWaitCondition m_condition;
  std::queue<Command> m_commandQueue;

  void processCommand(const Command &cmd);
  void registerInterruptCallback();
  void unregisterInterruptCallback();
};

#endif // DOCUMENT_THREAD_H
