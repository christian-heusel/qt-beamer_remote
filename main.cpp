#include "beamerfernbedienung.h"
#include <QApplication>
#include <QStyleFactory>

#include <QVBoxLayout>
#include <QPushButton>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    // qInfo() << QStyleFactory::keys();
    QApplication::setStyle("Fusion");
    QCoreApplication::setOrganizationName("Kirchengemeinde Öschingen");
    QCoreApplication::setOrganizationDomain("www.gemeinde.oeschingen.elk-wue.de");
    QCoreApplication::setApplicationName("Beamerfernbedienung");
    BeamerFernbedienung w;
    w.show();

    return a.exec();
}
