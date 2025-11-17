#ifndef OUTPUT_WINDOW_H
#define OUTPUT_WINDOW_H

#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QStringList>
#include <QTextEdit>
#include <QWidget>

class SoarAgent;

/**
 * @brief Combined window for displaying Soar output and executing commands
 */
class OutputWindow : public QWidget {
  Q_OBJECT

public:
  explicit OutputWindow(SoarAgent *agent = nullptr, QWidget *parent = nullptr);
  ~OutputWindow();

public slots:
  void appendOutput(const QString &text);
  void clearOutput();
  void toggleAutoScroll(bool enabled);
  void executeCommand();
  void openCliDocumentation();

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

private:
  void createLayout();
  void onPrintCallback(const QString &message);
  void navigateHistory(bool up);

  SoarAgent *m_agent;
  QTextEdit *m_textEdit;
  QPushButton *m_clearButton;
  QCheckBox *m_autoScrollCheckbox;
  bool m_autoScroll;

  // Command line components
  QLineEdit *m_commandLine;
  QStringList m_commandHistory;
  int m_historyIndex;
};

#endif // OUTPUT_WINDOW_H
