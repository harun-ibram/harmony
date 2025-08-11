#include <QDebug>
#include <QTcpSocket>
#include <QInputEvent>
#include <QTcpServer>
#include <QMessageBox>
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "connectdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->sendButton, &QPushButton::clicked, this, &MainWindow::onSendButtonClicked);
    connect(ui->actionConnect, &QAction::triggered, this, &MainWindow::onConnectClicked);

    server = new QTcpServer(this);
    server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection, this, &MainWindow::onNewConnection);

    if (!server->listen(QHostAddress::Any, 5002)) {
        qDebug() << "Server failed to start:" << server->errorString();
    } else {
        qDebug() << "Server listening on port" << server->serverPort();
    }

    ui->messageDisplay->setIconSize(QSize(48, 48));
    ui->messageEdit->installEventFilter(this);
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

void MainWindow::onConnectClicked() {
    connectDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString ip = dialog.getIP();
        quint16 port = dialog.getPort();


        socket = new QTcpSocket(this);
        connectSocket(socket, ip, port);

        ui->messageDisplay->clear();

        ui->messageDisplay->addItem("Connected to " + ip + " : " + port);
    }
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

void MainWindow::onNewConnection() {
    if (QMessageBox::question(
            this,
            "Incoming Connection",
            "A client is trying to connect.\nDo you want to accept?",
            QMessageBox::Yes | QMessageBox::No
            ) == QMessageBox::Yes)
    {
        QTcpSocket *clientSocket = server->nextPendingConnection();
        qDebug() << "Client connected from" << clientSocket->peerAddress().toString();

        connect(clientSocket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
        connect(clientSocket, &QTcpSocket::disconnected, clientSocket, &QTcpSocket::deleteLater);
    }
    else {
        // Refuse the connection: just discard the pending connection
        QTcpSocket *temp = server->nextPendingConnection();
        temp->disconnectFromHost();
        temp->deleteLater();
    }
}

void MainWindow::onReadyRead() {

}




