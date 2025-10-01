#include "registrationwindow.h"
#include "ui_registrationwindow.h"
#include <QMessageBox>
#include "client.h"
#include <QCryptographicHash>
#include <loginwindow.h>

RegistrationWindow::RegistrationWindow(QWidget *parent, Client *client)
    : QDialog(parent)
    , ui(new Ui::RegistrationWindow)
    , client(client)
{
    ui->setupUi(this);
    disconnect(client, &Client::messageReceived, this, &RegistrationWindow::onMessageReceived);
    connect(client, &Client::messageReceived, this, &RegistrationWindow::onMessageReceived, Qt::UniqueConnection);

}
RegistrationWindow::~RegistrationWindow()
{
    delete ui;
}



void RegistrationWindow::on_makeAcc_Button_clicked()
{
    QString login = ui->login_lineEdit->text();
    QString pass1 = ui->pass1_lineEdit->text();
    QString pass2 = ui->pass2_lineEdit->text();
    QString name = ui->name_lineEdit->text();

    if (login.isEmpty() || name.isEmpty() || pass1.isEmpty() || pass2.isEmpty()) {
        QMessageBox::warning(this, "Missing Information", "Please fill in all fields!");
        return;
    }

    if (pass1 != pass2) {
        QMessageBox::warning(this, "Password Mismatch", "Passwords do not match!");
        return;
    }

    QByteArray hash = QCryptographicHash::hash(pass1.toUtf8(), QCryptographicHash::Sha256);
    QString hashedPassword = hash.toHex();

    QJsonObject message;
    message["command"] = "CREATE_USER";
    message["login"] = login;
    message["password"] = hashedPassword;
    message["name"] = name;

    if (client->state() == true) {
        client->sendMessage(message);
       // QMessageBox::information(this, "Server Response", "Удачно");
    } else {
        qDebug() << "Сокет не подключен. Невозможно отправить сообщение.";
        QMessageBox::warning(this, "Ошибка соединения", "Сокет не подключен к серверу.");
    }

    this->close();
}

void RegistrationWindow::onMessageReceived(const QString &message) {
    //QMessageBox::information(this, "2Server Response", message);

    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());

    if (!doc.isObject()) {
        qDebug() << "JSON problem!";
        return;
    }

    QJsonObject json = doc.object();

    // Обработка ответа от сервера, например:
    if (json.contains("status")) {
        QString status = json["status"].toString();

        if (status == "success") {
               disconnect(client, &Client::messageReceived, this, &RegistrationWindow::onMessageReceived);
            QMessageBox::information(this, "Success", "Account created successfully!");

               LoginWindow *loginWindow= new LoginWindow();
               loginWindow->show();

        } else {
               disconnect(client, &Client::messageReceived, this, &RegistrationWindow::onMessageReceived);
            QString errorMessage = json["error"].toString();
            QMessageBox::warning(this, "Error", errorMessage);
            LoginWindow *loginWindow= new LoginWindow();
            loginWindow->show();
        }
    }
}

void RegistrationWindow::on_back_Button_clicked()
{
    disconnect(client, &Client::messageReceived, this, &RegistrationWindow::onMessageReceived);
    close();
    LoginWindow *loginWindow= new LoginWindow();
    loginWindow->show();
}
void RegistrationWindow::closeEvent(QCloseEvent *event) {
    // Выполните необходимую очистку здесь

    /*if (client) {
        client->closeConnection();
    }*/
    // Вызовите реализацию базового класса, чтобы гарантировать правильное закрытие окна
    QDialog::closeEvent(event);
}



