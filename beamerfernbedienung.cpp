#include "beamerfernbedienung.h"
#include "ui_beamerfernbedienung.h"

BeamerFernbedienung::BeamerFernbedienung(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BeamerFernbedienung)
{
    ui->setupUi(this);
}

BeamerFernbedienung::~BeamerFernbedienung()
{
    delete ui;
}
