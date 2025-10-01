#include <QCoreApplication>
#include <QtConcurrent>
#include <QJsonObject>
#include <QString>
#include <QVector>
#include "server.h"
//#include "databasemanager.h"
#include "testclient.h"
#include <QThread>


void sendLoginMessage(TestClient& client, int k);
void sendCreateUserMessage(TestClient& client, int k);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Server server1;
    server1.startServer();

    return a.exec();
    /*
    DatabaseManager db("db.db");
    if(db.connect()){
        qDebug() << "Can open database:" ;
    }
    else{
        qDebug() << "Cannot open database:" ;
    }

    db.createTable();

    db.createUser("name", "login", "password");
    db.createUser("name2", "login2", "password2");
    db.createUser("name2", "login3", "password2");
    db.printDatabase();


    qDebug() << db.loginUser("login", "password");
    qDebug() << db.loginUser("login", "passxsxswodrd");
    qDebug() << db.loginUser("logwin", "passxsxswodrd");

    QThread::sleep(2);
    Server server;
    server.startServer();

    */

    // Второе сообщение
    /*
    QJsonObject message2;
    message2["command"] = "CREATE_USER";
    message2["login"] = "logвавваin";
    message2["password"] = "password";
    client.sendMessage(message2);
    */
    /*
    for(int k = 0; k < 100; k ++){
        QJsonObject message3;
        message3["command"] = "LOGIN";
        message3["login"] = "log433ifn2" + QString::number(k);
        message3["name"] = "pa4ssword2";
        message3["password"] = "password2";
        client.sendMessage(message3);
    }
    */
    /*
    QVector<QFuture<void>> futures;

    for (int k = 0; k < 100; k++) {
        futures.append(QtConcurrent::run(sendCreateUserMessage, std::ref(client), k));
          futures.append(QtConcurrent::run( sendLoginMessage, std::ref(client), k));
    }

    // Ожидаем завершения всех потоков
    for (auto& future : futures) {
        future.waitForFinished();
    }
    */

    return a.exec();
}
///добавить локи
///
void sendLoginMessage(TestClient& client, int k) {
    QJsonObject message;
    message["command"] = "LOGIN";
    message["login"] = "log433ifn2" + QString::number(k);
    message["name"] = "pa4ssword2";
    message["password"] = "password2";
    client.sendMessage(message);
}
void sendCreateUserMessage(TestClient& client, int k) {
    QJsonObject message;
    message["command"] = "CREATE_USER";
    message["login"] = "log433ifn2" + QString::number(k*100);
    message["name"] = "pa4ssword2";
    message["password"] = "password2";
    client.sendMessage(message);
}
