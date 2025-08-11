#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QTcpSocket>
#include <QTcpServer>
#include "ui_mainwindow.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QTcpSocket *socket = nullptr;
    QTcpServer *server = nullptr;
    QTcpSocket *clientSocket = nullptr;
    Ui::MainWindow *ui;
    void onSendButtonClicked();
    void onConnectClicked();
    void connectSocket(QTcpSocket *socket, QString ip, quint16 port);
    void onNewConnection();
    void onReadyRead();
    QTcpSocket *startServer(quint16 port);
};
#endif // MAINWINDOW_H
