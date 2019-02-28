#ifndef BEAMERFERNBEDIENUNG_H
#define BEAMERFERNBEDIENUNG_H

// Custom includes
#include "ui_beamerfernbedienung.h"

// QT includes
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpSocket>
#include <QMessageBox>
#include <QSettings>
#include <QComboBox>
#include <QWidget>
#include <QObject>
#include <QVector>
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
    virtual ~BeamerFernbedienung();

private slots:
    void on_avMute_clicked();
    void on_powerSwitch_clicked();
    void on_inputSelector_activated(int input);
    void on_reconnectButton_clicked();

    void on_lensSelector_currentIndexChanged(const QString &arg1);

    void on_lensSelector_currentTextChanged(const QString &arg1);

private:
    unique_ptr<QTcpSocket> _beamerConnection;
    unique_ptr<QSettings> _settings;
    Ui::BeamerFernbedienung* _ui;

    const QHostAddress _beamerAddress;
    const quint16 _beamerPort;
    const QMap<QString,QString> _commands;
    QVector<QString> _lensSelectorSlotNames;

    bool _power;
    bool _muted;
    bool _connected;

    void establishConnection();
    void sendCommand(const QString& cmd, const QString& value);
    void lensSelector_currentTextChanged(const QString& input);
    void loadSettings();
    void saveSettings();
    QString readAnswer();
    QString full_addr() const;
};

#endif // BEAMERFERNBEDIENUNG_H
