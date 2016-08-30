#include "PodnekoMainWindow.h"
#include "ui_PodnekoMainWindow.h"

PodnekoMainWindow::PodnekoMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PodnekoMainWindow)
{
    ui->setupUi(this);

    setWindowIcon(QIcon(":/podneko.png"));
}

PodnekoMainWindow::~PodnekoMainWindow()
{
    delete ui;
}
