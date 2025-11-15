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
  void expandAll();
  void collapseAll();
  void updateCommand();

private:
  void createLayout();
  void populateTree();
  void addWMEToTree(QTreeWidgetItem *parent, void *identifier, int depth = 0);

  SoarAgent *m_agent;
  QTreeWidget *m_tree;
  QLineEdit *m_commandEdit;
  QPushButton *m_refreshButton;
  QPushButton *m_expandButton;
  QPushButton *m_collapseButton;
  QString m_printCommand;
};

#endif // WORKING_MEMORY_TREE_H
