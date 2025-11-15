#include "SoarDebugger.h"
#include "SoarAgent.h"

// Include SML headers directly
#include "sml_Client.h"
#include "sml_ClientAgent.h"
#include "sml_ClientKernel.h"

#include <QCoreApplication>
#include <QDebug>

SoarDebugger::SoarDebugger(QObject *parent)
    : QObject(parent), m_kernel(nullptr), m_messageTimer(new QTimer(this)),
      m_isLocalKernel(false) {
  // Set up message pumping timer
  m_messageTimer->setSingleShot(false);
  m_messageTimer->setInterval(50); // 20 FPS
  connect(m_messageTimer, &QTimer::timeout, this, &SoarDebugger::pumpMessages);
}

SoarDebugger::~SoarDebugger() { shutdown(); }

bool SoarDebugger::initialize() {
  // Start with a local kernel by default
  return startLocalKernel();
}

void SoarDebugger::shutdown() {
  // Stop message pump immediately
  m_messageTimer->stop();

  // Destroy all agents first
  QList<SoarAgent *> agentsCopy =
      m_agents; // Copy list to avoid modification during iteration
  m_agents.clear();

  for (SoarAgent *agent : agentsCopy) {
    if (agent) {
      // Remove callbacks BEFORE destroying the SML agent
      agent->removeCallbacks();

      if (m_kernel) {
        try {
          m_kernel->DestroyAgent(agent->smlAgent());
        } catch (...) {
          // Ignore exceptions during shutdown
        }
      }
      delete agent; // Delete immediately, not later
    }
  }

  // Shutdown kernel
  stopKernel();
}

bool SoarDebugger::startLocalKernel(int port) {
  if (m_kernel) {
    qWarning() << "Kernel already running";
    return false;
  }

  try {
    // Create kernel in current thread
    m_kernel.reset(sml::Kernel::CreateKernelInCurrentThread(
        port == -1 ? sml::Kernel::kUseAnyPort : port));

    if (!m_kernel) {
      qCritical() << "Failed to create Soar kernel";
      return false;
    }

    m_isLocalKernel = true;
    m_messageTimer->start();

    qInfo() << "Soar kernel started on port" << m_kernel->GetListenerPort();
    emit kernelStarted();

    return true;

  } catch (const std::exception &e) {
    qCritical() << "Exception starting kernel:" << e.what();
    return false;
  }
}

bool SoarDebugger::connectToRemoteKernel(const QString &host, int port) {
  if (m_kernel) {
    qWarning() << "Kernel already running";
    return false;
  }

  try {
    // Create remote connection
    m_kernel.reset(sml::Kernel::CreateRemoteConnection(
        true, host.toUtf8().constData(), port));

    if (!m_kernel) {
      qCritical() << "Failed to connect to remote kernel at" << host << ":"
                  << port;
      return false;
    }

    // Test the connection
    if (m_kernel->HadError()) {
      QString error = m_kernel->GetLastErrorDescription();
      qCritical() << "Kernel connection error:" << error;
      m_kernel.reset();
      return false;
    }

    m_isLocalKernel = false;
    m_messageTimer->start();

    qInfo() << "Connected to remote kernel at" << host << ":" << port;
    emit kernelStarted();

    return true;

  } catch (const std::exception &e) {
    qCritical() << "Exception connecting to remote kernel:" << e.what();
    return false;
  }
}

void SoarDebugger::stopKernel() {
  if (!m_kernel) {
    return;
  }

  m_messageTimer->stop();

  try {
    if (m_isLocalKernel) {
      // Shutdown can block, but we need to do it
      m_kernel->Shutdown();
    }

    m_kernel.reset();
    m_isLocalKernel = false;

    qInfo() << "Soar kernel stopped";
    emit kernelStopped();

  } catch (const std::exception &e) {
    qWarning() << "Exception stopping kernel:" << e.what();
    // Force reset even if exception occurred
    m_kernel.reset();
    m_isLocalKernel = false;
  } catch (...) {
    qWarning() << "Unknown exception stopping kernel";
    m_kernel.reset();
    m_isLocalKernel = false;
  }
}

SoarAgent *SoarDebugger::createAgent(const QString &name) {
  if (!m_kernel) {
    qWarning() << "No kernel available";
    return nullptr;
  }

  // Check if agent already exists
  if (getAgent(name)) {
    qWarning() << "Agent" << name << "already exists";
    return nullptr;
  }

  try {
    // Create the SML agent
    sml::Agent *smlAgent = m_kernel->CreateAgent(name.toUtf8().constData());
    if (!smlAgent) {
      qCritical() << "Failed to create agent" << name;
      return nullptr;
    }

    // Create our wrapper
    SoarAgent *agent = new SoarAgent(smlAgent, this);
    m_agents.append(agent);

    qInfo() << "Created agent:" << name;
    emit agentCreated(agent);

    return agent;

  } catch (const std::exception &e) {
    qCritical() << "Exception creating agent:" << e.what();
    return nullptr;
  }
}

void SoarDebugger::destroyAgent(SoarAgent *agent) {
  if (!agent || !m_kernel) {
    return;
  }

  try {
    QString name = agent->name();
    sml::Agent *smlAgent = agent->smlAgent();

    // Remove from our list first
    m_agents.removeAll(agent);

    // Emit signal before deleting the agent object
    emit agentDestroyed(agent);

    // Destroy the SML agent
    if (smlAgent) {
      m_kernel->DestroyAgent(smlAgent);
    }

    // Delete our wrapper
    delete agent;

    qInfo() << "Destroyed agent:" << name;

  } catch (const std::exception &e) {
    qWarning() << "Exception destroying agent:" << e.what();
  }
}

SoarAgent *SoarDebugger::getAgent(const QString &name) {
  for (SoarAgent *agent : m_agents) {
    if (agent->name() == name) {
      return agent;
    }
  }
  return nullptr;
}

QList<SoarAgent *> SoarDebugger::getAllAgents() { return m_agents; }

void SoarDebugger::pumpMessages() {
  if (m_kernel && !m_isLocalKernel) {
    // For remote connections, we need to pump messages
    try {
      m_kernel->CheckForIncomingCommands();
    } catch (const std::exception &e) {
      qWarning() << "Exception pumping messages:" << e.what();
    }
  }
}
