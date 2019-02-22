#include "beamerfernbedienung.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BeamerFernbedienung w;
    w.show();

    return a.exec();
}
