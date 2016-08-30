#include "GUI/PodnekoMainWindow.h"
#include <QApplication>

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PodnekoMainWindow w;
    w.show();
    
    return a.exec();
}

