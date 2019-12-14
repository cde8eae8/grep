#include "grep.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Grep w;
    w.show();
    QObject::connect(&a, SIGNAL(aboutToQuit()), &w, SLOT(closing()));
    return a.exec();
}
