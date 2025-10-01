#ifndef TESTCLIENT_H
#define TESTCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>

class TestClient : public QObject {
    Q_OBJECT

public:
    explicit TestClient(QObject *parent = nullptr);
    void connectToServer(const QString &host, quint16 port);
    void sendMessage(const QJsonObject &message);

private slots:
    void onConnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket *socket;
};

#endif // TESTCLIENT_H
