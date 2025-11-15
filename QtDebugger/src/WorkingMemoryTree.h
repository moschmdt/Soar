#ifndef WORKING_MEMORY_TREE_H
#define WORKING_MEMORY_TREE_H

#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QWidget>

class SoarAgent;

class WorkingMemoryTree : public QWidget {
  Q_OBJECT

public:
  explicit WorkingMemoryTree(SoarAgent *agent, QWidget *parent = nullptr);

public slots:
  void refresh();
  void updateCommand();
  void onItemClicked(QTreeWidgetItem *item, int column);
  void goBack();

private:
  void createLayout();
  void populateTree();
  void addWMEToTree(QTreeWidgetItem *parent, void *identifier, int depth = 0);
  void parseAndBuildTree();
  QString makeIdentifierClickable(const QString &text);
  void navigateToIdentifier(const QString &identifier);

  SoarAgent *m_agent;
  QTreeWidget *m_tree;
  QLineEdit *m_commandEdit;
  QPushButton *m_backButton;
  QPushButton *m_refreshButton;
  QString m_printCommand;
  QMap<QString, QTreeWidgetItem *> m_identifierMap;
  QStringList m_commandHistory;
  int m_historyIndex;
};

#endif // WORKING_MEMORY_TREE_H
