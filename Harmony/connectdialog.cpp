#include "connectdialog.h"
#include "ui_connectdialog.h"

connectDialog::connectDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::connectDialog)
{
    ui->setupUi(this);
}

connectDialog::~connectDialog()
{
    delete ui;
}

QString connectDialog::getIP() const {
    return ui->ip_addr->text();
}

quint16 connectDialog::getPort() const {
    return ui->port->text().toUShort();
}

void connectDialog::on_buttonBox_accepted()
{

}

