#ifndef PROPIETE_H
#define PROPIETE_H

#include <QDialog>

namespace Ui {
class Propiete;
}

class Propiete : public QDialog
{
    Q_OBJECT

public:
    explicit Propiete(QWidget *parent = nullptr);
    ~Propiete();
    int getWidth();
    int getHeight();


private:
    Ui::Propiete *ui;

};

#endif // PROPIETE_H
