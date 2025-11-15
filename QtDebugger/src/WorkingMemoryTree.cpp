#include "WorkingMemoryTree.h"
#include "SoarAgent.h"

#include "sml_Client.h"
#include "sml_ClientAgent.h"
#include "sml_ClientIdentifier.h"
#include "sml_ClientWMElement.h"

#include <QBrush>
#include <QColor>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QTimer>
#include <QVBoxLayout>

WorkingMemoryTree::WorkingMemoryTree(SoarAgent *agent, QWidget *parent)
    : QWidget(parent), m_agent(agent), m_tree(nullptr), m_commandEdit(nullptr),
      m_backButton(nullptr), m_refreshButton(nullptr),
      m_printCommand("print -d 3 <s>"), m_historyIndex(-1) {
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

  m_backButton = new QPushButton("← Back");
  m_backButton->setMaximumWidth(80);
  m_backButton->setEnabled(false);
  connect(m_backButton, &QPushButton::clicked, this,
          &WorkingMemoryTree::goBack);
  headerLayout->addWidget(m_backButton);

  m_refreshButton = new QPushButton("Refresh");
  m_refreshButton->setMaximumWidth(80);
  connect(m_refreshButton, &QPushButton::clicked, this,
          &WorkingMemoryTree::refresh);
  headerLayout->addWidget(m_refreshButton);

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
  connect(m_tree, &QTreeWidget::itemClicked, this,
          &WorkingMemoryTree::onItemClicked);
  layout->addWidget(m_tree);
}

void WorkingMemoryTree::refresh() {
  m_tree->clear();
  populateTree();
}

void WorkingMemoryTree::populateTree() {
  if (!m_agent)
    return;

  m_identifierMap.clear();

  // Use the configured print command
  QString result = m_agent->executeCommand(m_printCommand);

  QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_tree);
  rootItem->setText(0, "Working Memory");

  if (!result.isEmpty()) {
    QStringList lines = result.split('\n');

    // First pass: identify all identifiers
    QRegularExpression idRegex("\\(([A-Z]\\d+)");
    for (const QString &line : lines) {
      QRegularExpressionMatch match = idRegex.match(line);
      if (match.hasMatch()) {
        QString identifier = match.captured(1);
        if (!m_identifierMap.contains(identifier)) {
          QTreeWidgetItem *idItem = new QTreeWidgetItem(rootItem);
          idItem->setText(0, line);
          idItem->setData(0, Qt::UserRole, identifier); // Store identifier
          idItem->setForeground(
              0,
              QBrush(QColor(0, 100, 200))); // Blue color for main identifiers
          idItem->setToolTip(0, "Click to view " + identifier);
          m_identifierMap[identifier] = idItem;
        }
      } else if (!line.trimmed().isEmpty()) {
        QTreeWidgetItem *child = new QTreeWidgetItem(rootItem);
        QString displayText = makeIdentifierClickable(line);
        child->setText(0, displayText);
        // Color the item if it contains identifier references
        if (displayText.contains("→")) {
          child->setForeground(
              0, QBrush(QColor(0, 100, 200))); // Blue for lines with references
        }
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

QString WorkingMemoryTree::makeIdentifierClickable(const QString &text) {
  // Add arrow prefix to identifier references in the text
  QString result = text;
  QRegularExpression refRegex("\\b([A-Z]\\d+)\\b");
  QRegularExpressionMatchIterator it = refRegex.globalMatch(text);

  while (it.hasNext()) {
    QRegularExpressionMatch match = it.next();
    QString identifier = match.captured(1);
    if (m_identifierMap.contains(identifier)) {
      // Mark it as a clickable reference
      result.replace(identifier, "→" + identifier);
    }
  }
  return result;
}

void WorkingMemoryTree::navigateToIdentifier(const QString &identifier) {
  if (m_identifierMap.contains(identifier)) {
    QTreeWidgetItem *item = m_identifierMap[identifier];
    m_tree->setCurrentItem(item);
    m_tree->scrollToItem(item);
    item->setExpanded(true);

    // Flash the item to indicate navigation
    QBrush originalBrush = item->foreground(0);
    item->setForeground(0, QBrush(QColor(255, 100, 0))); // Orange highlight

    // Reset color after a delay
    QTimer::singleShot(500, [item, originalBrush]() {
      item->setForeground(0, originalBrush);
    });
  }
}

void WorkingMemoryTree::onItemClicked(QTreeWidgetItem *item, int column) {
  Q_UNUSED(column);

  // Check if this item represents an identifier (has identifier stored in
  // UserRole)
  QString identifier = item->data(0, Qt::UserRole).toString();

  if (!identifier.isEmpty()) {
    // Save current command to history before changing
    if (m_historyIndex == -1 || m_commandHistory.isEmpty() ||
        m_commandHistory.last() != m_printCommand) {
      m_commandHistory.append(m_printCommand);
      m_historyIndex = m_commandHistory.size() - 1;
    }

    // Update the command to print this identifier
    m_printCommand = QString("print -d 3 %1").arg(identifier);
    m_commandEdit->setText(m_printCommand);
    m_backButton->setEnabled(true);
    refresh();
    return;
  }

  // Otherwise check if this is a line with an identifier reference (contains →)
  QString text = item->text(0);
  QRegularExpression refRegex("→([A-Z]\\d+)");
  QRegularExpressionMatch match = refRegex.match(text);

  if (match.hasMatch()) {
    identifier = match.captured(1);

    // Save current command to history before changing
    if (m_historyIndex == -1 || m_commandHistory.isEmpty() ||
        m_commandHistory.last() != m_printCommand) {
      m_commandHistory.append(m_printCommand);
      m_historyIndex = m_commandHistory.size() - 1;
    }

    // Update the command to print this identifier
    m_printCommand = QString("print -d 3 %1").arg(identifier);
    m_commandEdit->setText(m_printCommand);
    m_backButton->setEnabled(true);
    refresh();
  }
}

void WorkingMemoryTree::goBack() {
  if (m_commandHistory.isEmpty() || m_historyIndex <= 0) {
    return;
  }

  m_historyIndex--;
  m_printCommand = m_commandHistory[m_historyIndex];
  m_commandEdit->setText(m_printCommand);

  if (m_historyIndex <= 0) {
    m_backButton->setEnabled(false);
  }

  refresh();
}
