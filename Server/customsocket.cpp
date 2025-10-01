#include "customsocket.h"
#include <QDebug>

CustomSocket::CustomSocket(QObject *parent) : QTcpSocket(parent) {
    connect(this, &QTcpSocket::readyRead, this, &CustomSocket::processData);
}

void CustomSocket::processData() {
    QDataStream in(this);
    in.setVersion(QDataStream::Qt_5_7);

    while (bytesAvailable() > 0) {
        QString message;
        in >> message;
        qDebug() << "ClientSocket received message:" << message;
        emit messageReceived(message);
    }
}
/*
void CustomSocket::processData() {
    QDataStream in(this);
    in.setVersion(QDataStream::Qt_5_7);

    // Если буфер пуст, сначала читаем размер сообщения
    if (messageBuffer.isEmpty()) {
   if (bytesAvailable() < static_cast<qint64>(sizeof(quint32))) {
            return;
        }
        quint32 messageSize;
        in >> messageSize;
        messageBuffer.reserve(messageSize);
    }

    // Читаем данные в буфер
    messageBuffer.append(readAll());

    // Если буфер содержит полное сообщение, обрабатываем его
    if (messageBuffer.size() >= static_cast<int>(sizeof(quint32))) {
        QJsonDocument doc = QJsonDocument::fromJson(messageBuffer);
        if (!doc.isNull()) {
            emit messageReceived(doc.object());
            messageBuffer.clear();
        }
    }
}
*/
