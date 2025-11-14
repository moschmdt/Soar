#ifndef PREFERENCES_WINDOW_H
#define PREFERENCES_WINDOW_H

#include <QWidget>

class PreferencesWindow : public QWidget
{
    Q_OBJECT
public:
    explicit PreferencesWindow(QWidget *parent = nullptr);
};

#endif // PREFERENCES_WINDOW_H