#ifndef BEAMERFERNBEDIENUNG_H
#define BEAMERFERNBEDIENUNG_H

// QT includes
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpSocket>
#include <QComboBox>
#include <QMessageBox>
#include <QWidget>
#include <QtCore>
#include <QMap>

// Includes from the Standard Template Library
#include <memory>

using std::unique_ptr;
using std::make_unique;

namespace Ui {
class BeamerFernbedienung;
}

class BeamerFernbedienung : public QWidget {
    Q_OBJECT

public:
    explicit BeamerFernbedienung(QWidget *parent = nullptr);
    ~BeamerFernbedienung();

private slots:
    void on_avMute_clicked();
    void on_powerSwitch_clicked();

private:
    unique_ptr<Ui::BeamerFernbedienung> _ui;
    unique_ptr<QTcpSocket> _beamerConnection;

    const QHostAddress _beamerAddress;
    const quint16 _beamerPort;
    const QMap<QString,QString> _commands;

    bool _power;
    bool _muted;

    void establishConnection();
    void sendCommand(const QString& cmd, const QString& value);
    QString full_addr() const;
};

#endif // BEAMERFERNBEDIENUNG_H
