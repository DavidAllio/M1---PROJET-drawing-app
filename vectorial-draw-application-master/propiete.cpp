#include "propiete.h"
#include "ui_propiete.h"
#include "drawzone.h"

Propiete::Propiete(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Propiete)
{
    ui->setupUi(this);

    int larg = drawZone::getWidth();
    int haut = drawZone::getHeight();
    ui->LargeurNumber->setValue(larg);
    ui->HauteurNumber->setValue(haut);

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

int Propiete::getWidth(){
    return ui->LargeurNumber->value();
}

int Propiete::getHeight(){
    return ui->HauteurNumber->value();
}

Propiete::~Propiete()
{
    delete ui;
}

