#include <QApplication>
#include <QDir>
#include <QIcon>
#include <QPixmap>
#include <QSplashScreen>
#include <QStandardPaths>
#include <QStyleFactory>
#include <QTimer>

#include "MainWindow.h"
#include "SoarDebugger.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  // Set application properties
  app.setApplicationName("Soar Debugger");
  app.setApplicationVersion("9.6.4");
  app.setOrganizationName("University of Michigan");
  app.setOrganizationDomain("umich.edu");

  // Set a modern style if available
  if (QStyleFactory::keys().contains("Fusion")) {
    app.setStyle("Fusion");
  }

  // Create and initialize the debugger
  SoarDebugger debugger;
  if (!debugger.initialize()) {
    return 1;
  }

  // Create the main window
  MainWindow window(&debugger);
  window.show();

  return app.exec();
}
