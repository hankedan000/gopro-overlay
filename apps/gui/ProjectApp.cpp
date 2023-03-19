#include "projectwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ProjectWindow pw;
    pw.setWindowIcon(QIcon(":/icons/gpo_icon.ico"));
    pw.show();

    return a.exec();
}
