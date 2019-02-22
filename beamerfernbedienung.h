#ifndef BEAMERFERNBEDIENUNG_H
#define BEAMERFERNBEDIENUNG_H

#include <QWidget>

namespace Ui {
class BeamerFernbedienung;
}

class BeamerFernbedienung : public QWidget
{
    Q_OBJECT

public:
    explicit BeamerFernbedienung(QWidget *parent = nullptr);
    ~BeamerFernbedienung();

private:
    Ui::BeamerFernbedienung *ui;
};

#endif // BEAMERFERNBEDIENUNG_H
