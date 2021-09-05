#ifndef BEAMERFERNBEDIENUNG_H
#define BEAMERFERNBEDIENUNG_H

// Custom includes
#include "ui_beamerfernbedienung.h"

// QT includes
#include <QComboBox>
#include <QFile>
#include <QMap>
#include <QMessageBox>
#include <QObject>
#include <QSettings>
#include <QVector>
#include <QWidget>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpSocket>

// Includes from the Standard Template Library
#include <memory>

using std::make_unique;
using std::unique_ptr;

namespace Ui {
class BeamerFernbedienung;
}

class BeamerFernbedienung : public QWidget {
    Q_OBJECT

public:
    explicit BeamerFernbedienung(QWidget* parent = nullptr);
    virtual ~BeamerFernbedienung();

private slots:
    void on_avMute_clicked();
    void on_powerSwitch_clicked();
    void on_inputSelector_activated(int input);
    void on_reconnectButton_clicked();
    void on_lensSelector_currentTextChanged(const QString& arg1);
    void on_lensSelector_activated(int index);

    void on_horizontalSlider_Brightness_valueChanged(int value);

    void on_horizontalSlider_Contrast_valueChanged(int value);

    void on_focus_plus_clicked();
    void on_focus_minus_clicked();

    void on_zoom_plus_clicked();
    void on_zoom_minus_clicked();

private:
    unique_ptr<QTcpSocket> _beamerConnection;
    unique_ptr<QSettings> _settings;
    Ui::BeamerFernbedienung* _ui;

    QHostAddress _beamerAddress;
    quint16 _beamerPort;
    const QMap<QString, QString> _commands;
    QVector<QString> _lensSelectorSlotNames;
    QString lastResponse;
    unique_ptr<QTimer> _timer;
    // bool _power;
    bool _muted;
    int _inputSelector;
    int _lensSelector;
    int _brightness;
    int _contrast;
    int _timeoutTimer;

    enum PowerStatus {
        unknown,    //
        powerup,    //
        poweron,    //
        powerdown,  //
        poweroff    //
    } status;
    enum CommandType {
        question,
        exec,
        set,
    };

    bool _connected;

    QString sendCommand(const QString& cmd,
                        CommandType ctype = CommandType::question,
                        const QString& value = nullptr);
    void establishConnection();
    void loadSettings();
    void saveSettings();
    QString readAnswer();
    QString full_addr() const;
    void disableGui();
    void updateGui();
    void updateVar();
    void loadDarkSkin();

public slots:
    void yourSlot();
};

#endif  // BEAMERFERNBEDIENUNG_H
