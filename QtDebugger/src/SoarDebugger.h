#ifndef SOAR_DEBUGGER_H
#define SOAR_DEBUGGER_H

#include <QObject>
#include <QTimer>
#include <memory>

// Forward declare Soar SML classes to avoid including everything
namespace sml {
    class Kernel;
    class Agent;
}

class SoarAgent;

/**
 * @brief Main debugger controller that manages Soar kernel and agents
 */
class SoarDebugger : public QObject
{
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
    SoarAgent* createAgent(const QString &name);
    void destroyAgent(SoarAgent* agent);
    SoarAgent* getAgent(const QString &name);
    QList<SoarAgent*> getAllAgents();

    // Kernel access
    sml::Kernel* kernel() const { return m_kernel.get(); }

signals:
    void kernelStarted();
    void kernelStopped();
    void agentCreated(SoarAgent* agent);
    void agentDestroyed(SoarAgent* agent);

private slots:
    void pumpMessages();

private:
    std::unique_ptr<sml::Kernel> m_kernel;
    QList<SoarAgent*> m_agents;
    QTimer* m_messageTimer;
    bool m_isLocalKernel;
};

#endif // SOAR_DEBUGGER_H