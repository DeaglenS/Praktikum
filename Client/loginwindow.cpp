#include "loginwindow.h"
#include "ui_loginwindow.h"
#include <QJsonDocument>
#include <QJsonObject>
#include "client.h"
#include <QMessageBox>
#include "accountwindow.h"
#include "registrationwindow.h"
#include <QCryptographicHash>
#include <QMainWindow>

LoginWindow::LoginWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LoginWindow)
    , client(new Client(this))
{
    ui->setupUi(this);
    if(!client->connectToServer("127.0.0.1", 50002)){
        close();
        QMessageBox::warning(this, "Ошибка", "Нет ответа от сервера");
    }
    connect(client, &Client::messageReceived, this, &LoginWindow::onMessageReceived);
}//создать время у токена
///переделать в токены
LoginWindow::~LoginWindow()
{
    // qDebug() << "close";


    delete ui;
   delete client;

}

void LoginWindow::on_auth_Button_clicked()
{


    QString login = ui->login_lineEdit->text();
    QString password = ui->pass_lineEdit->text();
    this->login = login;
    this->password = password;
    if(login.isEmpty() || password.isEmpty()){
        QMessageBox::warning(this, "Ошибка", "Введите все данные.");
    }

    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QString hashedPassword = hash.toHex();

    QJsonObject message;
    message["command"] = "LOGIN";
    message["login"] = login;
    message["password"] = hashedPassword; //шифровать

    if (client->state() == true) {
        client->sendMessage(message);
        //QMessageBox::information(this, "Server Response", "sвв");
    } else {
        qDebug() << "Сокет не подключен. Невозможно отправить сообщение.";
        QMessageBox::warning(this, "Ошибка соединения", "Сокет не подключен к серверу.");
    }


}

void LoginWindow::onMessageReceived(const QString &message) {

        // QMessageBox::information(this, "1Server Response", message);
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    //senderSocket->write(doc.toJson());
    if (!doc.isObject()) {
        qDebug() << "JSON problem!";
        //return;
    }

    QJsonObject json = doc.object();
    if (json.contains("status") && json.contains("wins")) {
        QString status = json["status"].toString();
        int wins = json["wins"].toInt();
        if(status == "success"){
            hide();
            disconnect(client, &Client::messageReceived, this, &LoginWindow::onMessageReceived);
            AccountWindow *accountWindow= new AccountWindow(this, client, login, password, wins);
            accountWindow->show();
        }
    }
    if (json.contains("type") && json.contains("state")) {
        QString type = json["type"].toString();
        QString state = json["state"].toString();

        if(type == "game_status" && state == "started"){
            //hide();
            disconnect(client, &Client::messageReceived, this, &LoginWindow::onMessageReceived);
            close();

            GameWindow *gameWindow = new GameWindow(this, this->client);
            gameWindow->show();
        }
    }
    /*
    else if (json.contains("status")) {
        QString status = json["status"].toString();

        if (status == "success") {
            QMessageBox::information(this, "Success", "Account created successfully!");
            // this->close(); // Закрываем окно после успешного создания аккаунта
        } else {
            QString errorMessage = json["error"].toString();
          //  QMessageBox::warning(this, "1Error", errorMessage);
        }
    }
    */
    else { // QMessageBox::warning(this, "1Получил", "1Получил.");
    }

}


void LoginWindow::on_registration_Button_clicked()
{
    disconnect(client, &Client::messageReceived, this, &LoginWindow::onMessageReceived);
    hide();
    RegistrationWindow *registrationWindow = new RegistrationWindow(this, this->client);
    registrationWindow->show();
}




void LoginWindow::on_guest_Button_clicked()
{

    ui->auth_Button->setEnabled(false);
    ui->guest_Button->setEnabled(false);
    ui->registration_Button->setEnabled(false);
    QJsonObject message;
    message["command"] = "START_ANONYM_GAME";

    if (client->state() == true) {
        client->sendMessage(message);
        //QMessageBox::information(this, "Server Response", "sвв");
    } else {
        qDebug() << "Сокет не подключен. Невозможно отправить сообщение.";
        QMessageBox::warning(this, "Ошибка соединения", "Сокет не подключен к серверу.");
    }

    /*
    QJsonObject message;
    message["command"] = "CREATE_USER";
    message["login"] = "1";
    message["password"] = "hashedPassword";
    message["name"] = "1";

    if (client->state() == true) {
        client->sendMessage(message);
        QMessageBox::information(this, "Server Response", "Удачно");
    } else {
        qDebug() << "Сокет не подключен. Невозможно отправить сообщение.";
        QMessageBox::warning(this, "Ошибка соединения", "Сокет не подключен к серверу.");
    }
*/
}


