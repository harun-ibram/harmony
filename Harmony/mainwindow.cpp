#include <QDebug>
#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->sendButton, &QPushButton::clicked, this, &MainWindow::onSendButtonClicked);
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
