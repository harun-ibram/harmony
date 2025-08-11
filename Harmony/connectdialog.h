#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include <QDialog>

namespace Ui {
class connectDialog;
}

class connectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit connectDialog(QWidget *parent = nullptr);
    ~connectDialog();
    QString getIP() const;
    quint16 getPort() const;

private slots:
    void on_buttonBox_accepted();

private:
    Ui::connectDialog *ui;
};

#endif // CONNECTDIALOG_H
