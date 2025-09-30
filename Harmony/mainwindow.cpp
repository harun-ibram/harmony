#include <QDebug>
#include <QTcpSocket>
#include <QInputEvent>
#include <QTcpServer>
#include <QMessageBox>
#include "mainwindow.h"
#include "./ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->sendButton, &QPushButton::clicked, this, &MainWindow::onSendButtonClicked);

    server = new QTcpSocket(this);



    connect(server, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(server, &QTcpSocket::connected, this, [](){
        qDebug() << "Connected to server";
    });
    connect(server, &QTcpSocket::disconnected, this, [](){
        qDebug() << "Disconnected from server";
    });
    server->connectToHost("127.0.0.1", 5002);
    ui->messageDisplay->setIconSize(QSize(48, 48));
    ui->messageEdit->installEventFilter(this);
    qDebug() << "FINISHED";
}

MainWindow::~MainWindow()
{
    ui->messageDisplay->clear();
    delete ui;
}

void MainWindow::onSendButtonClicked() {
    QString message = ui->messageEdit->toPlainText();
    if (!message.isEmpty()) {
        QListWidgetItem *item = new QListWidgetItem("Art\n" + message);

        QPixmap avatarImage(":/assets/assets/small_cassie.png");

        item->setIcon(QIcon(avatarImage));

        ui->messageDisplay->addItem(item);

        ui->messageDisplay->scrollToBottom();

        ui->messageEdit->clear();
    }
}

void MainWindow::connectSocket(QTcpSocket *socket, QString ip, quint16 port) {
    socket->connectToHost(ip, port);
}


bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == ui->messageEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(static_cast<QInputEvent *>(event));
        if (keyEvent->key() == Qt::Key_Return && !(keyEvent->modifiers() & Qt::ShiftModifier)) {
            onSendButtonClicked();
            return true; // prevent newline
        }
    }
    return QMainWindow::eventFilter(obj, event);
}


void MainWindow::onReadyRead() {
    qDebug() << "Well, hello there!";
}




