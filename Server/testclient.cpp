#include "testclient.h"
#include <QDebug>

TestClient::TestClient(QObject *parent) : QObject(parent), socket(new QTcpSocket(this)) {
    connect(socket, &QTcpSocket::connected, this, &TestClient::onConnected);
    connect(socket, &QTcpSocket::readyRead, this, &TestClient::onReadyRead);
    connect(socket, &QAbstractSocket::errorOccurred, this, &TestClient::onError);
}

void TestClient::connectToServer(const QString &host, quint16 port) {
    socket->connectToHost("127.0.0.1", 50002);
    if (!socket->waitForConnected(3000)) {
        qDebug() << "Connection failed!";
    }
}

void TestClient::sendMessage(const QJsonObject &message) {
    if (socket->state() == QAbstractSocket::ConnectedState) {
        QJsonDocument doc(message);
        qDebug() << "Sending message:" << doc.toJson();
        QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
        QDataStream out(socket);
        out.setVersion(QDataStream::Qt_5_7);
        out << QString::fromUtf8(jsonData);
    } else {
        qDebug() << "Socket is not connected!";
    }
}

void TestClient::onConnected() {
    qDebug() << "Connected to server!";
}

void TestClient::onReadyRead() {
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_7);

    while (socket->bytesAvailable() > 0) {
        QString message;
        in >> message;
        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
        if (!doc.isNull()) {
            QJsonObject response = doc.object();
            qDebug() << "Received response:" << response;
        } else {
            qDebug() << "Received invalid JSON data.";
        }
    }
}
void TestClient::onError(QAbstractSocket::SocketError socketError) {
    qDebug() << "Socket error:" << socketError << " - " << socket->errorString();
}
