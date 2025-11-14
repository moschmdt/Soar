#ifndef PRODUCTIONS_WINDOW_H
#define PRODUCTIONS_WINDOW_H

#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>

class SoarAgent;

class ProductionsWindow : public QWidget {
  Q_OBJECT

public:
  explicit ProductionsWindow(SoarAgent *agent, QWidget *parent = nullptr);

public slots:
  void refresh();
  void filterProductions(const QString &filter);
  void showProductionDetails();

private:
  void createLayout();
  void loadProductions();

  SoarAgent *m_agent;
  QLineEdit *m_filterEdit;
  QListWidget *m_productionList;
  QTextEdit *m_productionDetails;
  QPushButton *m_refreshButton;
};

#endif // PRODUCTIONS_WINDOW_H
