#include "beamerfernbedienung.h"
#include "ui_beamerfernbedienung.h"

QByteArray IntToArray(qint32 source) {
    QByteArray temp;
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << source;
    return temp;
}

BeamerFernbedienung::BeamerFernbedienung(QWidget *parent)
    :
        QWidget(parent),
        _ui(make_unique<Ui::BeamerFernbedienung>()),
        _beamerConnection(make_unique<QTcpSocket>(this)),
        // _beamerAddress("192.168.0.100"),
        _beamerAddress("127.0.0.1"),
        _beamerPort(7000),
        _commands(
            {
                {"avMute","pic.mute"},
                {"inputSelector","input"},
                {"powerSwitch","power"}
            }
        )
    {
        _ui->setupUi(this);
        establishConnection();
        // Setting up all inputs
        _ui->inputSelector->addItem(tr("HDMI 1"));
        _ui->inputSelector->addItem(tr("HDMI 2"));
        _ui->inputSelector->addItem(tr("DVI-D"));
        _ui->inputSelector->addItem(tr("VGA"));
        _ui->inputSelector->addItem(tr("Component"));
        _ui->inputSelector->addItem(tr("HDBaseT"));
        // Checking current status
        /* TODO */
        _power = true;
        _muted = true;
        // Setting all textfields
        /* TODO */
    }

void BeamerFernbedienung::establishConnection() {
    _beamerConnection->connectToHost(_beamerAddress, _beamerPort);
    if(_beamerConnection->waitForConnected()) {
        QMessageBox::information(
                this,
                tr("Application Name"),
                "Successfully connected to " + full_addr() + "!");
    }
    else {
        QString error = _beamerConnection->errorString();
        QMessageBox::critical(
                this,
                tr("Application Name"),
                "Error while connecting to " + full_addr() + "!\n"
                + error);
    }
}

void BeamerFernbedienung::sendCommand(const QString& cmd, const QString& value) {
    QString pre = "*";
    QString suf = "=" + value + "\r";
    QByteArray data = pre.toUtf8() + cmd.toUtf8() + suf.toUtf8();
    if(_beamerConnection->state() == QAbstractSocket::ConnectedState) {
        _beamerConnection->write(IntToArray(data.size()));
        _beamerConnection->write(data);
    }
}

void BeamerFernbedienung::on_avMute_clicked() {
    if(_muted) {
        _ui->avMute->setText("Mute");
    }
    else {
        _ui->avMute->setText("Unmute");
    }
    _muted = not _muted;
    sendCommand(_commands["avMute"], QString::number(_muted));
}

void BeamerFernbedienung::on_powerSwitch_clicked() {
    if(not _power) {
        _ui->powerSwitch->setText("Anschalten");
    }
    else {
        _ui->powerSwitch->setText("Ausschalten");
    }
    _power = not _power;
    sendCommand(_commands["powerSwitch"], QString::number(_power));
}

void BeamerFernbedienung::on_inputSelector_activated(int input) {
     sendCommand(_commands["inputSelector"], QString::number(input));
}

QString BeamerFernbedienung::full_addr() const {
    return _beamerAddress.toString() + ":" + QString::number(_beamerPort);
}

BeamerFernbedienung::~BeamerFernbedienung(){}
