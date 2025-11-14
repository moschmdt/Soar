#ifndef TRACES_WINDOW_H
#define TRACES_WINDOW_H

#include <QWidget>

class TracesWindow : public QWidget
{
    Q_OBJECT
public:
    explicit TracesWindow(QWidget *parent = nullptr);
};

#endif // TRACES_WINDOW_H