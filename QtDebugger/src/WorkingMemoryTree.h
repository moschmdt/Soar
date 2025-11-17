#ifndef WORKING_MEMORY_TREE_H
#define WORKING_MEMORY_TREE_H

#include <QLineEdit>
#include <QPushButton>
#include <QStyledItemDelegate>
#include <QTabWidget>
#include <QTreeWidget>
#include <QWidget>

class SoarAgent;

class HtmlDelegate : public QStyledItemDelegate {
  Q_OBJECT

public:
  explicit HtmlDelegate(QObject *parent = nullptr);
  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override;
  QSize sizeHint(const QStyleOptionViewItem &option,
                 const QModelIndex &index) const override;
};

class WorkingMemoryTree : public QWidget {
  Q_OBJECT

public:
  explicit WorkingMemoryTree(SoarAgent *agent, QWidget *parent = nullptr);

public slots:
  void refresh();
  void updateCommand();
  void onItemClicked(QTreeWidgetItem *item, int column);
  void goBack();
  void onTabChanged(int index);

private:
  // Per-tab data
  struct TabData {
    QTreeWidget *tree;
    QLineEdit *commandEdit;
    QPushButton *backButton;
    QString printCommand;
    QMap<QString, QTreeWidgetItem *> identifierMap;
    QStringList commandHistory;
    int historyIndex;
  };

  void createLayout();
  void populateTree();
  void addWMEToTree(QTreeWidgetItem *parent, void *identifier, int depth = 0);
  void parseAndBuildTree();
  QString makeIdentifierClickable(const QString &text);
  QString makeIdentifierClickableForTab(const QString &text, TabData &tab);
  void navigateToIdentifier(const QString &identifier);
  void createTab(const QString &label, const QString &defaultCommand);
  void refreshCurrentTab();

  SoarAgent *m_agent;
  QTabWidget *m_tabWidget;
  QTreeWidget *m_tree;
  QLineEdit *m_commandEdit;
  QPushButton *m_backButton;
  QPushButton *m_refreshButton;
  QString m_printCommand;
  QMap<QString, QTreeWidgetItem *> m_identifierMap;
  QStringList m_commandHistory;
  int m_historyIndex;
  QList<TabData> m_tabs;
};

#endif // WORKING_MEMORY_TREE_H
