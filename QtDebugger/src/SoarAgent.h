#ifndef SOAR_AGENT_H
#define SOAR_AGENT_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <sml_Client.h>
#include <sml_Events.h>

// Forward declare SML classes
namespace sml {
class Agent;
class Identifier;
class WMElement;
} // namespace sml

/**
 * @brief Wrapper for SML Agent with Qt-friendly interface
 */
class SoarAgent : public QObject {
  Q_OBJECT

public:
  explicit SoarAgent(sml::Agent *smlAgent, QObject *parent = nullptr);
  ~SoarAgent();

  // Agent properties
  QString name() const;
  bool isRunning() const;

  // Agent control
  QString executeCommand(const QString &command);
  bool runStep(int count = 1);
  bool runTilDecision();
  bool runTilElaboration();
  bool runTilHalt();
  bool runTilOutput();
  bool stop();
  bool initSoar();

  // Working memory access
  QStringList getWorkingMemory();
  QStringList getPreferences(const QString &object = QString());
  QString getSlotValue(const QString &object, const QString &attribute);

  // Productions
  QStringList getProductions();
  QString getProductionText(const QString &prodName);
  bool loadProduction(const QString &prodText);

  // Access to underlying SML agent
  sml::Agent *smlAgent() const { return m_smlAgent; }

  // Callback management (public for shutdown sequence)
  void removeCallbacks();

signals:
  void outputReceived(const QString &output);
  void runStarted();
  void runStopped();
  void workingMemoryChanged();

private slots:
  void onPrintEvent();

private:
  sml::Agent *m_smlAgent;
  int m_printCallbackId;
  int m_updateCallbackId;

  void setupCallbacks();

  static void printHandler(sml::smlPrintEventId eventId, void *userData,
                           sml::Agent *agent, const char *message);
  static void updateHandler(sml::smlUpdateEventId eventId, void *userData,
                            sml::Kernel *kernel, sml::smlRunFlags runFlags);
};

#endif // SOAR_AGENT_H
