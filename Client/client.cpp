#include "client.h"
#include <QDebug>
#include <QThread>

Client::Client(QObject *parent) : QObject(parent), socket(new QTcpSocket(this)) {
    connect(socket, &QTcpSocket::connected, this, &Client::onConnected);
    connect(socket, &QTcpSocket::readyRead, this, &Client::onReadyRead);
    connect(socket, &QAbstractSocket::errorOccurred, this, &Client::onError);
}

bool Client::connectToServer(const QString &host, quint16 port) {
    socket->connectToHost(host, port);
    if (!socket->waitForConnected(3000)) {
        qDebug() << "Trying of connection is failed!";
        return false;
    }
    return true;
}


void Client::sendMessage(const QJsonObject &message) {
    if (socket->state() == QAbstractSocket::ConnectedState) {
        QJsonDocument doc(message);
        qDebug() << "Sending message:" << doc.toJson();
        QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
        QDataStream out(socket);
        out.setVersion(QDataStream::Qt_5_7);
        out << QString::fromUtf8(jsonData);

        // Отладочное сообщение для подтверждения отправки
        qDebug() << "Message sent to server. " << message << "!";
    } else {
        qDebug() << "Socket is not connected!";
    }
}

void Client::onConnected() {
    qDebug() << "Connected to server!";
}

void Client::onReadyRead() {
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_7);

    while (socket->bytesAvailable() > 0) {
        QString message;
        in >> message;

        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());

        if (!doc.isNull()) {
            QJsonObject response = doc.object();
            qDebug() << "Client received response:" << response;
            emit messageReceived(message);
        } else {
            qDebug() << "Client received invalid JSON data.";
        }
    }
}

void Client::onError(QAbstractSocket::SocketError socketError) {
    qDebug() << "Socket error:" << socketError << " - " << socket->errorString();
}

Client::~Client() {
    if (socket && socket->state() == QAbstractSocket::ConnectedState) {
        QJsonObject message;
        message["command"] = "DELETE_SESSION";
        this->sendMessage(message);
        QThread::sleep(100);
        socket->disconnectFromHost();
    }
    delete socket;  // Удаляем сокет
}

void Client::closeConnection() {
    if (socket) {
        QJsonObject message;
        message["command"] = "DELETE_SESSION";
        this->sendMessage(message);
        socket->close();
        socket->deleteLater();
    }
}

bool Client::state(){
    if(socket->state() == QAbstractSocket::ConnectedState){
            return true;
    }
    else{
        return false;
    }
}

