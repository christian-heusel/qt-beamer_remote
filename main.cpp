#include <QApplication>
#include <QStyleFactory>

#include "beamerfernbedienung.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QApplication::setStyle("Fusion");
    QCoreApplication::setOrganizationName("Kirchengemeinde_Oeschingen");
    QCoreApplication::setOrganizationDomain(
        "www.gemeinde.oeschingen.elk-wue.de");
    QCoreApplication::setApplicationName("Beamerfernbedienung");
    BeamerFernbedienung w;
    w.showNormal();

    return a.exec();
}
