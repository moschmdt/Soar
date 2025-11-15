#include "SoarAgent.h"

#include "sml_Client.h"
#include "sml_ClientAgent.h"
#include "sml_ClientIdentifier.h"
#include "sml_ClientWMElement.h"

#include <QDebug>

SoarAgent::SoarAgent(sml::Agent *smlAgent, QObject *parent)
    : QObject(parent), m_smlAgent(smlAgent), m_printCallbackId(-1),
      m_updateCallbackId(-1) {
  setupCallbacks();
}

SoarAgent::~SoarAgent() { removeCallbacks(); }

QString SoarAgent::name() const {
  if (m_smlAgent) {
    return QString::fromUtf8(m_smlAgent->GetAgentName());
  }
  return QString();
}

bool SoarAgent::isRunning() const {
  if (!m_smlAgent)
    return false;
  // Use command line to check if agent is running
  // For now just return false as this is a stub
  return false;
}

QString SoarAgent::executeCommand(const QString &command) {
  if (!m_smlAgent) {
    return "Error: No agent";
  }

  try {
    const char *result =
        m_smlAgent->ExecuteCommandLine(command.toUtf8().constData());

    // Emit workingMemoryChanged for commands that modify WM
    QString cmd = command.trimmed().toLower();
    if (cmd.startsWith("run") || cmd.startsWith("step") ||
        cmd.startsWith("init-soar") || cmd.startsWith("excise") ||
        cmd.startsWith("sp ") || cmd.startsWith("learn")) {
      emit workingMemoryChanged();
    }

    return QString::fromUtf8(result ? result : "");
  } catch (const std::exception &e) {
    return QString("Exception: %1").arg(e.what());
  }
}

bool SoarAgent::runStep(int count) {
  if (!m_smlAgent)
    return false;

  QString command = QString("run %1").arg(count);
  const char *result =
      m_smlAgent->ExecuteCommandLine(command.toUtf8().constData());

  // Check if command executed successfully (non-null result)
  return (result != nullptr);
}

bool SoarAgent::runTilDecision() {
  if (!m_smlAgent)
    return false;

  const char *result = m_smlAgent->ExecuteCommandLine("run --decision");
  return (result != nullptr);
}

bool SoarAgent::runTilHalt() {
  if (!m_smlAgent)
    return false;

  const char *result = m_smlAgent->ExecuteCommandLine("run");
  return (result != nullptr);
}

bool SoarAgent::runTilElaboration() {
  if (!m_smlAgent)
    return false;

  const char *result = m_smlAgent->ExecuteCommandLine("run --elaboration");
  return (result != nullptr);
}

bool SoarAgent::runTilOutput() {
  if (!m_smlAgent)
    return false;

  const char *result = m_smlAgent->ExecuteCommandLine("run --output");
  return (result != nullptr);
}

bool SoarAgent::stop() {
  if (!m_smlAgent) {
    return false;
  }

  try {
    m_smlAgent->StopSelf();
    emit runStopped();
    return true;
  } catch (const std::exception &e) {
    qWarning() << "Exception in stop:" << e.what();
    return false;
  }
}

bool SoarAgent::initSoar() {
  if (!m_smlAgent) {
    return false;
  }

  try {
    const char *result = m_smlAgent->InitSoar();
    return result == nullptr ||
           strlen(result) == 0; // No error message means success
  } catch (const std::exception &e) {
    qWarning() << "Exception in initSoar:" << e.what();
    return false;
  }
}

QStringList SoarAgent::getWorkingMemory() {
  QStringList result;

  if (!m_smlAgent) {
    return result;
  }

  try {
    QString wmOutput = executeCommand("print --internal");
    result = wmOutput.split('\n', Qt::SkipEmptyParts);
  } catch (const std::exception &e) {
    qWarning() << "Exception getting working memory:" << e.what();
  }

  return result;
}

QStringList SoarAgent::getPreferences(const QString &object) {
  QStringList result;

  if (!m_smlAgent) {
    return result;
  }

  try {
    QString cmd = "preferences";
    if (!object.isEmpty()) {
      cmd += " " + object;
    }

    QString prefOutput = executeCommand(cmd);
    result = prefOutput.split('\n', Qt::SkipEmptyParts);
  } catch (const std::exception &e) {
    qWarning() << "Exception getting preferences:" << e.what();
  }

  return result;
}

QString SoarAgent::getSlotValue(const QString &object,
                                const QString &attribute) {
  if (!m_smlAgent) {
    return QString();
  }

  try {
    QString cmd = QString("print %1 ^%2").arg(object, attribute);
    return executeCommand(cmd);
  } catch (const std::exception &e) {
    qWarning() << "Exception getting slot value:" << e.what();
    return QString();
  }
}

QStringList SoarAgent::getProductions() {
  QStringList result;

  if (!m_smlAgent) {
    return result;
  }

  try {
    QString prodOutput = executeCommand("print --productions");
    result = prodOutput.split('\n', Qt::SkipEmptyParts);
  } catch (const std::exception &e) {
    qWarning() << "Exception getting productions:" << e.what();
  }

  return result;
}

QString SoarAgent::getProductionText(const QString &prodName) {
  if (!m_smlAgent) {
    return QString();
  }

  try {
    QString cmd = "print --exact " + prodName;
    return executeCommand(cmd);
  } catch (const std::exception &e) {
    qWarning() << "Exception getting production text:" << e.what();
    return QString();
  }
}

bool SoarAgent::loadProduction(const QString &prodText) {
  if (!m_smlAgent) {
    return false;
  }

  try {
    QString result = executeCommand(prodText);
    return !result.contains("Error", Qt::CaseInsensitive);
  } catch (const std::exception &e) {
    qWarning() << "Exception loading production:" << e.what();
    return false;
  }
}

void SoarAgent::setupCallbacks() {
  if (!m_smlAgent) {
    return;
  }

  try {
    // Register for print events to capture output
    m_printCallbackId = m_smlAgent->RegisterForPrintEvent(
        sml::smlEVENT_PRINT, SoarAgent::printHandler, this);

    // Register for update events (after each decision cycle)
    m_updateCallbackId = m_smlAgent->GetKernel()->RegisterForUpdateEvent(
        sml::smlEVENT_AFTER_ALL_OUTPUT_PHASES, SoarAgent::updateHandler, this);
  } catch (const std::exception &e) {
    qWarning() << "Exception setting up callbacks:" << e.what();
  }
}

void SoarAgent::removeCallbacks() {
  if (!m_smlAgent) {
    return;
  }

  try {
    if (m_printCallbackId != -1) {
      m_smlAgent->UnregisterForPrintEvent(m_printCallbackId);
      m_printCallbackId = -1;
    }
    if (m_updateCallbackId != -1) {
      m_smlAgent->GetKernel()->UnregisterForUpdateEvent(m_updateCallbackId);
      m_updateCallbackId = -1;
    }
  } catch (...) {
    // Agent may already be destroyed
  }
}

void SoarAgent::printHandler(sml::smlPrintEventId eventId, void *userData,
                             sml::Agent *agent, const char *message) {
  Q_UNUSED(eventId);
  Q_UNUSED(agent);

  SoarAgent *self = static_cast<SoarAgent *>(userData);
  if (self && message) {
    emit self->outputReceived(QString::fromUtf8(message));
  }
}

void SoarAgent::updateHandler(sml::smlUpdateEventId eventId, void *userData,
                              sml::Kernel *kernel, sml::smlRunFlags runFlags) {
  Q_UNUSED(eventId);
  Q_UNUSED(kernel);
  Q_UNUSED(runFlags);

  SoarAgent *self = static_cast<SoarAgent *>(userData);
  if (self) {
    emit self->workingMemoryChanged();
  }
}

void SoarAgent::onPrintEvent() {
  // This slot can be used for additional processing if needed
}
