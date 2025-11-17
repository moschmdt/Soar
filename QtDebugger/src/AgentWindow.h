#ifndef AGENT_WINDOW_H
#define AGENT_WINDOW_H

#include <QSplitter>
#include <QWidget>

class SoarAgent;
class WorkingMemoryTree;
class ProductionsWindow;
class OutputWindow;

/**
 * @brief Window for displaying and controlling a single Soar agent
 */
class AgentWindow : public QWidget {
  Q_OBJECT

public:
  explicit AgentWindow(SoarAgent *agent, QWidget *parent = nullptr);
  ~AgentWindow();

  SoarAgent *agent() const { return m_agent; }

private:
  void createLayout();

  SoarAgent *m_agent;

  // Main splitters
  QSplitter *m_mainSplitter;
  QSplitter *m_leftSplitter;
  QSplitter *m_rightSplitter;

  // Component widgets
  WorkingMemoryTree *m_wmTree;
  ProductionsWindow *m_productionsWindow;
  OutputWindow *m_outputWindow;
};

#endif // AGENT_WINDOW_H
