#ifndef COMMAND_WINDOW_H
#define COMMAND_WINDOW_H

#include <QLineEdit>
#include <QStringList>
#include <QTextEdit>
#include <QWidget>

class SoarAgent;

class CommandWindow : public QWidget {
  Q_OBJECT

public:
  explicit CommandWindow(SoarAgent *agent, QWidget *parent = nullptr);

public slots:
  void executeCommand();
  void clearOutput();

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

private:
  void createLayout();
  void navigateHistory(bool up);

  SoarAgent *m_agent;
  QLineEdit *m_commandLine;
  QTextEdit *m_outputDisplay;
  QStringList m_commandHistory;
  int m_historyIndex;
};

#endif // COMMAND_WINDOW_H
