#include "widget.h"
#include "FileFunctions.h"
#include "CreateLaunchScript.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return QApplication::exec();
}
