#include "WorkingMemoryTree.h"
#include "SoarAgent.h"

#include "sml_Client.h"
#include "sml_ClientAgent.h"
#include "sml_ClientIdentifier.h"
#include "sml_ClientWMElement.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

WorkingMemoryTree::WorkingMemoryTree(SoarAgent *agent, QWidget *parent)
    : QWidget(parent), m_agent(agent), m_tree(nullptr),
      m_refreshButton(nullptr), m_expandButton(nullptr),
      m_collapseButton(nullptr) {
  createLayout();
  refresh();
}

void WorkingMemoryTree::createLayout() {
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(4, 4, 4, 4);

  // Header with buttons
  QHBoxLayout *headerLayout = new QHBoxLayout();
  QLabel *label = new QLabel("Working Memory");
  label->setStyleSheet("font-weight: bold;");
  headerLayout->addWidget(label);
  headerLayout->addStretch();

  m_refreshButton = new QPushButton("Refresh");
  m_refreshButton->setMaximumWidth(80);
  connect(m_refreshButton, &QPushButton::clicked, this,
          &WorkingMemoryTree::refresh);
  headerLayout->addWidget(m_refreshButton);

  m_expandButton = new QPushButton("Expand");
  m_expandButton->setMaximumWidth(80);
  connect(m_expandButton, &QPushButton::clicked, this,
          &WorkingMemoryTree::expandAll);
  headerLayout->addWidget(m_expandButton);

  m_collapseButton = new QPushButton("Collapse");
  m_collapseButton->setMaximumWidth(80);
  connect(m_collapseButton, &QPushButton::clicked, this,
          &WorkingMemoryTree::collapseAll);
  headerLayout->addWidget(m_collapseButton);

  layout->addLayout(headerLayout);

  // Tree widget
  m_tree = new QTreeWidget();
  m_tree->setHeaderLabel("Working Memory Elements");
  m_tree->setAlternatingRowColors(true);
  layout->addWidget(m_tree);
}

void WorkingMemoryTree::refresh() {
  m_tree->clear();
  populateTree();
}

void WorkingMemoryTree::expandAll() { m_tree->expandAll(); }

void WorkingMemoryTree::collapseAll() { m_tree->collapseAll(); }

void WorkingMemoryTree::populateTree() {
  if (!m_agent)
    return;

  // Get the root state from the agent
  QString result = m_agent->executeCommand("print --depth 3 s1");

  // For now, just display the raw output in a single node
  // TODO: Parse the WM structure properly using SML API
  QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_tree);
  rootItem->setText(0, "State S1");

  if (!result.isEmpty()) {
    QStringList lines = result.split('\n');
    for (const QString &line : lines) {
      if (!line.trimmed().isEmpty()) {
        QTreeWidgetItem *child = new QTreeWidgetItem(rootItem);
        child->setText(0, line);
      }
    }
  }

  m_tree->expandToDepth(1);
}

void WorkingMemoryTree::addWMEToTree(QTreeWidgetItem *parent, void *identifier,
                                     int depth) {
  // TODO: Implement proper WME traversal using SML API
  // This would require accessing the sml::Identifier and iterating through WMEs
}
