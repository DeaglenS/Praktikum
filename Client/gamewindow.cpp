#include "gamewindow.h"
#include "ui_gamewindow.h"
#include <QMessageBox>
#include <QTimer>
#include <accountwindow.h>
#include <loginwindow.h>
GameWindow::GameWindow(QWidget *parent, Client *client, QString Login, QString Password)
    : QDialog(parent)
    , ui(new Ui::GameWindow)
    , client(client)
    , Login(Login)
    , Password(Password)
{
    ui->setupUi(this);
    disconnect(client, &Client::messageReceived, this, &GameWindow::onMessageReceived);
    connect(client, &Client::messageReceived, this, &GameWindow::onMessageReceived, Qt::UniqueConnection);
}

GameWindow::~GameWindow()
{
    if (client) {
        disconnect(client, &Client::messageReceived, this, &GameWindow::onMessageReceived);
            // Не удаляем клиент, так как он передан извне
      //  client->closeConnection();
      //  delete client;
    }
    delete ui;
}

void GameWindow::on_rock_Button_clicked()
{
    QJsonObject message;
    message["command"] = "MAKE_CHOOSE";
    message["choose"] = "Rock";
    if (client->state() == true) {
        client->sendMessage(message);
        //QMessageBox::information(this, "Server Response", "sвв");
        ui->paper_Button->setEnabled(false);
        ui->rock_Button->setEnabled(false);
        ui->scissors_Button->setEnabled(false);

    } else {
        qDebug() << "Сокет не подключен. Невозможно отправить сообщение.";
        QMessageBox::warning(this, "Ошибка соединения", "Сокет не подключен к серверу.");
    }

}


void GameWindow::on_scissors_Button_clicked()
{
    QJsonObject message;
    message["command"] = "MAKE_CHOOSE";
    message["choose"] = "Scissors";
    if (client->state() == true) {
        client->sendMessage(message);
        //QMessageBox::information(this, "Server Response", "sвв");
        ui->paper_Button->setEnabled(false);
        ui->rock_Button->setEnabled(false);
        ui->scissors_Button->setEnabled(false);
    } else {
        qDebug() << "Сокет не подключен. Невозможно отправить сообщение.";
        QMessageBox::warning(this, "Ошибка соединения", "Сокет не подключен к серверу.");
    }

}


void GameWindow::on_paper_Button_clicked()
{
    QJsonObject message;
    message["command"] = "MAKE_CHOOSE";
    message["choose"] = "Paper";
    if (client->state() == true) {
        client->sendMessage(message);
        //QMessageBox::information(this, "Server Response", "sвв");
        ui->paper_Button->setEnabled(false);
        ui->rock_Button->setEnabled(false);
        ui->scissors_Button->setEnabled(false);
    } else {
        qDebug() << "Сокет не подключен. Невозможно отправить сообщение.";
        QMessageBox::warning(this, "Ошибка соединения", "Сокет не подключен к серверу.");
    }

}

void GameWindow::onMessageReceived(const QString &message) {


    QMessageBox::information(this, "3Server Response", message);
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    //senderSocket->write(doc.toJson());
    if (!doc.isObject()) {
        qDebug() << "JSON problem!";
        //return;
    }

    QJsonObject json = doc.object();
    if (json.contains("type") && json.contains("result")) {
        QString type = json["type"].toString();
        QString result = json["result"].toString();

        if(type == "game_status"){
            ui->stateLabel->setText(result);
            QTimer *timer = new QTimer(this);
            timer->start(3000);
            Client *newClient = new Client(this);
            newClient->connectToServer("127.0.0.1", 50002);//поменять чтобы было в динамике

            if(Login != nullptr && Password!= nullptr){
                if (newClient->state() == true) {
                    hide();
                    AccountWindow *accountWindow = new AccountWindow(this, newClient, Login, Password);
                    accountWindow->show();
                } else {
                    this->close();
                }

            }
            else{
                     if (newClient->state() == true) {
                    hide();
                         LoginWindow *loginWindow = new LoginWindow(this);
                         loginWindow->show();
                    }
                     else{
                        this->close();
                    }
            }
        }
    }
    else{  QMessageBox::warning(this, "4Получил", "4Получил.");}


}




