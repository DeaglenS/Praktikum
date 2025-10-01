#include "worker.h"
#include <QDebug>

Worker::Worker(QTcpSocket *socket, QObject *parent) : QObject(parent), socket(socket) {
    connect(socket, &QTcpSocket::readyRead, this, &Worker::processMessage);
    connect(socket, &QTcpSocket::disconnected, this, &Worker::handleDisconnect);
}

void Worker::processMessage() {
    while (socket->canReadLine()) {
        QString message = QString::fromUtf8(socket->readLine()).trimmed();
        qDebug() << "Received message:" << message;

        // Обработка JSON
        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
        if (!doc.isObject()) {
            qDebug() << "JSON problem!";
            return;
        }

        QJsonObject json = doc.object();
        handleCommand(json); // Обработка команды
    }
}

void Worker::handleDisconnect() {
    socket->deleteLater();
    this->deleteLater(); // Удаляем обработчик
}

void Worker::handleCommand(const QJsonObject &json) {
    if (json.contains("command")) {
        QString command = json["command"].toString();

        if (command == "LOGIN") {
            QString username = json["username"].toString();
            // Здесь добавьте логику для проверки логина
            qDebug() << "Login command received for user:" << username;
            // Например, отправляем ответ обратно клиенту
            QJsonObject response;
            response["status"] = "success";
            response["message"] = "Login successful.";
            socket->write(QJsonDocument(response).toJson() + "\n");
            socket->flush();
        } else if (command == "CREATE_USER") {
            QString username = json["username"].toString();
            QString password = json["password"].toString();
            // Здесь добавьте логику для создания пользователя
            qDebug() << "Create user command received for user:" << username;
            // Например, отправляем ответ обратно клиенту
            QJsonObject response;
            response["status"] = "success";
            response["message"] = "User created successfully.";
            socket->write(QJsonDocument(response).toJson() + "\n");
            socket->flush();
        } else {
            qDebug() << "Unknown command:" << command;
            QJsonObject response;
            response["status"] = "error";
            response["message"] = "Unknown command.";
            socket->write(QJsonDocument(response).toJson() + "\n");
            socket->flush();
        }
    } else {
        qDebug() << "No command found in JSON.";
    }
}
