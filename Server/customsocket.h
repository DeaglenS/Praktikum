#ifndef CUSTOM_SOCKET_H
#define CUSTOM_SOCKET_H

#include <QTcpSocket>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>

class CustomSocket : public QTcpSocket {
    Q_OBJECT

public:
    explicit CustomSocket(QObject *parent = nullptr);

signals:
    void messageReceived(QString  &message);

private slots:
    void processData();

private:
    QByteArray messageBuffer;
};

#endif // CUSTOM_SOCKET_H
