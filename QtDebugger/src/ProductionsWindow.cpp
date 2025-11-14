#include "ProductionsWindow.h"
#include "SoarAgent.h"

#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QSplitter>
#include <QVBoxLayout>

ProductionsWindow::ProductionsWindow(SoarAgent *agent, QWidget *parent)
    : QWidget(parent), m_agent(agent), m_filterEdit(nullptr),
      m_productionList(nullptr), m_productionDetails(nullptr),
      m_refreshButton(nullptr) {
  createLayout();
  loadProductions();
}

void ProductionsWindow::createLayout() {
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(4, 4, 4, 4);

  // Header with filter and refresh
  QHBoxLayout *headerLayout = new QHBoxLayout();
  QLabel *label = new QLabel("Productions");
  label->setStyleSheet("font-weight: bold;");
  headerLayout->addWidget(label);
  headerLayout->addStretch();

  m_refreshButton = new QPushButton("Refresh");
  m_refreshButton->setMaximumWidth(80);
  connect(m_refreshButton, &QPushButton::clicked, this,
          &ProductionsWindow::refresh);
  headerLayout->addWidget(m_refreshButton);

  layout->addLayout(headerLayout);

  // Filter box
  QHBoxLayout *filterLayout = new QHBoxLayout();
  filterLayout->addWidget(new QLabel("Filter:"));
  m_filterEdit = new QLineEdit();
  m_filterEdit->setPlaceholderText("Search productions...");
  connect(m_filterEdit, &QLineEdit::textChanged, this,
          &ProductionsWindow::filterProductions);
  filterLayout->addWidget(m_filterEdit);
  layout->addLayout(filterLayout);

  // Splitter for list and details
  QSplitter *splitter = new QSplitter(Qt::Vertical);

  // Production list
  m_productionList = new QListWidget();
  m_productionList->setAlternatingRowColors(true);
  connect(m_productionList, &QListWidget::currentRowChanged, this,
          &ProductionsWindow::showProductionDetails);
  splitter->addWidget(m_productionList);

  // Production details
  m_productionDetails = new QTextEdit();
  m_productionDetails->setReadOnly(true);
  m_productionDetails->setFont(QFont("Courier New", 10));
  splitter->addWidget(m_productionDetails);

  splitter->setStretchFactor(0, 1);
  splitter->setStretchFactor(1, 2);

  layout->addWidget(splitter);
}

void ProductionsWindow::refresh() { loadProductions(); }

void ProductionsWindow::loadProductions() {
  m_productionList->clear();
  m_productionDetails->clear();

  if (!m_agent)
    return;

  QString result = m_agent->executeCommand("print --name");
  QStringList productions = result.split('\n');

  for (const QString &prod : productions) {
    QString trimmed = prod.trimmed();
    if (!trimmed.isEmpty() && !trimmed.startsWith("#")) {
      m_productionList->addItem(trimmed);
    }
  }
}

void ProductionsWindow::filterProductions(const QString &filter) {
  for (int i = 0; i < m_productionList->count(); ++i) {
    QListWidgetItem *item = m_productionList->item(i);
    if (filter.isEmpty()) {
      item->setHidden(false);
    } else {
      item->setHidden(!item->text().contains(filter, Qt::CaseInsensitive));
    }
  }
}

void ProductionsWindow::showProductionDetails() {
  QListWidgetItem *item = m_productionList->currentItem();
  if (!item || !m_agent) {
    m_productionDetails->clear();
    return;
  }

  QString prodName = item->text();
  QString result = m_agent->executeCommand("print " + prodName);
  m_productionDetails->setPlainText(result);
}
