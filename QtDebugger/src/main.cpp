#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QIcon>
#include <QPixmap>
#include <QSplashScreen>
#include <QStandardPaths>
#include <QStyleFactory>
#include <QTimer>

#include "MainWindow.h"
#include "SoarDebugger.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  // Set application properties
  app.setApplicationName("Soar Debugger");
  app.setApplicationVersion("9.6.4");
  app.setOrganizationName("University of Michigan");
  app.setOrganizationDomain("umich.edu");

  // Set a modern style if available
  if (QStyleFactory::keys().contains("Fusion"))
  {
    app.setStyle("Fusion");
  }

  // Parse command line arguments
  QCommandLineParser parser;
  parser.setApplicationDescription("Soar Debugger - A native C++ Qt debugger for Soar");
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption remoteOption(QStringList() << "r" << "remote",
                                  "Connect to a remote kernel instead of creating a local one");
  parser.addOption(remoteOption);

  QCommandLineOption portOption(QStringList() << "p" << "port",
                                "Port number for connection (default: 12121 for remote, any available for local)",
                                "port");
  parser.addOption(portOption);

  QCommandLineOption hostOption(QStringList() << "h" << "host",
                                "Host address for remote connection (default: localhost)",
                                "host",
                                "localhost");
  parser.addOption(hostOption);

  QCommandLineOption agentOption(QStringList() << "a" << "agent",
                                 "Agent name to connect to (will create if doesn't exist)",
                                 "agent");
  parser.addOption(agentOption);

  parser.process(app);

  // Create and initialize the debugger
  SoarDebugger debugger;

  bool success = false;
  if (parser.isSet(remoteOption))
  {
    // Connect to remote kernel
    QString host = parser.value(hostOption);
    int port = parser.isSet(portOption) ? parser.value(portOption).toInt() : 12121;

    qInfo() << "Connecting to remote kernel at" << host << ":" << port;
    success = debugger.connectToRemoteKernel(host, port);
  }
  else
  {
    // Start local kernel
    int port = parser.isSet(portOption) ? parser.value(portOption).toInt() : -1;
    success = debugger.startLocalKernel(port);
  }

  if (!success)
  {
    qCritical() << "Failed to initialize debugger";
    return 1;
  }

  // If an agent name was specified, ensure it exists
  if (parser.isSet(agentOption))
  {
    QString agentName = parser.value(agentOption);
    SoarAgent *agent = debugger.getAgent(agentName);
    if (!agent)
    {
      qInfo() << "Creating agent:" << agentName;
      agent = debugger.createAgent(agentName);
      if (!agent)
      {
        qCritical() << "Failed to create agent:" << agentName;
        return 1;
      }
    }
  }

  // Create the main window
  MainWindow window(&debugger);
  window.show();

  return app.exec();
}
