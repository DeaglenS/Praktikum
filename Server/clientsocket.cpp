#include "clientsocket.h"

ClientSocket::ClientSocket(QObject *parent) : QTcpSocket(parent) {
    connect(this, &QTcpSocket::readyRead, this, &ClientSocket::readData);
}

void ClientSocket::readData() {
    QDataStream in(this);
    in.setVersion(QDataStream::Qt_5_7);

    while (bytesAvailable() > 0) {
        QString message;
        in >> message;
        qDebug() << "ClientSocket received message:" << message;
        emit messageReceived(message);
    }
}
