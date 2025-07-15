#include <QDebug>
#include <QTcpSocket>
#include <QInputEvent>
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

void MainWindow::onConnectClicked() {
    connectDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString ip = dialog.getIP();
        quint16 port = dialog.getPort();


        QTcpSocket *socket = new QTcpSocket(this);
        socket->connectToHost(ip, port);
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
