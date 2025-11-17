#include "WorkingMemoryTree.h"
#include "SoarAgent.h"

#include "sml_Client.h"
#include "sml_ClientAgent.h"
#include "sml_ClientIdentifier.h"
#include "sml_ClientWMElement.h"

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QBrush>
#include <QColor>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QRegularExpression>
#include <QTabWidget>
#include <QTextDocument>
#include <QTimer>
#include <QVBoxLayout>

HtmlDelegate::HtmlDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

void HtmlDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const {
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();

  QTextDocument doc;
  doc.setHtml(opt.text);
  doc.setDefaultFont(opt.font);

  painter->save();

  // Draw background
  opt.text = "";
  style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

  // Draw text
  QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt);
  painter->translate(textRect.topLeft());
  QRect clip(0, 0, textRect.width(), textRect.height());
  doc.drawContents(painter, clip);

  painter->restore();
}

QSize HtmlDelegate::sizeHint(const QStyleOptionViewItem &option,
                             const QModelIndex &index) const {
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  QTextDocument doc;
  doc.setHtml(opt.text);
  doc.setDefaultFont(opt.font);
  doc.setTextWidth(opt.rect.width());

  return QSize(doc.idealWidth(), doc.size().height());
}

WorkingMemoryTree::WorkingMemoryTree(SoarAgent *agent, QWidget *parent)
    : QWidget(parent), m_agent(agent), m_tabWidget(nullptr), m_tree(nullptr),
      m_commandEdit(nullptr), m_backButton(nullptr), m_refreshButton(nullptr),
      m_printCommand("print -d 3 -t <s>"), m_historyIndex(-1) {
  createLayout();

  // Auto-refresh when working memory changes
  if (m_agent) {
    connect(m_agent, &SoarAgent::workingMemoryChanged, this,
            &WorkingMemoryTree::refresh);
  }
}

void WorkingMemoryTree::createLayout() {
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(4, 4, 4, 4);

  // Header with label
  QHBoxLayout *headerLayout = new QHBoxLayout();
  QLabel *label = new QLabel("Working Memory");
  label->setStyleSheet("font-weight: bold;");
  headerLayout->addWidget(label);
  headerLayout->addStretch();
  layout->addLayout(headerLayout);

  // Create tab widget
  m_tabWidget = new QTabWidget();
  connect(m_tabWidget, &QTabWidget::currentChanged, this,
          &WorkingMemoryTree::onTabChanged);
  layout->addWidget(m_tabWidget);

  // Create tabs
  createTab("State", "print -d 3 -t <s>");
  createTab("Operator", "print -d 3 -t <o>");
  createTab("IO Link", "print -d 3 -t I1");
  createTab("Stack", "print --stack");

  // Set current to first tab and populate
  m_tabWidget->setCurrentIndex(0);
  refreshCurrentTab();
}

void WorkingMemoryTree::createTab(const QString &label,
                                  const QString &defaultCommand) {
  QWidget *tabWidget = new QWidget();
  QVBoxLayout *tabLayout = new QVBoxLayout(tabWidget);
  tabLayout->setContentsMargins(4, 4, 4, 4);

  // Header with back and refresh buttons
  QHBoxLayout *headerLayout = new QHBoxLayout();
  headerLayout->addStretch();

  QPushButton *backButton = new QPushButton("← Back");
  backButton->setMaximumWidth(80);
  backButton->setEnabled(false);
  headerLayout->addWidget(backButton);

  QPushButton *refreshButton = new QPushButton("Refresh");
  refreshButton->setMaximumWidth(80);
  headerLayout->addWidget(refreshButton);

  tabLayout->addLayout(headerLayout);

  // Command input row
  QHBoxLayout *commandLayout = new QHBoxLayout();
  QLabel *cmdLabel = new QLabel("Command:");
  commandLayout->addWidget(cmdLabel);

  QLineEdit *commandEdit = new QLineEdit();
  commandEdit->setText(defaultCommand);
  commandEdit->setPlaceholderText("e.g., print --depth 3 <s>");
  commandLayout->addWidget(commandEdit);

  tabLayout->addLayout(commandLayout);

  // Tree widget
  QTreeWidget *tree = new QTreeWidget();
  tree->setHeaderLabel("Working Memory Elements");
  tree->setAlternatingRowColors(true);
  tree->setItemDelegate(new HtmlDelegate(this));
  tabLayout->addWidget(tree);

  // Store tab data
  TabData tabData;
  tabData.tree = tree;
  tabData.commandEdit = commandEdit;
  tabData.backButton = backButton;
  tabData.printCommand = defaultCommand;
  tabData.historyIndex = -1;
  m_tabs.append(tabData);

  int tabIndex = m_tabs.size() - 1;

  // Connect signals
  connect(commandEdit, &QLineEdit::textChanged, [this, tabIndex]() {
    if (tabIndex < m_tabs.size()) {
      m_tabs[tabIndex].printCommand = m_tabs[tabIndex].commandEdit->text();
    }
  });
  connect(commandEdit, &QLineEdit::returnPressed,
          [this, tabIndex]() { refreshCurrentTab(); });
  connect(refreshButton, &QPushButton::clicked,
          [this, tabIndex]() { refreshCurrentTab(); });
  connect(backButton, &QPushButton::clicked, [this, tabIndex]() {
    if (tabIndex >= m_tabs.size())
      return;
    TabData &tab = m_tabs[tabIndex];
    if (tab.commandHistory.isEmpty() || tab.historyIndex <= 0)
      return;

    tab.historyIndex--;
    tab.printCommand = tab.commandHistory[tab.historyIndex];
    tab.commandEdit->setText(tab.printCommand);

    if (tab.historyIndex <= 0) {
      tab.backButton->setEnabled(false);
    }

    refreshCurrentTab();
  });
  connect(tree, &QTreeWidget::itemClicked,
          [this, tabIndex](QTreeWidgetItem *item, int column) {
            onItemClicked(item, column);
          });

  // Add tab
  m_tabWidget->addTab(tabWidget, label);
}

void WorkingMemoryTree::onTabChanged(int index) {
  Q_UNUSED(index);
  refreshCurrentTab();
}

void WorkingMemoryTree::refreshCurrentTab() {
  int currentIndex = m_tabWidget->currentIndex();
  if (currentIndex < 0 || currentIndex >= m_tabs.size())
    return;

  TabData &tab = m_tabs[currentIndex];
  tab.tree->clear();
  tab.identifierMap.clear();

  if (!m_agent)
    return;

  // Use the configured print command for this tab
  QString result = m_agent->executeCommand(tab.printCommand);

  QTreeWidgetItem *rootItem = new QTreeWidgetItem(tab.tree);
  rootItem->setText(0, "Working Memory");

  if (!result.isEmpty()) {
    QStringList lines = result.split('\n');

    // Check if tree mode is enabled
    bool isTreeMode = tab.printCommand.contains("-t");

    if (isTreeMode) {
      // Parse tree mode output
      QMap<int, QTreeWidgetItem *> indentStack;
      indentStack[0] = rootItem;
      QString lastRootIdentifier;

      // First pass: collect all identifiers
      QRegularExpression idRegex("\\(([A-Z]\\d+)");
      for (const QString &line : lines) {
        QRegularExpressionMatch match = idRegex.match(line);
        if (match.hasMatch()) {
          QString identifier = match.captured(1);
          if (!tab.identifierMap.contains(identifier)) {
            tab.identifierMap[identifier] = nullptr;
          }
        }
      }

      // Second pass: build tree structure
      for (const QString &line : lines) {
        if (line.trimmed().isEmpty())
          continue;

        int indent = 0;
        for (QChar c : line) {
          if (c == ' ')
            indent++;
          else
            break;
        }

        QRegularExpression idRegex("\\(([A-Z]\\d+)");
        QRegularExpressionMatch match = idRegex.match(line);

        QString currentIdentifier;
        if (match.hasMatch()) {
          currentIdentifier = match.captured(1);
        }

        QTreeWidgetItem *parent = rootItem;
        if (indent == 0) {
          parent = rootItem;
          if (!currentIdentifier.isEmpty()) {
            lastRootIdentifier = currentIdentifier;
          }
        } else {
          for (int i = indent - 2; i >= 0; i -= 2) {
            if (indentStack.contains(i)) {
              parent = indentStack[i];
              break;
            }
          }
        }

        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        QString displayText = line.trimmed();

        if (match.hasMatch()) {
          item->setData(0, Qt::UserRole, currentIdentifier);
          QString htmlText = displayText.toHtmlEscaped();
          htmlText.replace(currentIdentifier,
                           QString("<span style='color:#0064C8;'>%1</span>")
                               .arg(currentIdentifier));
          displayText = htmlText;
          item->setToolTip(0, "Click to view " + currentIdentifier);
          tab.identifierMap[currentIdentifier] = item;
        } else {
          displayText = makeIdentifierClickableForTab(displayText, tab);
        }

        item->setText(0, displayText);
        indentStack[indent] = item;

        QList<int> keysToRemove;
        for (int key : indentStack.keys()) {
          if (key > indent) {
            keysToRemove.append(key);
          }
        }
        for (int key : keysToRemove) {
          indentStack.remove(key);
        }
      }
    } else {
      // Non-tree mode
      QRegularExpression idRegex("\\(([A-Z]\\d+)");
      for (const QString &line : lines) {
        QRegularExpressionMatch match = idRegex.match(line);
        if (match.hasMatch()) {
          QString identifier = match.captured(1);
          if (!tab.identifierMap.contains(identifier)) {
            QTreeWidgetItem *idItem = new QTreeWidgetItem(rootItem);
            QString htmlText = line.toHtmlEscaped();
            htmlText.replace(identifier,
                             QString("<span style='color:#0064C8;'>%1</span>")
                                 .arg(identifier));
            idItem->setText(0, htmlText);
            idItem->setData(0, Qt::UserRole, identifier);
            idItem->setToolTip(0, "Click to view " + identifier);
            tab.identifierMap[identifier] = idItem;
          }
        } else if (!line.trimmed().isEmpty()) {
          QTreeWidgetItem *child = new QTreeWidgetItem(rootItem);
          QString displayText = makeIdentifierClickableForTab(line, tab);
          child->setText(0, displayText);

          QRegularExpression refRegex("\\b([A-Z]\\d+)\\b");
          QRegularExpressionMatchIterator it = refRegex.globalMatch(line);
          bool hasIdentifier = false;
          while (it.hasNext()) {
            QRegularExpressionMatch refMatch = it.next();
            if (tab.identifierMap.contains(refMatch.captured(1))) {
              hasIdentifier = true;
              break;
            }
          }
          if (hasIdentifier) {
            child->setToolTip(0, "Click to navigate to identifier");
          }
        }
      }
    }
  }

  tab.tree->expandToDepth(1);
}

QString WorkingMemoryTree::makeIdentifierClickableForTab(const QString &text,
                                                         TabData &tab) {
  QString result = text.toHtmlEscaped();
  QRegularExpression refRegex("\\b([A-Z]\\d+)\\b");

  QList<QPair<int, QString>> replacements;
  QRegularExpressionMatchIterator it = refRegex.globalMatch(text);

  while (it.hasNext()) {
    QRegularExpressionMatch match = it.next();
    QString identifier = match.captured(1);
    if (tab.identifierMap.contains(identifier)) {
      replacements.append(qMakePair(match.capturedStart(1), identifier));
    }
  }

  for (int i = replacements.size() - 1; i >= 0; --i) {
    int pos = replacements[i].first;
    QString identifier = replacements[i].second;
    QString coloredId =
        QString("<span style='color:#0064C8;'>%1</span>").arg(identifier);
    result.replace(pos, identifier.length(), coloredId);
  }

  return result;
}

void WorkingMemoryTree::refresh() { refreshCurrentTab(); }

void WorkingMemoryTree::populateTree() {
  // Deprecated - use refreshCurrentTab() instead
}

void WorkingMemoryTree::addWMEToTree(QTreeWidgetItem *parent, void *identifier,
                                     int depth) {
  // TODO: Implement proper WME traversal using SML API
}

QString WorkingMemoryTree::makeIdentifierClickable(const QString &text) {
  // Deprecated - use makeIdentifierClickableForTab() instead
  return text;
}

void WorkingMemoryTree::navigateToIdentifier(const QString &identifier) {
  // Navigation is handled through onItemClicked
}

void WorkingMemoryTree::onItemClicked(QTreeWidgetItem *item, int column) {
  Q_UNUSED(column);

  int currentIndex = m_tabWidget->currentIndex();
  if (currentIndex < 0 || currentIndex >= m_tabs.size())
    return;

  TabData &tab = m_tabs[currentIndex];

  // Check if this item represents an identifier
  QString identifier = item->data(0, Qt::UserRole).toString();

  if (!identifier.isEmpty()) {
    // Build the new command
    QString newCommand = QString("print -d 3 -t %1").arg(identifier);

    // Don't do anything if we're clicking the same identifier
    if (newCommand == tab.printCommand) {
      return;
    }

    // If we're in the middle of history, truncate forward history
    if (tab.historyIndex >= 0 && tab.historyIndex < tab.commandHistory.size()) {
      tab.commandHistory = tab.commandHistory.mid(0, tab.historyIndex + 1);
    }

    // Add current command to history
    tab.commandHistory.append(tab.printCommand);

    // Update to the new command and move forward in history
    tab.printCommand = newCommand;
    tab.commandEdit->setText(tab.printCommand);
    tab.historyIndex = tab.commandHistory.size();
    tab.backButton->setEnabled(true);
    refreshCurrentTab();
    return;
  }

  // Otherwise check if this is a line with an identifier reference
  QString text = item->text(0);
  QRegularExpression refRegex("\\b([A-Z]\\d+)\\b");
  QRegularExpressionMatch match = refRegex.match(text);

  if (match.hasMatch()) {
    identifier = match.captured(1);

    // Only process if it's a known identifier
    if (!tab.identifierMap.contains(identifier)) {
      return;
    }

    // Build the new command
    QString newCommand = QString("print -d 3 -t %1").arg(identifier);

    // Don't do anything if we're clicking the same identifier
    if (newCommand == tab.printCommand) {
      return;
    }

    // If we're in the middle of history, truncate forward history
    if (tab.historyIndex >= 0 && tab.historyIndex < tab.commandHistory.size()) {
      tab.commandHistory = tab.commandHistory.mid(0, tab.historyIndex + 1);
    }

    // Add current command to history
    tab.commandHistory.append(tab.printCommand);

    // Update to the new command and move forward in history
    tab.printCommand = newCommand;
    tab.commandEdit->setText(tab.printCommand);
    tab.historyIndex = tab.commandHistory.size();
    tab.backButton->setEnabled(true);
    refreshCurrentTab();
  }
}

void WorkingMemoryTree::goBack() {
  // Back button is handled per-tab in createTab lambda
}

void WorkingMemoryTree::updateCommand() {
  // Command updates are handled per-tab in createTab lambda
}
