#ifndef PODNEKOMAINWINDOW_H
#define PODNEKOMAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class PodnekoMainWindow;
}

class PodnekoMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PodnekoMainWindow(QWidget *parent = 0);
    ~PodnekoMainWindow();

private:
    Ui::PodnekoMainWindow *ui;
};

#endif // PODNEKOMAINWINDOW_H
