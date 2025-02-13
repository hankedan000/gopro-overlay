#include "projectwindow.h"

#include <tracy/Tracy.hpp>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
#ifdef TRACY_ENABLE
    tracy::StartupProfiler();
#endif

    ProjectWindow pw;
    pw.setWindowIcon(QIcon(":/icons/gpo_icon.ico"));
    pw.show();

    return a.exec();
}
