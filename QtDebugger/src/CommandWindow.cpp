#include "CommandWindow.h"
#include "SoarAgent.h"

#include <QDesktopServices>
#include <QFont>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QUrl>
#include <QVBoxLayout>

CommandWindow::CommandWindow(SoarAgent *agent, QWidget *parent)
    : QWidget(parent), m_agent(agent), m_commandLine(nullptr),
      m_outputDisplay(nullptr), m_historyIndex(-1) {
  createLayout();
}

void CommandWindow::createLayout() {
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(4, 4, 4, 4);

  // Header
  QHBoxLayout *headerLayout = new QHBoxLayout();
  QLabel *label = new QLabel("Command Line");
  label->setStyleSheet("font-weight: bold;");
  headerLayout->addWidget(label);
  headerLayout->addStretch();

  QPushButton *clearButton = new QPushButton("Clear");
  clearButton->setMaximumWidth(80);
  connect(clearButton, &QPushButton::clicked, this,
          &CommandWindow::clearOutput);
  headerLayout->addWidget(clearButton);

  layout->addLayout(headerLayout);

  // Output display (read-only)
  m_outputDisplay = new QTextEdit();
  m_outputDisplay->setReadOnly(true);
  m_outputDisplay->setFont(QFont("Courier New", 10));
  m_outputDisplay->setMaximumHeight(100);
  layout->addWidget(m_outputDisplay);

  // Command input
  QHBoxLayout *cmdLayout = new QHBoxLayout();
  cmdLayout->addWidget(new QLabel(">"));

  m_commandLine = new QLineEdit();
  m_commandLine->setPlaceholderText("Enter Soar command...");
  m_commandLine->installEventFilter(this);
  connect(m_commandLine, &QLineEdit::returnPressed, this,
          &CommandWindow::executeCommand);
  cmdLayout->addWidget(m_commandLine);

  // Info button for CLI documentation
  QPushButton *infoButton = new QPushButton("ℹ");
  infoButton->setMaximumWidth(30);
  infoButton->setToolTip("Open Soar CLI Documentation");
  infoButton->setStyleSheet("font-size: 14pt; font-weight: bold;");
  connect(infoButton, &QPushButton::clicked, this,
          &CommandWindow::openCliDocumentation);
  cmdLayout->addWidget(infoButton);

  layout->addLayout(cmdLayout);
}

void CommandWindow::executeCommand() {
  QString command = m_commandLine->text().trimmed();
  if (command.isEmpty() || !m_agent)
    return;

  // Add to history
  if (m_commandHistory.isEmpty() || m_commandHistory.last() != command) {
    m_commandHistory.append(command);
  }
  m_historyIndex = m_commandHistory.size();

  // Display command
  m_outputDisplay->append("> " + command);

  // Execute command
  QString result = m_agent->executeCommand(command);
  m_outputDisplay->append(result);

  // Clear input
  m_commandLine->clear();

  // Auto-scroll to bottom
  m_outputDisplay->verticalScrollBar()->setValue(
      m_outputDisplay->verticalScrollBar()->maximum());
}

void CommandWindow::clearOutput() {
  if (m_outputDisplay) {
    m_outputDisplay->clear();
  }
}

void CommandWindow::openCliDocumentation() {
  QDesktopServices::openUrl(QUrl("https://soar.eecs.umich.edu/reference/cli/"));
}

bool CommandWindow::eventFilter(QObject *obj, QEvent *event) {
  if (obj == m_commandLine && event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if (keyEvent->key() == Qt::Key_Up) {
      navigateHistory(true);
      return true;
    } else if (keyEvent->key() == Qt::Key_Down) {
      navigateHistory(false);
      return true;
    }
  }
  return QWidget::eventFilter(obj, event);
}

void CommandWindow::navigateHistory(bool up) {
  if (m_commandHistory.isEmpty())
    return;

  if (up) {
    if (m_historyIndex > 0) {
      m_historyIndex--;
      m_commandLine->setText(m_commandHistory[m_historyIndex]);
    }
  } else {
    if (m_historyIndex < m_commandHistory.size() - 1) {
      m_historyIndex++;
      m_commandLine->setText(m_commandHistory[m_historyIndex]);
    } else {
      m_historyIndex = m_commandHistory.size();
      m_commandLine->clear();
    }
  }
}
