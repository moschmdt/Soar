#include "OutputWindow.h"
#include "SoarAgent.h"

#include <QCheckBox>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QTextEdit>
#include <QVBoxLayout>

OutputWindow::OutputWindow(SoarAgent *agent, QWidget *parent)
    : QWidget(parent), m_agent(agent), m_textEdit(new QTextEdit(this)),
      m_clearButton(nullptr), m_autoScrollCheckbox(nullptr),
      m_autoScroll(true) {
  createLayout();
}

OutputWindow::~OutputWindow() {}

void OutputWindow::createLayout() {
  setMinimumSize(400, 300);

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(4, 4, 4, 4);

  // Header
  QHBoxLayout *headerLayout = new QHBoxLayout();
  QLabel *label = new QLabel("Agent Output");
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

  // Configure text edit
  m_textEdit->setReadOnly(true);
  m_textEdit->setFont(QFont("Courier New", 10));

  layout->addWidget(m_textEdit);
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
