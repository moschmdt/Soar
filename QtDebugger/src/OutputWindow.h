#ifndef OUTPUT_WINDOW_H
#define OUTPUT_WINDOW_H

#include <QCheckBox>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>

class SoarAgent;

/**
 * @brief Window for displaying Soar output and messages
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

private:
  void createLayout();
  void onPrintCallback(const QString &message);

  SoarAgent *m_agent;
  QTextEdit *m_textEdit;
  QPushButton *m_clearButton;
  QCheckBox *m_autoScrollCheckbox;
  bool m_autoScroll;
};

#endif // OUTPUT_WINDOW_H
