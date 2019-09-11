#include "beamerfernbedienung.h"
#include <QThread>
#include <QTimer>

BeamerFernbedienung::BeamerFernbedienung(QWidget *parent)
    :
        QWidget(parent),
        _beamerConnection(make_unique<QTcpSocket>(this)),
        _ui(new Ui::BeamerFernbedienung()),
        _beamerAddress("192.168.0.38"),
        //_beamerAddress("127.0.0.1"),
        _beamerPort(7000),
        _commands(
            {
                {"avMute", "pic.mute"},
                {"inputSelector", "input"},
                {"powerSwitch", "power"},
                {"brightness", "brightness"},
                {"contrast", "contrast"},
                {"lensSelector", "lens.load"},
                {"signal", "signal"}
            }
        ),
        _lensSelectorSlotNames(
            {{
                "Mem 1 - Leinwand oben",
                "Mem 2",
                "Mem 3 - Godi unten",
                "Mem 4 - Godi unten",
                "Mem 5 - Godi oben",
                "Mem 6 - Leinwand unten",
                "Mem 7",
                "Mem 8",
                "Mem 9",
                "Mem 10 - Standard -130 DBI",
            }}
        )
        // End of the constructur initialization list
    {
        _ui->setupUi(this);
        _timer = new QTimer();
        _timer->stop();
        _timer->setInterval(1000);
        connect(_timer, SIGNAL(timeout()), this,SLOT(yourSlot()));
        loadDarkSkin();
        QString version = "V1.1.0";
        setWindowTitle("Beamerfernbedienung " + version);

        _ui->label->setText("Beamerfernbedienung " + version);
        establishConnection();
        if(_connected)
        {
            _ui->label_Status->setText(full_addr());
        }
        else
        {
            _ui->label_Status->setText("Beamer nicht gefunden! ");
        }
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
        _ui->lensSelector->setEditable(false);
        loadSettings();
        // Setting lens Names
        for(const auto& name : _lensSelectorSlotNames) {
            _ui->lensSelector->addItem(name);
        }
        _lensSelector = 0;
        bool pow = sendCommand(_commands["powerSwitch"]).toInt();
        if (pow)
        {
            status = poweron;
        }
        else
        {
            status =poweroff;
        }
        disableGui();
        updateVar();
        updateGui();
    }

void BeamerFernbedienung::loadDarkSkin() {
    QFile f("qdarkstyle/style.qss");
    if (! f.exists()) {
        #ifdef QT_DEBUG
        qInfo() << "Unable to set stylesheet, file not found!";
        #endif
    }
    else {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());

        #ifdef QT_DEBUG
        qInfo() << "Stylesheet geladen!";
        #endif
    }
}
void BeamerFernbedienung::updateVar(){
    // Checking current status
    //_power = sendCommand(_commands["powerSwitch"]).toInt();

    if (status == poweron)
    {
        _muted = sendCommand(_commands["avMute"]).toInt();
        _inputSelector= sendCommand(_commands["inputSelector"]).toInt();
        _brightness = sendCommand(_commands["brightness"]).toInt();
        _contrast = sendCommand(_commands["contrast"]).toInt();
    }
}

void BeamerFernbedienung::loadSettings() {
    _settings = make_unique<QSettings>();
    _settings->setPath(QSettings::IniFormat, QSettings::UserScope, "settings.ini");

    #ifdef QT_DEBUG
    qInfo() << "read settings: " << _settings->fileName();
    #endif

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
    //_beamerAddress(_settings->value("IP").toString());
    _beamerPort = static_cast<quint16>(_settings->value("port").toUInt());
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
        _connected = true;
       // QMessageBox::information(
       //         this,
       //         tr("Beamerfernbedienung"),
       //         "Verbunden mit " + full_addr() + "!");
    }
    else {
        QString error = _beamerConnection->errorString();
        _connected = false;
        QMessageBox::critical(
                this,
                tr("Beamerfernbedienung"),
                "Beamer nicht gefunden " + full_addr() + "!\n"
                + error);
    }

    if(_connected) {
        _ui->reconnectButton->setEnabled(false);
    }
}

QString BeamerFernbedienung::sendCommand(const QString& cmd, const QString& value) {

    QString pre = "*";

    QString suf = " ?\r";

    if (value != nullptr){
         suf = " = " + value + "\r";
    }

    QByteArray data = pre.toUtf8() + cmd.toUtf8() + suf.toUtf8();

    lastResponse.clear();
    //if(_beamerConnection->state() == QAbstractSocket::ConnectedState)
    {
        _beamerConnection->readAll();
         _beamerConnection->write(data);
         _beamerConnection->waitForBytesWritten(1000);

       // QThread::msleep(500);
         #ifdef QT_DEBUG
         qInfo() << data;
         #endif

         QByteArray buffer;
         buffer.clear();

         _beamerConnection->waitForReadyRead(5000);
             buffer =_beamerConnection->readLine(128);
             qInfo() << buffer;
             lastResponse = buffer;

    }
    #ifdef QT_DEBUG
    qInfo() << "DEBUG: command -> " << data;
    #endif

    if ((lastResponse.contains("NACK") | (!lastResponse.contains(cmd)))) {

        #ifdef QT_DEBUG
        qInfo() << "DEBUG: response NACK ->" << lastResponse;
        #endif
        //Exception??
        return "";
    }

    QRegExp rx("(\\ |\\r)"); //RegEx for ' ' or ',' or '.' or ':' or '\r'
    QStringList query = lastResponse.split(rx);

    foreach(QString s, query) {
         qInfo() << "DEBUG: response ACK -> " << s;
    }
    return query[3];
}


void BeamerFernbedienung::disableGui(){

    _ui->powerSwitch->setEnabled(false);
    _ui->avMute->setEnabled(false);
    _ui->label_2->setEnabled(false);
    _ui->label_3->setEnabled(false);
    _ui->lensSelector->setEnabled(false);
    _ui->inputSelector->setEnabled(false);
    _ui->horizontalSlider_Contrast->setEnabled(false);
    _ui->horizontalSlider_Brightness->setEnabled(false);
}
void BeamerFernbedienung::updateGui(){

    if (_connected)
    {
        _ui->powerSwitch->setEnabled(true);
    }

    if(status == poweron) {
        _ui->powerSwitch->setText("Ausschalten");

        _ui->label_2->setEnabled(true);
        _ui->label_3->setEnabled(true);
        _ui->avMute->setEnabled(true);
        _ui->lensSelector->setEnabled(true);
        _ui->inputSelector->setEnabled(true);
        _ui->horizontalSlider_Contrast->setEnabled(true);
        _ui->horizontalSlider_Brightness->setEnabled(true);


        if (_muted) {
            _ui->avMute->setText("Bild reaktivieren");
        }
        else {
            _ui->avMute->setText("Bild schwarz");
        }

        _ui->inputSelector->setCurrentIndex(_inputSelector);
        _ui->lensSelector->setCurrentIndex(_lensSelector);
        _ui->horizontalSlider_Contrast->setValue(_contrast);
        _ui->horizontalSlider_Contrast->setToolTip( QString::number(_contrast));

        _ui->horizontalSlider_Brightness->setValue(_brightness);
        _ui->horizontalSlider_Brightness->setToolTip( QString::number(_brightness));
    }
    else {
        _ui->powerSwitch->setText("Anschalten");
    }

}

void BeamerFernbedienung::on_avMute_clicked() {
    disableGui();
    _muted = sendCommand(_commands["avMute"], QString::number(!_muted)).toInt();
    updateGui();
}

 void BeamerFernbedienung::yourSlot(){


     bool power = sendCommand(_commands["powerSwitch"]).toInt();

     if (power && ( status == powerup))
     {
         status = poweron;
        _ui->label_Status->setText("Beamer ist betriebsbereit");
        qApp->processEvents();
         sendCommand(_commands["inputSelector"],QString::number(_ui->inputSelector->currentIndex()));
        _timer->stop();
        updateVar();
        updateGui();
     }
     else if (!power && ( status == powerdown))
     {
         status = poweroff;
        _ui->label_Status->setText("Beamer ist ausgeschaltet");
        qApp->processEvents();
        _timer->stop();
        updateVar();
        updateGui();
     }
     else
     {
         QString s ;

         if (status == powerup)
         {
             s = "Beamer wird gestartet: " ;
         }
         else
         {
             s = "Beamer schaltet ab: " ;
         }
         s.append(QString::number(++_timeoutTimer));
         s.append(" sec");
         _ui->label_Status->setText(s);

         //qInfo() << "DEBUG: Timer " << _timeoutTimer << " sec";
         //QTimer::singleShot(1000, this, SLOT(yourSlot()));

     }
}
void BeamerFernbedienung::on_powerSwitch_clicked() {
    disableGui();
    bool power = sendCommand(_commands["powerSwitch"], QString::number(!(status == poweron))).toInt();
    //_power = sendCommand(_commands["status"]).toInt();
    _timeoutTimer = 0;

    if (power)
        status = powerup;
    else
        status = powerdown;


    QString s ;
    if (status == powerup)
    {
        s = "Beamer wird gestartet: " ;
    }
    else
    {
        s = "Beamer schaltet ab: " ;
    }
    s.append(QString::number(_timeoutTimer));
    s.append(" sec");
    _ui->label_Status->setText(s);
    _timer->start();
}

void BeamerFernbedienung::on_reconnectButton_clicked() {
    disableGui();
    saveSettings();
    establishConnection();

    updateVar();
    updateGui();
}

void BeamerFernbedienung::on_inputSelector_activated(int input) {
disableGui();
    _inputSelector = sendCommand(_commands["inputSelector"], QString::number(input)).toInt();
    updateVar();
updateGui();
}

QString BeamerFernbedienung::full_addr() const {
    return _beamerAddress.toString() + ":" + QString::number(_beamerPort);
}

void BeamerFernbedienung::on_lensSelector_currentTextChanged(const QString& arg1) {
    auto index = _ui->lensSelector->currentIndex();
    _lensSelectorSlotNames[index] = arg1;
}

void BeamerFernbedienung::on_lensSelector_activated(int index) {
   _lensSelector = sendCommand(_commands["lensSelector"],QString::number(index)).toInt();
}

BeamerFernbedienung::~BeamerFernbedienung(){
    saveSettings();
    delete _ui;
}

void BeamerFernbedienung::on_horizontalSlider_Brightness_valueChanged(int value)
{
    _brightness = sendCommand(_commands["brightness"], QString::number(value)).toInt();
    _ui->horizontalSlider_Brightness->setToolTip( QString::number(_brightness));
}



void BeamerFernbedienung::on_horizontalSlider_Contrast_valueChanged(int value)
{
    _contrast = sendCommand(_commands["contrast"], QString::number(value)).toInt();
    _ui->horizontalSlider_Contrast->setToolTip( QString::number(_contrast));
}
