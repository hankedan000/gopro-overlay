#include "projectwindow.h"

#include <easy/profiler.h>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    profiler::startListen();

    ProjectWindow pw;
    pw.setWindowIcon(QIcon(":/icons/gpo_icon.ico"));
    pw.show();

    return a.exec();
}
