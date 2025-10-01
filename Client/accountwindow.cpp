#include "accountwindow.h"
#include "ui_accountwindow.h"
#include <QMessageBox>
#include <QMainWindow>

AccountWindow::AccountWindow(QWidget *parent, Client *client, QString Login, QString Password, int wins)
    : QDialog(parent)
    , ui(new Ui::AccountWindow)
    , client(client)
    , Login(Login)
    , Password(Password)
{
    ui->setupUi(this);
    disconnect(client, &Client::messageReceived, this, &AccountWindow::onMessageReceived);
    connect(client, &Client::messageReceived, this, &AccountWindow::onMessageReceived, Qt::UniqueConnection);
    QJsonObject message;
    message["command"] = "GET_INFO";
    ui->label->setText(QString::number(wins));
    if (client->state() == true) {
        client->sendMessage(message);
        // QMessageBox::information(this, "Server Response", "sвв");
    } else {
        qDebug() << "Сокет не подключен. Невозможно отправить сообщение.";
        QMessageBox::warning(this, "Ошибка соединения", "Сокет не подключен к серверу.");
    }
}

AccountWindow::~AccountWindow()
{
    if (client) {
        disconnect(client, &Client::messageReceived, this, &AccountWindow::onMessageReceived);
        // Не удаляем клиент, так как он передан извне
        //client->closeConnection();
        //delete client;
    }
    delete ui;
}

void AccountWindow::on_pushButton_clicked()
{

    ui->pushButton->setEnabled(false);

    QJsonObject message;
    message["command"] = "CREATE_GAME";

    if (client->state() == true) {
        client->sendMessage(message);
       // QMessageBox::information(this, "Server Response", "sвв");
    } else {
        qDebug() << "Сокет не подключен. Невозможно отправить сообщение.";
        QMessageBox::warning(this, "Ошибка соединения", "Сокет не подключен к серверу.");
    }

}
void AccountWindow::onMessageReceived(const QString &message) {

   //QMessageBox::information(this, "3Server Response", message);
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    //senderSocket->write(doc.toJson());
    if (!doc.isObject()) {
        qDebug() << "JSON problem!";
        //return;
    }

    QJsonObject json = doc.object();
    if (json.contains("type") && json.contains("state")) {
        QString type = json["type"].toString();
        QString state = json["state"].toString();

        if(type == "game_status" && state == "started"){
            close();
            GameWindow *gameWindow = new GameWindow(this, this->client, Login, Password);
            gameWindow->show();
        }
        disconnect(client, &Client::messageReceived, this, &AccountWindow::onMessageReceived);
    }
    else if(json.contains("socketDescriptor"))
    {
        QString socketDescriptor = json["socketDescriptor"].toString();
        if(ui->label->text()!="0")
        {
                 ui->label->setText("Игрок номер " + socketDescriptor + " имеет " + ui->label->text() + " выиграшей");
        }
        else{
            ui->label->setText("");
        }


    }
    else{ QMessageBox::warning(this, "3Получил", "3Получил.");}

}

void AccountWindow::on_privateGame_pushButton_clicked()
{
      ui->pushButton->setEnabled(false);
    QString id = ui->id_lineEdit->text();
    if(id.isEmpty()){
        QMessageBox::warning(this, "Ошибка", "Введите все данные.");
    }
    else{
        ui->pushButton->setEnabled(false);
        ui->privateGame_pushButton->setEnabled(false);

        QJsonObject message;
        message["command"] = "CREATE_GAME";
        message["password"] = id;

        if (client->state() == true) {
            client->sendMessage(message);
            // QMessageBox::information(this, "Server Response", "sвв");
        } else {
            qDebug() << "Сокет не подключен. Невозможно отправить сообщение.";
            QMessageBox::warning(this, "Ошибка соединения", "Сокет не подключен к серверу.");
        }
    }
}


