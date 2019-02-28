#include "beamerfernbedienung.h"

QByteArray IntToArray(qint32 source) {
    QByteArray temp;
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << source;
    return temp;
}


BeamerFernbedienung::BeamerFernbedienung(QWidget *parent)
    :
        QWidget(parent),
        _beamerConnection(make_unique<QTcpSocket>(this)),
        _ui(new Ui::BeamerFernbedienung()),
        // _beamerAddress("192.168.0.100"),
        _beamerAddress("127.0.0.1"),
        _beamerPort(7000),
        _commands(
            {
                {"avMute","pic.mute"},
                {"inputSelector","input"},
                {"powerSwitch","power"}
            }
        ),
        _lensSelectorSlotNames( )
    {
        _ui->setupUi(this);
        setWindowTitle("Beamerfernbedienung");
        establishConnection();
        // Setting up all inputs
        _ui->inputSelector->addItem(tr("HDMI 1"));
        _ui->inputSelector->addItem(tr("HDMI 2"));
        _ui->inputSelector->addItem(tr("DVI-D"));
        _ui->inputSelector->addItem(tr("VGA"));
        _ui->inputSelector->addItem(tr("Component"));
        _ui->inputSelector->addItem(tr("HDBaseT"));
        // Setting lens Names
        _ui->lensSelector->setEditable(true);
        loadSettings();
        for(const auto& name : _lensSelectorSlotNames) {
            _ui->lensSelector->addItem(name);
        }
        // Checking current status
        _power = true;
        _muted = true;
        // Setting all textfields
    }

void BeamerFernbedienung::loadSettings() {
    QString path = "/home/chris/Documents/QTCreator/BeamerFernbedienung/settings.ini";
    _settings = make_unique<QSettings>(path, QSettings::NativeFormat);
    // Read in the Slot names
    int size = _settings->beginReadArray("lensSelectorSlotNames");
    for(int i = 0; i < size; i++) {
        _settings->setArrayIndex(i);
        _lensSelectorSlotNames.append(_settings->value("Slot").toString());
    }
    _settings->endArray();
}

void BeamerFernbedienung::saveSettings() {
    qint8 counter = 0;
    _settings->beginWriteArray("lensSelectorSlotNames");
    for(const auto& name : _lensSelectorSlotNames) {
        _settings->setArrayIndex(counter++);
        if(_settings->value("Slot").toString() != name)
            _settings->setValue("Slot", name);
    }
    _settings->endArray();
}

void BeamerFernbedienung::establishConnection() {
    _beamerConnection->connectToHost(_beamerAddress, _beamerPort);
    if(_beamerConnection->waitForConnected()) {
        _connected =true;
        QMessageBox::information(
                this,
                tr("Beamerfernbedienung"),
                "Successfully connected to " + full_addr() + "!");
    }
    else {
        QString error = _beamerConnection->errorString();
        _connected = false;
        QMessageBox::critical(
                this,
                tr("Beamerfernbedienung"),
                "Error while connecting to " + full_addr() + "!\n"
                + error);
    }
    if(_connected) {
        _ui->reconnectButton->setEnabled(false);
    }
}

QString BeamerFernbedienung::readAnswer(){
    QByteArray buffer;
    buffer.append(_beamerConnection->readAll());
    const QString answer = buffer;
    #ifdef QT_DEBUG
    qInfo() << "DEBUG: answer  <- " << answer;
    #endif
    // const QStringRef executionStatus(&answer,0,3);
    // if(executionStatus.contains("ACK", Qt::CaseInsensitive))
    //     qInfo() << "Yay";
    // else if (executionStatus.contains("NAK", Qt::CaseInsensitive)) {
    //     qInfo() << "Nayy";
    // }
    // QString number = answer.back();
    // int result = number.toInt();
    // qInfo() << result << number;
    return answer;
}

void BeamerFernbedienung::sendCommand(const QString& cmd, const QString& value) {
    QString pre = "*";
    QString suf = "=" + value + "\r";
    QByteArray data = pre.toUtf8() + cmd.toUtf8() + suf.toUtf8();
    if(_beamerConnection->state() == QAbstractSocket::ConnectedState) {
        _beamerConnection->write(IntToArray(data.size()));
        _beamerConnection->write(data);
    }
    #ifdef QT_DEBUG
    qInfo() << "DEBUG: command -> " << pre.toUtf8() + cmd.toUtf8() + suf.toUtf8();
    #endif
}

void BeamerFernbedienung::on_avMute_clicked() {
    if(_muted) {
        _ui->avMute->setText("Mute");
    }
    else {
        _ui->avMute->setText("Unmute");
    }
    _muted = ! _muted;
    sendCommand(_commands["avMute"], QString::number(_muted));
    readAnswer();
}

void BeamerFernbedienung::on_powerSwitch_clicked() {
    if(! _power) {
        _ui->powerSwitch->setText("Anschalten");
    }
    else {
        _ui->powerSwitch->setText("Ausschalten");
    }
    _power = ! _power;
    sendCommand(_commands["powerSwitch"], QString::number(_power));
    readAnswer();
}

void BeamerFernbedienung::on_reconnectButton_clicked() {
    establishConnection();
}

void BeamerFernbedienung::on_inputSelector_activated(int input) {
     sendCommand(_commands["inputSelector"], QString::number(input));
     readAnswer();
}

QString BeamerFernbedienung::full_addr() const {
    return _beamerAddress.toString() + ":" + QString::number(_beamerPort);
}

void BeamerFernbedienung::on_lensSelector_currentIndexChanged(const QString &arg1) {
}

void BeamerFernbedienung::on_lensSelector_currentTextChanged(const QString& arg1) {
   auto index = _ui->lensSelector->currentIndex();
   _lensSelectorSlotNames[index] = arg1;
}

BeamerFernbedienung::~BeamerFernbedienung(){
    saveSettings();
    delete _ui;
}
