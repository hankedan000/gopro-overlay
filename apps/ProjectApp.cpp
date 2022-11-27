#include "projectwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ProjectWindow pw;
    pw.show();

    return a.exec();
}
