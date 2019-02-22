#include "beamerfernbedienung.h"
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    // qInfo() << QStyleFactory::keys();
    QApplication::setStyle("Fusion");
    QCoreApplication::setOrganizationName("Kirchengemeinde Öschingen");
    QCoreApplication::setOrganizationDomain("www.gemeinde.oeschingen.elk-wue.de");
    QCoreApplication::setApplicationName("Beamerferbedienung");
    BeamerFernbedienung w;
    w.show();

    return a.exec();
}
