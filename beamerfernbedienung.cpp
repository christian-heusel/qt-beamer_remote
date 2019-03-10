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
                {"avMute", "pic.mute"},
                {"inputSelector", "input"},
                {"powerSwitch", "power"},
                {"lensSelector", "recall.mem"}
            }
        ),
        _lensSelectorSlotNames(
            {{
                "Test0",
                "Test1",
                "Test2",
                "Test3",
            }}
        )
    {
        _ui->setupUi(this);
        setWindowTitle("Beamerfernbedienung");
        establishConnection();
        // Setting up all inputs
        _ui->inputSelector->addItems(
            {
                "HDMI 1",
                "HDMI 2",
                "DVI-D",
                "VGA",
                "Component",
                "HDBaseT"
            }
        );
        //_ui->lensSelector->setEditable(false);
        loadSettings();
        // Setting lens Names
        for(const auto& name : _lensSelectorSlotNames) {
            _ui->lensSelector->addItem(name);
        }
        // Checking current status
        _power = true;
        _muted = true;
        on_avMute_clicked();
        on_powerSwitch_clicked();
    }

void BeamerFernbedienung::loadSettings() {
    _settings = make_unique<QSettings>();
    _settings->setPath(QSettings::IniFormat, QSettings::UserScope, "settings.ini");

    qDebug() << "read settings: " << _settings->fileName();
    // Read in the Slot names
    int size = _settings->beginReadArray("lensSelectorSlotNames");
    if(_lensSelectorSlotNames.size() < size)
        _lensSelectorSlotNames.resize(size);
    for(int i = 0; i < size; i++) {
        _settings->setArrayIndex(i);
        _lensSelectorSlotNames[i] = _settings->value("Slot").toString();
    }
    _settings->endArray();

    _settings->beginGroup("MainWindow");
    resize(_settings->value("size", QSize(400, 400)).toSize());
    move(_settings->value("pos", QPoint(200, 200)).toPoint());
    _settings->endGroup();



    _settings->beginGroup("Connection");
    //_beamerAddress(_settings->value("IP"));
    //_beamerPort = _settings->value("port");
    _settings->endGroup();

}

void BeamerFernbedienung::saveSettings() {
    qint8 counter = 0;
    qDebug() << "write settings: " << _settings->fileName();
    _settings->beginWriteArray("lensSelectorSlotNames");
    for(const auto& name : _lensSelectorSlotNames) {
        _settings->setArrayIndex(counter++);
        if(_settings->value("Slot").toString() != name && name != "")
            _settings->setValue("Slot", name);
    }
    _settings->endArray();


    _settings->beginGroup("MainWindow");
    _settings->setValue("size", size());
    _settings->setValue("pos", pos());
    _settings->endGroup();


    _settings->beginGroup("Connection");
    _settings->setValue("IP", _beamerAddress.toString());
    _settings->setValue("port", _beamerPort);
    _settings->endGroup();

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
    saveSettings();
    establishConnection();
}

void BeamerFernbedienung::on_inputSelector_activated(int input) {
     sendCommand(_commands["inputSelector"], QString::number(input));
     readAnswer();
}

QString BeamerFernbedienung::full_addr() const {
    return _beamerAddress.toString() + ":" + QString::number(_beamerPort);
}

void BeamerFernbedienung::on_lensSelector_currentTextChanged(const QString& arg1) {
   auto index = _ui->lensSelector->currentIndex();
   _lensSelectorSlotNames[index] = arg1;
}

void BeamerFernbedienung::on_lensSelector_activated(int index) {
    sendCommand(_commands["lensSelector"],QString::number(index));
    readAnswer();
}

BeamerFernbedienung::~BeamerFernbedienung(){
    saveSettings();
    delete _ui;
}
