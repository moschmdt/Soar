#include "AgentWindow.h"
#include "OutputWindow.h"
#include "ProductionsWindow.h"
#include "SoarAgent.h"
#include "WorkingMemoryTree.h"

#include <QHBoxLayout>
#include <QSplitter>
#include <QVBoxLayout>

AgentWindow::AgentWindow(SoarAgent *agent, QWidget *parent)
    : QWidget(parent), m_agent(agent), m_mainSplitter(nullptr),
      m_leftSplitter(nullptr), m_rightSplitter(nullptr), m_wmTree(nullptr),
      m_productionsWindow(nullptr), m_outputWindow(nullptr) {
  setWindowTitle(QString("Agent: %1").arg(agent->name()));
  createLayout();
}

AgentWindow::~AgentWindow() {}

void AgentWindow::createLayout() {
  // Main layout
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);

  // Create main horizontal splitter (left | right)
  m_mainSplitter = new QSplitter(Qt::Horizontal, this);

  // Left side: vertical splitter for WM tree and productions
  m_leftSplitter = new QSplitter(Qt::Vertical, m_mainSplitter);

  m_wmTree = new WorkingMemoryTree(m_agent, m_leftSplitter);
  m_productionsWindow = new ProductionsWindow(m_agent, m_leftSplitter);

  m_leftSplitter->addWidget(m_wmTree);
  m_leftSplitter->addWidget(m_productionsWindow);
  m_leftSplitter->setStretchFactor(0, 1);
  m_leftSplitter->setStretchFactor(1, 1);

  // Right side: Combined command and output window
  m_outputWindow = new OutputWindow(m_agent, m_mainSplitter);

  // Add left and output window to main splitter
  m_mainSplitter->addWidget(m_leftSplitter);
  m_mainSplitter->addWidget(m_outputWindow);
  m_mainSplitter->setStretchFactor(0, 1);
  m_mainSplitter->setStretchFactor(1, 1);

  mainLayout->addWidget(m_mainSplitter);
}
