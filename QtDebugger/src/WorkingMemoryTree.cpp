#include "WorkingMemoryTree.h"
#include "SoarAgent.h"

#include "sml_Client.h"
#include "sml_ClientAgent.h"
#include "sml_ClientIdentifier.h"
#include "sml_ClientWMElement.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

WorkingMemoryTree::WorkingMemoryTree(SoarAgent *agent, QWidget *parent)
    : QWidget(parent), m_agent(agent), m_tree(nullptr), m_commandEdit(nullptr),
      m_refreshButton(nullptr), m_expandButton(nullptr),
      m_collapseButton(nullptr), m_printCommand("print -d 3 <s>") {
  createLayout();
  populateTree();

  // Auto-refresh when working memory changes
  if (m_agent) {
    connect(m_agent, &SoarAgent::workingMemoryChanged, this,
            &WorkingMemoryTree::refresh);
  }
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

  // Command input row
  QHBoxLayout *commandLayout = new QHBoxLayout();
  QLabel *cmdLabel = new QLabel("Command:");
  commandLayout->addWidget(cmdLabel);

  m_commandEdit = new QLineEdit();
  m_commandEdit->setText(m_printCommand);
  m_commandEdit->setPlaceholderText("e.g., print --depth 3 <s>");
  connect(m_commandEdit, &QLineEdit::textChanged, this,
          &WorkingMemoryTree::updateCommand);
  connect(m_commandEdit, &QLineEdit::returnPressed, this,
          &WorkingMemoryTree::refresh);
  commandLayout->addWidget(m_commandEdit);

  layout->addLayout(commandLayout);

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

  // Use the configured print command
  QString result = m_agent->executeCommand(m_printCommand);

  // For now, just display the raw output in a single node
  // TODO: Parse the WM structure properly using SML API
  QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_tree);
  rootItem->setText(0, "Working Memory");

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

void WorkingMemoryTree::updateCommand() {
  m_printCommand = m_commandEdit->text();
}

void WorkingMemoryTree::addWMEToTree(QTreeWidgetItem *parent, void *identifier,
                                     int depth) {
  // TODO: Implement proper WME traversal using SML API
  // This would require accessing the sml::Identifier and iterating through WMEs
}
