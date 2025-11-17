#include "OutputWindow.h"
#include "SoarAgent.h"

#include <QCheckBox>
#include <QDesktopServices>
#include <QFont>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>

OutputWindow::OutputWindow(SoarAgent *agent, QWidget *parent)
    : QWidget(parent), m_agent(agent), m_textEdit(new QTextEdit(this)),
      m_clearButton(nullptr), m_autoScrollCheckbox(nullptr), m_autoScroll(true),
      m_commandLine(nullptr), m_historyIndex(-1) {
  createLayout();

  // Connect to agent's output signal if agent is provided
  if (m_agent) {
    connect(m_agent, &SoarAgent::outputReceived, this,
            &OutputWindow::appendOutput);
  }
}

OutputWindow::~OutputWindow() {}

void OutputWindow::createLayout() {
  setMinimumSize(400, 300);

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(4, 4, 4, 4);

  // Header
  QHBoxLayout *headerLayout = new QHBoxLayout();
  QLabel *label = new QLabel("Command Line & Output");
  label->setStyleSheet("font-weight: bold;");
  headerLayout->addWidget(label);
  headerLayout->addStretch();

  m_autoScrollCheckbox = new QCheckBox("Auto-scroll");
  m_autoScrollCheckbox->setChecked(m_autoScroll);
  connect(m_autoScrollCheckbox, &QCheckBox::toggled, this,
          &OutputWindow::toggleAutoScroll);
  headerLayout->addWidget(m_autoScrollCheckbox);

  m_clearButton = new QPushButton("Clear");
  m_clearButton->setMaximumWidth(80);
  connect(m_clearButton, &QPushButton::clicked, this,
          &OutputWindow::clearOutput);
  headerLayout->addWidget(m_clearButton);

  layout->addLayout(headerLayout);

  // Configure text edit for output
  m_textEdit->setReadOnly(true);
  m_textEdit->setFont(QFont("Courier New", 10));
  layout->addWidget(m_textEdit);

  // Command input area
  QHBoxLayout *cmdLayout = new QHBoxLayout();
  cmdLayout->addWidget(new QLabel(">"));

  m_commandLine = new QLineEdit();
  m_commandLine->setPlaceholderText("Enter Soar command...");
  m_commandLine->installEventFilter(this);
  connect(m_commandLine, &QLineEdit::returnPressed, this,
          &OutputWindow::executeCommand);
  cmdLayout->addWidget(m_commandLine);

  // Info button for CLI documentation
  QPushButton *infoButton = new QPushButton("ℹ");
  infoButton->setMaximumWidth(30);
  infoButton->setToolTip("Open Soar CLI Documentation");
  infoButton->setStyleSheet("font-size: 14pt; font-weight: bold;");
  connect(infoButton, &QPushButton::clicked, this,
          &OutputWindow::openCliDocumentation);
  cmdLayout->addWidget(infoButton);

  layout->addLayout(cmdLayout);
}

void OutputWindow::appendOutput(const QString &text) {
  m_textEdit->append(text);

  if (m_autoScroll) {
    m_textEdit->verticalScrollBar()->setValue(
        m_textEdit->verticalScrollBar()->maximum());
  }
}

void OutputWindow::clearOutput() { m_textEdit->clear(); }

void OutputWindow::toggleAutoScroll(bool enabled) { m_autoScroll = enabled; }

void OutputWindow::onPrintCallback(const QString &message) {
  appendOutput(message);
}

void OutputWindow::executeCommand() {
  QString command = m_commandLine->text().trimmed();
  if (command.isEmpty() || !m_agent)
    return;

  // Add to history
  if (m_commandHistory.isEmpty() || m_commandHistory.last() != command) {
    m_commandHistory.append(command);
  }
  m_historyIndex = m_commandHistory.size();

  // Display command
  m_textEdit->append("> " + command);

  // Execute command
  QString result = m_agent->executeCommand(command);
  m_textEdit->append(result);

  // Clear input
  m_commandLine->clear();

  // Auto-scroll to bottom
  if (m_autoScroll) {
    m_textEdit->verticalScrollBar()->setValue(
        m_textEdit->verticalScrollBar()->maximum());
  }
}

void OutputWindow::openCliDocumentation() {
  QDesktopServices::openUrl(QUrl("https://soar.eecs.umich.edu/reference/cli/"));
}

bool OutputWindow::eventFilter(QObject *obj, QEvent *event) {
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

void OutputWindow::navigateHistory(bool up) {
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
