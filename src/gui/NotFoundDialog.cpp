#include "NotFoundDialog.hpp"
#include "ui_NotFoundDialog.h"

SigEstNotFound::SigEstNotFound(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SigEstNotFound)
{
    ui->setupUi(this);
}

SigEstNotFound::~SigEstNotFound()
{
    delete ui;
}

void SigEstNotFound::on_pushButton_clicked()
{
    this->close();
}