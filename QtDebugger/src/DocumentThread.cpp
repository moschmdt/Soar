#include "DocumentThread.h"
#include "sml_Client.h"
#include "sml_ClientAgent.h"
#include "sml_ClientKernel.h"
#include "sml_Events.h"
#include <QDebug>

DocumentThread::DocumentThread(QObject *parent)
    : QThread(parent), m_connected(false), m_stopping(false),
      m_stopRequested(false), m_kernel(nullptr), m_interruptCallbackId(-1) {}

DocumentThread::~DocumentThread() {
  unregisterInterruptCallback();
  stopThread();
  wait();
}

void DocumentThread::setConnected(bool connected) {
  QMutexLocker locker(&m_mutex);
  m_connected = connected;
  if (connected) {
    // Start thread if not running
    if (!isRunning()) {
      start();
    }
  }
}

void DocumentThread::setKernel(sml::Kernel *kernel) {
  QMutexLocker locker(&m_mutex);
  unregisterInterruptCallback();
  m_kernel = kernel;
  if (m_kernel) {
    registerInterruptCallback();
  }
}

void DocumentThread::runAllAgents(int numberSteps) {
  QMutexLocker locker(&m_mutex);

  if (!m_connected || !m_kernel) {
    qWarning() << "DocumentThread: Not connected or no kernel";
    return;
  }

  m_stopRequested = false;
  Command cmd;
  cmd.type = CMD_RUN_N_STEPS;
  cmd.steps = numberSteps;
  cmd.agent = nullptr;
  m_commandQueue.push(cmd);
  m_condition.wakeOne();
}

void DocumentThread::runAllTilOutput() {
  QMutexLocker locker(&m_mutex);

  if (!m_connected || !m_kernel) {
    qWarning() << "DocumentThread: Not connected or no kernel";
    return;
  }

  m_stopRequested = false;
  Command cmd;
  cmd.type = CMD_RUN_TIL_OUTPUT;
  cmd.agent = nullptr;
  m_commandQueue.push(cmd);
  m_condition.wakeOne();
}

void DocumentThread::runAllForever() {
  QMutexLocker locker(&m_mutex);

  if (!m_connected || !m_kernel) {
    qWarning() << "DocumentThread: Not connected or no kernel";
    return;
  }

  m_stopRequested = false;
  Command cmd;
  cmd.type = CMD_RUN_FOREVER;
  cmd.agent = nullptr;
  m_commandQueue.push(cmd);
  m_condition.wakeOne();
}

void DocumentThread::stopAllAgents() {
  QMutexLocker locker(&m_mutex);

  if (!m_kernel) {
    qWarning() << "DocumentThread: No kernel";
    return;
  }

  // Set flag - actual stop will happen in interrupt callback
  m_stopRequested = true;
  qDebug() << "DocumentThread: Stop requested";
}

void DocumentThread::executeCommand(sml::Agent *agent, const QString &command) {
  QMutexLocker locker(&m_mutex);

  if (!m_connected) {
    qWarning() << "DocumentThread: Not connected, cannot execute command";
    return;
  }

  Command cmd;
  cmd.type = CMD_EXECUTE;
  cmd.agent = agent;
  cmd.command = command;
  m_commandQueue.push(cmd);

  // Wake up the thread
  m_condition.wakeOne();
}

void DocumentThread::stopThread() {
  QMutexLocker locker(&m_mutex);
  m_stopping = true;
  m_connected = false;
  m_condition.wakeOne();
}

void DocumentThread::run() {
  qInfo() << "DocumentThread started";

  while (true) {
    Command cmd;

    {
      QMutexLocker locker(&m_mutex);

      // Wait for work or stop signal
      while (m_commandQueue.empty() && !m_stopping) {
        m_condition.wait(&m_mutex);
      }

      if (m_stopping) {
        break;
      }

      if (!m_commandQueue.empty()) {
        cmd = m_commandQueue.front();
        m_commandQueue.pop();
      } else {
        continue;
      }
    }

    // Process command outside the lock
    processCommand(cmd);
  }

  qInfo() << "DocumentThread stopped";
}

void DocumentThread::processCommand(const Command &cmd) {
  QString result;

  try {
    switch (cmd.type) {
    case CMD_RUN_N_STEPS:
      if (m_kernel) {
        qDebug() << "DocumentThread: RunAllAgents(" << cmd.steps << ")";
        const char *output = m_kernel->RunAllAgents(cmd.steps);
        result = QString::fromUtf8(output ? output : "");
        emit runCompleted(result);
      }
      break;

    case CMD_RUN_TIL_OUTPUT:
      if (m_kernel) {
        qDebug() << "DocumentThread: RunAllTilOutput()";
        const char *output = m_kernel->RunAllTilOutput();
        result = QString::fromUtf8(output ? output : "");
        emit runCompleted(result);
      }
      break;

    case CMD_RUN_FOREVER:
      if (m_kernel) {
        qDebug() << "DocumentThread: RunAllAgentsForever()";
        const char *output = m_kernel->RunAllAgentsForever();
        result = QString::fromUtf8(output ? output : "");
        emit runCompleted(result);
      }
      break;

    case CMD_EXECUTE:
      if (cmd.agent) {
        qDebug() << "DocumentThread: Executing:" << cmd.command;
        const char *output =
            cmd.agent->ExecuteCommandLine(cmd.command.toUtf8().constData());
        result = QString::fromUtf8(output ? output : "");
        emit commandCompleted(result);
      }
      break;
    }

    // Signal UI to update
    emit updateUI();

  } catch (const std::exception &e) {
    qWarning() << "DocumentThread: Exception:" << e.what();
    result = QString("Error: %1").arg(e.what());
    if (cmd.type == CMD_EXECUTE) {
      emit commandCompleted(result);
    } else {
      emit runCompleted(result);
    }
  }
}

void DocumentThread::registerInterruptCallback() {
  if (!m_kernel)
    return;

  // Register for interrupt check events (fires frequently during runs)
  m_interruptCallbackId = m_kernel->RegisterForSystemEvent(
      sml::smlEVENT_INTERRUPT_CHECK, &DocumentThread::interruptCheckHandler,
      this);

  qDebug() << "DocumentThread: Registered interrupt callback"
           << m_interruptCallbackId;
}

void DocumentThread::unregisterInterruptCallback() {
  if (m_kernel && m_interruptCallbackId != -1) {
    m_kernel->UnregisterForSystemEvent(m_interruptCallbackId);
    m_interruptCallbackId = -1;
    qDebug() << "DocumentThread: Unregistered interrupt callback";
  }
}

void DocumentThread::interruptCheckHandler(smlSystemEventId eventId,
                                           void *userData,
                                           sml::Kernel *kernel) {
  Q_UNUSED(eventId);

  DocumentThread *self = static_cast<DocumentThread *>(userData);
  if (!self || !kernel)
    return;

  QMutexLocker locker(&self->m_mutex);

  // Check if stop was requested
  if (self->m_stopRequested) {
    qDebug() << "DocumentThread: Interrupt callback - stopping agents";
    self->m_stopRequested = false;
    kernel->StopAllAgents();
  }
}
