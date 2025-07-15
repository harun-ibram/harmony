#include <QDebug>
#include <QTcpSocket>
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
    ui->messageDisplay->setIconSize(QSize(48, 48));
}

MainWindow::~MainWindow()
{
    ui->messageDisplay->clear();
    delete ui;
}


void MainWindow::onSendButtonClicked() {
    QString message = ui->messageEdit->toPlainText();
    if (!message.isEmpty()) {
        QListWidgetItem *item = new QListWidgetItem("You: " + message);
        QPixmap avatarImage(":/assets/assets/small_cassie.png");
        item->setIcon(QIcon(avatarImage));
        ui->messageDisplay->addItem(item);
        ui->messageDisplay->scrollToBottom();
        ui->messageEdit->clear();
    }
}

void MainWindow::onConnectClicked() {
    connectDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString ip = dialog.getIP();
        quint16 port = dialog.getPort();


        QTcpSocket *socket = new QTcpSocket(this);
        socket->connectToHost(ip, port);
        qDebug() << "Done";
        ui->messageDisplay->addItem("Connected to " + ip + " : " + port);
    }
}
