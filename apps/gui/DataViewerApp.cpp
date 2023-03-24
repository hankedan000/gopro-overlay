#include <QApplication>

#include "DataViewerWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    DataViewerWindow dvw;
    dvw.setWindowIcon(QIcon(":/icons/gpo_icon.ico"));
    dvw.show();

    return a.exec();
}
